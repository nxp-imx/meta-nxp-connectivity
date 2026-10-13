/*
 * Copyright 2024 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * port_linux.c - Linux OS port for the shared uart_mux library.
 *
 * This is the exact mirror of port_zephyr.c: it owns every OS specific piece
 * (UART fd setup, one PTY per logical channel, the non-blocking PTY output
 * queues and the poll() loop) and binds the OS-agnostic shared core to it
 * through the struct uart_mux_io callbacks. The wire-protocol logic lives
 * entirely in the shared sources (uart_mux.c / hci_bridge.c /
 * spinel_bridge.c), the EXACT SAME C files compiled by the Zephyr companion
 * app. Application/bridge code above this port only ever calls the API in
 * port_linux.h, identical in shape to port_zephyr.h.
 *
 * Threading model: single-threaded poll() loop, so uart_mux_io lock/unlock are
 * NULL (no serialization needed for TX assembly).
 *
 * BLE output prioritization: the BLE channel splits its PTY output into a
 * high-priority ctrl queue and a small bounded adv-report ring (drop-oldest),
 * so a scan-disable Command Complete never sits behind an advertising-report
 * flood on a slow PTY consumer (e.g. btattach). All other channels use a
 * single FIFO output queue.
 */

#define _GNU_SOURCE
#include "port_linux.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <linux/serial.h>

#include "uart_mux.h"

static int g_verbose;


/*
 * Build a system wall-clock (CLOCK_REALTIME) timestamp prefix in the SAME
 * format used by the sched dump header and the coex orchestrator log lines:
 * "[YYYY-MM-DD HH:MM:SS.mmm] ". Every muxd log line is prefixed with this so
 * muxd.log, coex.log and otbr-agent.log can be correlated by matching absolute
 * timestamps (there is no combined file). The caller passes a buffer; on any
 * failure the buffer is left as an empty string so logging still proceeds.
 */
void uart_mux_port_timestamp(char *buf, size_t n)
{
	struct timespec rt;
	struct tm tmv;
	char tbuf[32];

	if (!buf || n == 0) {
		return;
	}
	buf[0] = '\0';
	if (clock_gettime(CLOCK_REALTIME, &rt) != 0) {
		return;
	}
	localtime_r(&rt.tv_sec, &tmv);
	strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", &tmv);
	snprintf(buf, n, "[%s.%03ld] ", tbuf, rt.tv_nsec / 1000000L);
}

static void logv(const char *fmt, ...)
{
	if (!g_verbose) {
		return;
	}
	char tsb[48];
	va_list ap;

	uart_mux_port_timestamp(tsb, sizeof(tsb));
	fputs(tsb, stderr);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}


void uart_mux_port_set_verbose(int on)
{
	g_verbose = on;
}

/* Monotonic microsecond clock for the head-of-line latency metric. */
static uint64_t now_us(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}


/* -----------------------------------------------------------------------
 * Non-blocking PTY output queues.
 *
 * The UART read loop must NEVER block on a slow PTY consumer. Decoded frames
 * are enqueued here and drained only when poll() reports the PTY writable.
 * ----------------------------------------------------------------------- */
#define PTY_Q_MAX     256
#define BLE_ADV_Q_MAX 32

struct out_item {
	uint16_t len;
	uint16_t off;
	uint8_t  data[UART_MUX_PAYLOAD_MAX];
};

struct out_q {
	struct out_item *items;
	int cap, head, count;
};

static void q_push_drop_oldest(struct out_q *q, const uint8_t *d, size_t n)
{
	if (n > UART_MUX_PAYLOAD_MAX) {
		n = UART_MUX_PAYLOAD_MAX;
	}
	if (q->count == q->cap) {
		q->head = (q->head + 1) % q->cap; /* drop oldest */
		q->count--;
	}
	struct out_item *it = &q->items[(q->head + q->count) % q->cap];

	it->len = (uint16_t)n;
	it->off = 0;
	memcpy(it->data, d, n);
	q->count++;
}

static struct out_item *q_front(struct out_q *q)
{
	return q->count ? &q->items[q->head] : NULL;
}

static void q_pop(struct out_q *q)
{
	if (q->count) {
		q->head = (q->head + 1) % q->cap;
		q->count--;
	}
}

/* -----------------------------------------------------------------------
 * Egress (host -> MCXW72) per-channel FIFO.
 *
 * uart_mux_port_send() enqueues a logical payload here instead of writing it
 * to the wire immediately. The drainer in the poll loop picks a channel per
 * the runtime policy and transmits at most frag_bytes of the head payload as
 * one mux frame, so a large Thread payload can be preempted between fragments
 * by a freshly queued BLE payload. The off field tracks how many bytes of the
 * head payload have already been emitted as fragments.
 * ----------------------------------------------------------------------- */
#define EGRESS_Q_MAX 64

struct egress_item {
	uint16_t len;
	uint16_t off;
	uint64_t enq_us;               /* monotonic enqueue time (HOL metric) */
	uint8_t  data[UART_MUX_PAYLOAD_MAX];
};

struct egress_q {
	struct egress_item items[EGRESS_Q_MAX];
	int head;
	int count;
};


/* -----------------------------------------------------------------------
 * Per-channel endpoint state.
 * ----------------------------------------------------------------------- */
struct mux_channel {
	int used;
	uint8_t id;
	int pty_fd;                     /* PTY master for this channel */
	uart_mux_port_rx_cb_t wire_rx;  /* wire payload -> bridge from_mux */
	uart_mux_port_rx_cb_t local_rx; /* PTY bytes    -> bridge from_local */
	void *user;

	/* Output queues (wire -> PTY). BLE uses ctrl + adv; others use ctrl. */
	int split;                   /* 1 = ctrl/adv split (BLE only) */
	struct out_q ctrl_q;
	struct out_q adv_q;
	struct out_item ctrl_items[PTY_Q_MAX];
	struct out_item adv_items[BLE_ADV_Q_MAX];

	/* Egress (host -> wire) FIFO + counters for this channel. */
	struct egress_q eq;
	struct mux_sched_ch_stats stats;
};

#define MAX_CHANNELS (UART_MUX_CH_MAX + 1)

struct port_ctx {
	int uart_fd;
	int baud;                         /* configured wire baud rate */
	uint16_t frag_auto;               /* baud-derived preemption fragment
					   * size (bytes) targeting ~1 ms of
					   * wire time; used when a policy sets
					   * frag_bytes == 0 */
	struct uart_mux core;
	struct mux_channel ch[MAX_CHANNELS];
	volatile int stop;


	/* Egress scheduler state. */
	struct mux_sched_policy policy;   /* live policy consulted by picker */
	uint32_t strict_starve;           /* STRICT: consecutive hi-prio frames
					   * served while a lower-prio channel
					   * had pending data */
	uint16_t wrr_ch;                  /* WRR: channel currently being served */
	uint16_t wrr_left;                /* WRR: frames left in this channel's
					   * turn for the current round */
	volatile sig_atomic_t pending_profile; /* set by signal handler, -1 idle */
};

static struct port_ctx g_port;

/* -----------------------------------------------------------------------
 * Built-in scheduler profiles, cycled by SIGUSR1 during bring-up/tuning.
 *
 *   0 STRICT BLE > Thread + Thread anti-starvation quota (nominal default:
 *     BLE audio latency protected, Thread never fully starved).
 *   1 WRR BLE:Thread = 3:1 (bandwidth sharing, softer BLE bias).
 *   2 STRICT Thread > BLE (inverted: Thread latency prioritised, e.g. if the
 *     mux later detects Thread must win).
 *   3 COEX-SAFE: WRR BLE:Thread = 2:1 with auto (~1 ms) fragments. Tuned so
 *     the Thread/Spinel channel is never starved long enough to trip the
 *     host RCP radio-tx timeout under a simultaneous BLE load (the observed
 *     otbr-agent self-kill), while BLE still gets twice the wire turns. This
 *     is the recommended coex default when both links run at once.
 * ----------------------------------------------------------------------- */
#define NUM_SCHED_PROFILES 4

static const struct mux_sched_policy sched_profiles[NUM_SCHED_PROFILES] = {

	/* profile 0: STRICT BLE > Thread, quota 4, 256-byte fragments. */
	{
		.mode = MUX_SCHED_STRICT,
		.prio_order = { UART_MUX_CH_BLE, UART_MUX_CH_THREAD, 0 },
		.weight = { 0 },
		.frag_bytes = 256,
		.starve_quota = 4,
	},
	/* profile 1: WRR BLE:Thread = 3:1, 256-byte fragments. */
	{
		.mode = MUX_SCHED_WRR,
		.prio_order = { 0 },
		.weight = {
			[UART_MUX_CH_BLE]    = 3,
			[UART_MUX_CH_THREAD] = 1,
		},
		.frag_bytes = 256,
		.starve_quota = 0,
	},
	/* profile 2: STRICT Thread > BLE, quota 4, 256-byte fragments. */
	{
		.mode = MUX_SCHED_STRICT,
		.prio_order = { UART_MUX_CH_THREAD, UART_MUX_CH_BLE, 0 },
		.weight = { 0 },
		.frag_bytes = 256,
		.starve_quota = 4,
	},
	/*
	 * profile 3: COEX-SAFE WRR BLE:Thread = 2:1, auto fragments.
	 * frag_bytes = 0 selects the baud-derived ~1 ms fragment (frag_auto),
	 * so preemption granularity scales with the wire speed instead of a
	 * fixed 256 bytes. The 2:1 weighting keeps BLE latency good while
	 * guaranteeing the Thread/Spinel channel one wire turn out of three,
	 * bounding its head-of-line wait well below the host RCP radio-tx
	 * timeout. Use this when BLE and Thread run simultaneously.
	 */
	{
		.mode = MUX_SCHED_WRR,
		.prio_order = { 0 },
		.weight = {
			[UART_MUX_CH_BLE]    = 2,
			[UART_MUX_CH_THREAD] = 1,
		},
		.frag_bytes = 0,
		.starve_quota = 0,
	},
};


static int g_active_profile;


static struct mux_channel *channel_lookup(uint8_t id)
{
	if (id < UART_MUX_CH_MIN || id > UART_MUX_CH_MAX) {
		return NULL;
	}
	struct mux_channel *c = &g_port.ch[id];

	return c->used ? c : NULL;
}

/* True if this ctrl->host H4 event is an LE Extended Advertising Report. */
static int is_ble_adv_report(const uint8_t *p, size_t n)
{
	return (n >= 4 && p[0] == 0x04 && p[1] == 0x3e && p[3] == 0x0d);
}

static void channel_drain(struct mux_channel *c)
{
	for (;;) {
		struct out_q *q = c->ctrl_q.count ? &c->ctrl_q :
				  (c->split && c->adv_q.count ? &c->adv_q : NULL);
		if (!q) {
			break;
		}
		struct out_item *it = q_front(q);
		ssize_t w = write(c->pty_fd, it->data + it->off,
				  (size_t)(it->len - it->off));
		if (w < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				break;
			}
			perror("[muxd] pty write");
			q_pop(q);
			continue;
		}
		it->off += (uint16_t)w;
		if (it->off >= it->len) {
			q_pop(q);
		}
	}
}

/* -----------------------------------------------------------------------
 * UART setup: raw mode + optional RTS/CTS.
 * ----------------------------------------------------------------------- */
static speed_t baud_to_speed(int baud)
{
	switch (baud) {
	case 115200:  return B115200;
	case 230400:  return B230400;
	case 460800:  return B460800;
	case 921600:  return B921600;
	case 1000000: return B1000000;
	case 1500000: return B1500000;
	case 2000000: return B2000000;
	case 3000000: return B3000000;
	case 4000000: return B4000000;
	default:      return 0;
	}
}

static int uart_open(const char *path, int baud, int flow)
{
	int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (fd < 0) {
		perror("open uart");
		return -1;
	}

	struct termios tio;

	if (tcgetattr(fd, &tio) < 0) {
		perror("tcgetattr");
		close(fd);
		return -1;
	}
	cfmakeraw(&tio);

	speed_t sp = baud_to_speed(baud);

	if (sp == 0) {
		fprintf(stderr, "unsupported baud %d\n", baud);
		close(fd);
		return -1;
	}
	cfsetispeed(&tio, sp);
	cfsetospeed(&tio, sp);

	tio.c_cflag |= (CLOCAL | CREAD);
	tio.c_cflag &= ~(size_t)CSIZE;
	tio.c_cflag |= CS8;
	tio.c_cflag &= ~(size_t)PARENB;
	tio.c_cflag &= ~(size_t)CSTOPB;
	if (flow) {
		tio.c_cflag |= CRTSCTS;
	} else {
		tio.c_cflag &= ~(size_t)CRTSCTS;
	}
	tio.c_cc[VMIN] = 0;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr(fd, TCSANOW, &tio) < 0) {
		perror("tcsetattr");
		close(fd);
		return -1;
	}

	/*
	 * iMX93 is much more capable than the MCXW72, so push the latency work
	 * to the host side. ASYNC_LOW_LATENCY tells the Linux tty layer to flush
	 * received bytes to userspace immediately instead of coalescing them on
	 * the ~10 ms flip-buffer timer, so a BLE event coming off the wire is
	 * seen by the poll loop without added driver latency. Best-effort: if
	 * the driver does not support it we keep going.
	 */
	struct serial_struct ss;

	if (ioctl(fd, TIOCGSERIAL, &ss) == 0) {
		ss.flags |= ASYNC_LOW_LATENCY;
		if (ioctl(fd, TIOCSSERIAL, &ss) != 0) {
			logv("[muxd] ASYNC_LOW_LATENCY not applied: %s\n",
			     strerror(errno));
		}
	}

	tcflush(fd, TCIOFLUSH);
	return fd;
}


/* -----------------------------------------------------------------------
 * PTY setup: create a master, return master fd, copy the slave name out.
 * ----------------------------------------------------------------------- */
static int pty_open(char *slave_name, size_t slen)
{
	int m = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (m < 0) {
		perror("posix_openpt");
		return -1;
	}
	if (grantpt(m) < 0) {
		perror("grantpt");
		close(m);
		return -1;
	}
	if (unlockpt(m) < 0) {
		perror("unlockpt");
		close(m);
		return -1;
	}
	if (ptsname_r(m, slave_name, slen) != 0) {
		perror("ptsname_r");
		close(m);
		return -1;
	}

	int s = open(slave_name, O_RDWR | O_NOCTTY);

	if (s >= 0) {
		struct termios tio;

		if (tcgetattr(s, &tio) == 0) {
			cfmakeraw(&tio);
			tcsetattr(s, TCSANOW, &tio);
		}
		close(s);
	}
	return m;
}

/* -----------------------------------------------------------------------
 * struct uart_mux_io callbacks (mirror of port_zephyr.c io_*).
 * ----------------------------------------------------------------------- */
static int io_write_raw(void *ctx, const uint8_t *buf, size_t len)
{
	struct port_ctx *p = ctx;
	size_t off = 0;

	while (off < len && !p->stop) {
		ssize_t w = write(p->uart_fd, buf + off, len - off);

		if (w < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				usleep(200);
				continue;
			}
			perror("uart write");
			return -1;
		}
		off += (size_t)w;
	}
	return 0;
}

/*
 * io_deliver: a complete, FCS-valid mux payload was decoded off the wire.
 * Dispatch it to the registered per-channel RX consumer (the bridge). This is
 * the exact analog of the Zephyr per-channel RX callback dispatch.
 */
static void io_deliver(void *ctx, uint8_t channel,
		       const uint8_t *payload, size_t len)
{
	struct port_ctx *p = ctx;
	struct mux_channel *c = channel_lookup(channel);

	if (!c || !c->wire_rx) {
		logv("[muxd] RX ch=0x%02x len=%zu (no consumer)\n",
		     channel, len);
		return;
	}
	(void)p;
	logv("[muxd] RX ch=0x%02x len=%zu\n", channel, len);
	c->wire_rx(payload, len, c->user);
}

static void io_log(void *ctx, const char *msg)
{
	char tsb[48];

	(void)ctx;
	uart_mux_port_timestamp(tsb, sizeof(tsb));
	fprintf(stderr, "%s[muxd] core: %s\n", tsb, msg);
}


static const struct uart_mux_io linux_io = {
	.write_raw = io_write_raw,
	.deliver   = io_deliver,
	.lock      = NULL,
	.unlock    = NULL,
	.log       = io_log,
	.ctx       = &g_port,
};

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */
int uart_mux_port_init(const char *uart_path, int baud, int flow)
{
	memset(&g_port, 0, sizeof(g_port));

	g_port.uart_fd = uart_open(uart_path, baud, flow);
	if (g_port.uart_fd < 0) {
		return -1;
	}
	fprintf(stderr, "[muxd] UART %s @ %d baud, flow=%s\n",
		uart_path, baud, flow ? "RTS/CTS" : "none");

	/*
	 * Baud-derived preemption fragment size. At 8N1 the wire moves baud/10
	 * bytes per second, so ~1 ms of wire time is baud/10000 bytes. A policy
	 * that leaves frag_bytes == 0 gets this value, keeping the worst-case
	 * BLE preemption latency near 1 ms regardless of the negotiated baud
	 * (e.g. ~92 bytes at 921600, ~200 at 2 Mbaud) instead of a fixed 256
	 * that would be too coarse at low baud and needlessly fine at high baud.
	 * Clamp to a sane 64..1024 window.
	 */
	g_port.baud = baud;
	{
		int f = baud / 10000;

		if (f < 64) {
			f = 64;
		} else if (f > 1024) {
			f = 1024;
		}
		g_port.frag_auto = (uint16_t)f;
	}
	fprintf(stderr, "[muxd] sched: auto fragment = %u bytes (~1 ms wire)\n",
		g_port.frag_auto);

	uart_mux_init(&g_port.core, &linux_io);


	/* Egress scheduler: start on profile 0 (STRICT BLE > Thread + quota). */
	g_port.pending_profile = -1;
	g_active_profile = 0;
	uart_mux_port_sched_set_policy(&sched_profiles[0]);
	fprintf(stderr, "[muxd] sched: profile 0 (STRICT BLE>Thread) active\n");
	return 0;
}

int uart_mux_port_add_channel(uint8_t channel,
			      uart_mux_port_rx_cb_t wire_rx,
			      uart_mux_port_rx_cb_t local_rx,
			      void *user, char *name_out, size_t name_len)
{
	if (channel < UART_MUX_CH_MIN || channel > UART_MUX_CH_MAX) {
		fprintf(stderr, "[muxd] invalid channel 0x%02x\n", channel);

		return -1;
	}

	struct mux_channel *c = &g_port.ch[channel];

	if (c->used) {
		fprintf(stderr, "[muxd] channel 0x%02x already registered\n",
			channel);
		return -1;
	}

	char name[128];

	c->pty_fd = pty_open(name, sizeof(name));
	if (c->pty_fd < 0) {
		return -1;
	}

	c->used = 1;
	c->id = channel;
	c->wire_rx = wire_rx;
	c->local_rx = local_rx;
	c->user = user;
	c->ctrl_q.items = c->ctrl_items;
	c->ctrl_q.cap = PTY_Q_MAX;
	c->split = (channel == UART_MUX_CH_BLE);
	if (c->split) {
		c->adv_q.items = c->adv_items;
		c->adv_q.cap = BLE_ADV_Q_MAX;
	}

	if (name_out && name_len) {
		snprintf(name_out, name_len, "%s", name);
	}
	return 0;
}

/* -----------------------------------------------------------------------
 * Egress scheduler: enqueue, pick-next, drain.
 * ----------------------------------------------------------------------- */

/* Total pending payloads across all egress queues (loop-timeout hint). */
static int egress_pending(void)
{
	int total = 0;

	for (int id = UART_MUX_CH_MIN; id <= UART_MUX_CH_MAX; id++) {
		struct mux_channel *c = &g_port.ch[id];

		if (c->used) {
			total += c->eq.count;
		}
	}
	return total;
}

/*
 * Pick the next channel to service according to the live policy. Returns a
 * channel id in [UART_MUX_CH_MIN, UART_MUX_CH_MAX] with pending egress data,
 * or 0 if every egress queue is empty. Never hard-codes priority: it only
 * reads g_port.policy.
 */
static uint8_t mux_sched_pick_next(void)
{
	struct mux_sched_policy *pol = &g_port.policy;

	if (pol->mode == MUX_SCHED_STRICT) {
		/*
		 * Walk prio_order high-to-low. Track the first (highest) channel
		 * that has data; if a lower-priority channel is starved past the
		 * quota, force it once.
		 */
		uint8_t first = 0;
		uint8_t lower_pending = 0;

		for (int i = 0; i < MUX_SCHED_MAX_CH; i++) {
			uint8_t id = pol->prio_order[i];

			if (id == 0) {
				break;
			}
			struct mux_channel *c = channel_lookup(id);

			if (!c || c->eq.count == 0) {
				continue;
			}
			if (first == 0) {
				first = id;
			} else {
				/* A lower-priority channel also has data. */
				lower_pending = id;
			}
		}
		if (first == 0) {
			return 0;
		}
		if (pol->starve_quota && lower_pending &&
		    g_port.strict_starve >= pol->starve_quota) {
			g_port.strict_starve = 0;
			return lower_pending;
		}
		return first;
	}

	/* WRR: keep serving the current channel until its per-round budget is
	 * spent, then advance to the next weighted channel with data. */
	for (int scan = 0; scan < MUX_SCHED_MAX_CH; scan++) {
		if (g_port.wrr_ch >= UART_MUX_CH_MIN &&
		    g_port.wrr_ch <= UART_MUX_CH_MAX && g_port.wrr_left > 0) {
			struct mux_channel *c = channel_lookup(g_port.wrr_ch);

			if (c && c->eq.count > 0) {
				return g_port.wrr_ch;
			}
		}
		/* Advance to the next channel that has weight and data. */
		g_port.wrr_left = 0;
		for (int step = 0; step < MUX_SCHED_MAX_CH; step++) {
			uint16_t next = g_port.wrr_ch + 1;

			if (next < UART_MUX_CH_MIN || next > UART_MUX_CH_MAX) {
				next = UART_MUX_CH_MIN;
			}
			g_port.wrr_ch = next;
			struct mux_channel *c = channel_lookup((uint8_t)next);

			if (pol->weight[next] > 0 && c && c->eq.count > 0) {
				g_port.wrr_left = pol->weight[next];
				return (uint8_t)next;
			}
		}
		/* No weighted channel has data. */
		return 0;
	}
	return 0;
}

/*
 * Transmit one turn: emit at most frag_bytes of the head payload of the picked
 * channel as a single mux frame, advance its offset, and pop it when fully
 * sent. Returns 1 if a fragment was emitted, 0 if nothing to do.
 */
static int egress_drain_once(void)
{
	uint8_t id = mux_sched_pick_next();

	if (id == 0) {
		return 0;
	}
	struct mux_channel *c = channel_lookup(id);

	if (!c || c->eq.count == 0) {
		return 0;
	}

	struct egress_item *it = &c->eq.items[c->eq.head];
	/*
	 * Fragment size: honour an explicit policy frag_bytes, else fall back to
	 * the baud-derived auto value (~1 ms of wire time). This is where the
	 * head payload is measured for head-of-line latency, on the FIRST
	 * fragment only (off == 0), before any byte of it hits the wire.
	 */
	uint16_t frag = g_port.policy.frag_bytes ? g_port.policy.frag_bytes :
						   g_port.frag_auto;
	uint16_t remain = it->len - it->off;
	uint16_t n = (frag && remain > frag) ? frag : remain;

	if (it->off == 0) {
		uint64_t wait = now_us() - it->enq_us;

		c->stats.hol_us_last = wait;
		c->stats.hol_us_sum += wait;
		c->stats.hol_samples++;
		if (wait > c->stats.hol_us_max) {
			c->stats.hol_us_max = wait;
		}
	}

	if (uart_mux_send(&g_port.core, id, it->data + it->off, n) != 0) {

		/* Wire error: drop the whole payload to avoid a stuck head. */
		c->eq.head = (c->eq.head + 1) % EGRESS_Q_MAX;
		c->eq.count--;
		c->stats.drops++;
		return 1;
	}

	c->stats.tx_frags++;
	c->stats.tx_bytes += n;
	it->off += n;

	/*
	 * Wire-write trace: this is the ONLY place a channel's payload actually
	 * reaches the physical UART (via uart_mux_send above). TX enqueue
	 * logs the egress queueing; this logs the real drain so a frame can be
	 * proven to have hit the wire. "frag" is the bytes emitted this turn,
	 * "sent/len" is the running offset of the logical payload, and "done"
	 * marks the fragment that completed the whole logical payload.
	 */
	logv("[muxd] TX wire ch=0x%02x frag=%u sent=%u/%u done=%d q=%d\n",
	     id, n, it->off, it->len, (it->off >= it->len) ? 1 : 0,
	     c->eq.count - 1);

	if (it->off >= it->len) {
		/* Whole logical payload sent. */
		c->eq.head = (c->eq.head + 1) % EGRESS_Q_MAX;
		c->eq.count--;
		c->stats.tx_frames++;
	}


	/*
	 * STRICT anti-starvation bookkeeping: count a served turn against the
	 * highest-priority channel while a lower-priority one is waiting.
	 */
	if (g_port.policy.mode == MUX_SCHED_STRICT &&
	    g_port.policy.starve_quota) {
		uint8_t top = g_port.policy.prio_order[0];

		if (id == top) {
			g_port.strict_starve++;
		} else {
			g_port.strict_starve = 0;
		}
	}
	if (g_port.policy.mode == MUX_SCHED_WRR && g_port.wrr_left > 0 &&
	    it->off == 0) {
		/* A full payload completed this WRR turn: consume one credit. */
		g_port.wrr_left--;
	}
	return 1;
}

int uart_mux_port_send(uint8_t channel, const uint8_t *buf, size_t len)
{
	struct mux_channel *c = channel_lookup(channel);

	if (!c) {
		return -1;
	}
	if (len > UART_MUX_PAYLOAD_MAX) {
		len = UART_MUX_PAYLOAD_MAX;
	}

	logv("[muxd] TX enqueue ch=0x%02x len=%zu q=%d\n",
	     channel, len, c->eq.count);

	if (c->eq.count == EGRESS_Q_MAX) {
		/* Queue full: drop oldest to keep the freshest data flowing. */
		c->eq.head = (c->eq.head + 1) % EGRESS_Q_MAX;
		c->eq.count--;
		c->stats.drops++;
	}

	int tail = (c->eq.head + c->eq.count) % EGRESS_Q_MAX;
	struct egress_item *it = &c->eq.items[tail];

	it->len = (uint16_t)len;
	it->off = 0;
	it->enq_us = now_us();
	memcpy(it->data, buf, len);
	c->eq.count++;


	c->stats.q_depth = (uint32_t)c->eq.count;
	if (c->stats.q_depth > c->stats.q_max) {
		c->stats.q_max = c->stats.q_depth;
	}
	return 0;
}

/* -----------------------------------------------------------------------
 * Egress scheduler public API.
 * ----------------------------------------------------------------------- */
void uart_mux_port_sched_set_policy(const struct mux_sched_policy *pol)
{
	if (!pol) {
		return;
	}
	g_port.policy = *pol;
	/* Reset WRR bookkeeping so the new weights take effect cleanly. */
	g_port.wrr_ch = 0;
	g_port.wrr_left = 0;
	g_port.strict_starve = 0;
}

void uart_mux_port_sched_get_policy(struct mux_sched_policy *out)
{
	if (out) {
		*out = g_port.policy;
	}
}

int uart_mux_port_sched_select_profile(int idx)
{
	/*
	 * Apply a built-in scheduler profile by index immediately (synchronous,
	 * NOT from a signal handler). Used by the CLI so an operator can pick
	 * the arbitration policy at launch (e.g. --sched 3 for the coex-safe
	 * WRR 2:1). Returns 0 on success, -1 if the index is out of range.
	 */
	if (idx < 0 || idx >= NUM_SCHED_PROFILES) {
		return -1;
	}
	g_active_profile = idx;
	g_port.pending_profile = -1;
	uart_mux_port_sched_set_policy(&sched_profiles[idx]);
	fprintf(stderr, "[muxd] sched: profile %d selected\n", idx);
	return 0;
}

void uart_mux_port_sched_tune(int frag_bytes, int starve_quota)
{
	/*
	 * Override the live policy's fragment size and/or STRICT anti-starvation
	 * quota without changing the arbitration mode or weights. A negative
	 * value leaves the corresponding field untouched; frag_bytes == 0 keeps
	 * the baud-derived auto (~1 ms) fragment. Applied on top of whatever
	 * profile is currently active, so order with select_profile() matters:
	 * select first, then tune.
	 */
	if (frag_bytes >= 0) {
		g_port.policy.frag_bytes = (uint16_t)frag_bytes;
	}
	if (starve_quota >= 0) {
		g_port.policy.starve_quota = (uint16_t)starve_quota;
	}
	fprintf(stderr,
		"[muxd] sched: tuned frag_bytes=%u starve_quota=%u\n",
		g_port.policy.frag_bytes, g_port.policy.starve_quota);
}

int uart_mux_port_sched_cycle_profile(void)
{

	/*
	 * Async-signal-safe: only advance and stash a pending index; the poll
	 * loop applies it via uart_mux_port_sched_reevaluate(). Reading
	 * g_active_profile here is racy against the loop but harmless for a
	 * bring-up control, and sig_atomic_t makes the store atomic.
	 */
	int next = (g_active_profile + 1) % NUM_SCHED_PROFILES;

	g_port.pending_profile = next;
	return next;
}

void uart_mux_port_sched_reevaluate(void)
{
	/*
	 * Level 2 placeholder: a future closed-loop adaptation would inspect
	 * per-channel queue depth / head-of-line age / an in-flight BLE connect
	 * here and pick a policy. Today it only applies a profile switch that
	 * was requested from a signal handler.
	 */
	int pending = g_port.pending_profile;

	if (pending >= 0 && pending < NUM_SCHED_PROFILES) {
		g_port.pending_profile = -1;
		g_active_profile = pending;
		uart_mux_port_sched_set_policy(&sched_profiles[pending]);
		fprintf(stderr, "[muxd] sched: profile %d active\n", pending);
		uart_mux_port_sched_dump_stats();
	}
}

int uart_mux_port_sched_get_stats(uint8_t channel,
				  struct mux_sched_ch_stats *out)
{
	struct mux_channel *c = channel_lookup(channel);

	if (!c || !out) {
		return -1;
	}
	*out = c->stats;
	return 0;
}

void uart_mux_port_sched_dump_stats(void)
{
	/*
	 * Print a wall-clock (CLOCK_REALTIME) header before the per-channel
	 * loop so this dump can be lined up with the other logs. btmon
	 * timestamps its records with the system clock, and the coex
	 * orchestrator prefixes every log line with the same date format, so
	 * the three separate files (muxd.log, coex.log, btmon.snoop) are
	 * correlated by matching absolute timestamps (there is no combined
	 * file). The HOL numbers themselves stay CLOCK_MONOTONIC (NTP-immune);
	 * this REALTIME header is only the correlation anchor. Keep the host
	 * clock steady during a run (avoid an NTP step) so the alignment holds.
	 */
	struct timespec rt;
	struct tm tmv;
	char tbuf[64];

	clock_gettime(CLOCK_REALTIME, &rt);
	localtime_r(&rt.tv_sec, &tmv);
	strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", &tmv);
	fprintf(stderr, "[muxd] sched dump @ %s.%03ld (epoch %lld.%03ld)\n",
		tbuf, rt.tv_nsec / 1000000L,
		(long long)rt.tv_sec, rt.tv_nsec / 1000000L);

	for (int id = UART_MUX_CH_MIN; id <= UART_MUX_CH_MAX; id++) {

		struct mux_channel *c = &g_port.ch[id];

		if (!c->used) {
			continue;
		}
		struct mux_sched_ch_stats *s = &c->stats;
		uint64_t hol_mean = s->hol_samples ?
				    s->hol_us_sum / s->hol_samples : 0;

		fprintf(stderr,
			"[muxd] sched ch=0x%02x frames=%llu frags=%llu "
			"bytes=%llu drops=%llu q=%u qmax=%u "
			"hol_us(last=%llu mean=%llu max=%llu)\n",
			id,
			(unsigned long long)s->tx_frames,
			(unsigned long long)s->tx_frags,
			(unsigned long long)s->tx_bytes,
			(unsigned long long)s->drops,
			s->q_depth, s->q_max,
			(unsigned long long)s->hol_us_last,
			(unsigned long long)hol_mean,
			(unsigned long long)s->hol_us_max);
	}
}



int uart_mux_port_local_out(uint8_t channel, const uint8_t *buf, size_t len)
{
	struct mux_channel *c = channel_lookup(channel);

	if (!c) {
		return -1;
	}
	if (c->split && is_ble_adv_report(buf, len)) {
		q_push_drop_oldest(&c->adv_q, buf, len);

	} else {
		q_push_drop_oldest(&c->ctrl_q, buf, len);
	}
	channel_drain(c);
	return 0;
}

void uart_mux_port_stop(void)
{
	g_port.stop = 1;
}

int uart_mux_port_run(void)
{
	uint8_t buf[4096];
	struct pollfd pfd[1 + MAX_CHANNELS];
	struct mux_channel *pfd_ch[1 + MAX_CHANNELS];
	int nfds;

	while (!g_port.stop) {
		nfds = 0;

		pfd[nfds].fd = g_port.uart_fd;
		pfd[nfds].events = POLLIN;
		pfd_ch[nfds] = NULL;
		nfds++;

		for (int id = UART_MUX_CH_MIN; id <= UART_MUX_CH_MAX; id++) {
			struct mux_channel *c = &g_port.ch[id];

			if (!c->used) {
				continue;
			}
			short ev = POLLIN;

			if (c->ctrl_q.count || (c->split && c->adv_q.count)) {
				ev |= POLLOUT;
			}
			pfd[nfds].fd = c->pty_fd;
			pfd[nfds].events = ev;
			pfd_ch[nfds] = c;
			nfds++;
		}

		/*
		 * While egress payloads are pending, use a zero timeout so the
		 * loop keeps returning to the drainer and BLE can preempt Thread
		 * between fragments. Otherwise block up to 500 ms.
		 */
		int timeout = egress_pending() ? 0 : 500;
		int r = poll(pfd, (nfds_t)nfds, timeout);

		if (r < 0) {
			if (errno == EINTR) {
				/* Likely a scheduler control signal (SIGUSR*). */
				uart_mux_port_sched_reevaluate();
				continue;
			}
			perror("poll");
			return -1;
		}

		/* Level 2 hook + apply any pending profile switch requested from
		 * a signal handler; also refresh exported stats. */
		uart_mux_port_sched_reevaluate();

		/* Egress: transmit one fragment per iteration so a freshly
		 * queued higher-priority frame is re-picked on the next turn. */
		egress_drain_once();

		if (r == 0) {
			continue;
		}


		/* UART -> core -> bridges (via io_deliver) -> PTYs. */
		if (pfd[0].revents & POLLIN) {
			ssize_t n = read(g_port.uart_fd, buf, sizeof(buf));

			if (n > 0) {
				uart_mux_rx_bytes(&g_port.core, buf,
						       (size_t)n);
			}
		}

		/* Per-channel PTY -> bridge (local -> wire) and drain. */
		for (int i = 1; i < nfds; i++) {
			struct mux_channel *c = pfd_ch[i];

			if (pfd[i].revents & POLLIN) {
				ssize_t n = read(c->pty_fd, buf, sizeof(buf));

				if (n > 0 && c->local_rx) {
					c->local_rx(buf, (size_t)n, c->user);
				}
			}
			if ((pfd[i].revents & POLLOUT) || c->ctrl_q.count ||
			    (c->split && c->adv_q.count)) {
				channel_drain(c);
			}
			if (pfd[i].revents & (POLLERR | POLLNVAL)) {
				fprintf(stderr, "[muxd] poll error on fd %d\n",
					c->pty_fd);
			}
		}
	}
	return 0;
}

void uart_mux_port_cleanup(void)
{
	if (g_port.uart_fd > 0) {
		close(g_port.uart_fd);
		g_port.uart_fd = -1;
	}
	for (int id = UART_MUX_CH_MIN; id <= UART_MUX_CH_MAX; id++) {
		struct mux_channel *c = &g_port.ch[id];

		if (c->used && c->pty_fd > 0) {
			close(c->pty_fd);
			c->pty_fd = -1;
		}
	}
}
