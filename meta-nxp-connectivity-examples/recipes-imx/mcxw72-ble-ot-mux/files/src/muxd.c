/*
 * muxd - HCI/Spinel UART multiplexer daemon for MCXW72 (Linux host side)
 *
 * This file is now a THIN wire-up over the Linux OS port (port_linux.c) and the
 * SHARED bridges under src/shared/, which are the EXACT SAME C files compiled
 * by the Zephyr companion app (samples/net/openthread/ble_ot_rcp_mux_core):
 *
 *   shared/uart_mux.c  : mux framing, FCS16, RX state machine, TX assembly
 *   shared/hci_bridge.c     : BLE HCI H4 framer, adv/ctrl split, NBU 0x29 fixup
 *   shared/spinel_bridge.c  : OpenThread Spinel HDLC framer
 *
 * The symmetry with the Zephyr side is exact: main() here plays the same role
 * as the Zephyr companion main.c. It creates the two bridge instances (HCI in
 * HOST role, Spinel), fills struct bridge_ops so the bridges reach the wire
 * (mux_send -> uart_mux_port_send) and the local endpoint (to_local ->
 * uart_mux_port_local_out), then registers per-channel callbacks on the port:
 *
 *   wire_rx  : payload decoded off the mux wire  -> *_bridge_from_mux
 *   local_rx : raw bytes read from the local PTY -> *_bridge_from_local
 *
 * All OS specifics (UART/PTY setup, non-blocking PTY output queues, poll loop)
 * live in port_linux.c. This file contains ZERO OS primitives beyond argv
 * parsing and signal handling.
 *
 * Build:  see Makefile (compiles src/muxd.c + src/port_linux.c + src/shared/*).
 * Usage:  ./muxd --uart /dev/ttyLP4 --baud 921600 [--no-flow] [-v]
 *         ./muxd --replay <hexfile> [--out <base>] [-v]
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "port_linux.h"
#include "uart_mux.h"
#include "hci_bridge.h"
#include "spinel_bridge.h"

static int g_verbose;

/* -----------------------------------------------------------------------
 * Bridge instances. One HCI (HOST role) and one Spinel bridge, both fed by
 * the shared sources exactly as on the Zephyr side.
 * ----------------------------------------------------------------------- */
static struct hci_bridge    g_hci;
static struct spinel_bridge g_spinel;

/* -----------------------------------------------------------------------
 * struct bridge_ops callbacks (mirror of the Zephyr companion main.c).
 *
 * mux_send : push a framed payload onto a logical mux channel (to the wire).
 * to_local : hand a decoded, direction-correct frame to the local endpoint;
 *            the port queues it to the channel PTY without blocking the UART
 *            RX path on a slow reader.
 * ----------------------------------------------------------------------- */
static int bridge_mux_send(void *ctx, uint8_t channel,
			   const uint8_t *buf, size_t len)
{
	(void)ctx;
	return uart_mux_port_send(channel, buf, len);
}

static void hci_to_local(void *ctx, const uint8_t *buf, size_t len)
{
	(void)ctx;
	uart_mux_port_local_out(UART_MUX_CH_BLE, buf, len);
}

static void spinel_to_local(void *ctx, const uint8_t *buf, size_t len)
{
	(void)ctx;
	uart_mux_port_local_out(UART_MUX_CH_THREAD, buf, len);
}

static void bridge_sleep_ms(void *ctx, uint32_t ms)
{
	(void)ctx;
	if (ms == 0) {
		return;
	}
	struct timespec ts;

	ts.tv_sec  = ms / 1000;
	ts.tv_nsec = (long)(ms % 1000) * 1000000L;
	nanosleep(&ts, NULL);
}

static void bridge_log(void *ctx, const char *msg)
{
	(void)ctx;
	if (g_verbose) {
		char tsb[48];

		uart_mux_port_timestamp(tsb, sizeof(tsb));
		fprintf(stderr, "%s[muxd] bridge: %s\n", tsb, msg);
	}
}


static const struct bridge_ops hci_ops = {
	.mux_send = bridge_mux_send,
	.to_local = hci_to_local,
	.sleep_ms = bridge_sleep_ms,
	.now_ms   = NULL,
	.log      = bridge_log,
	.ctx      = NULL,
};

static const struct bridge_ops spinel_ops = {
	.mux_send = bridge_mux_send,
	.to_local = spinel_to_local,
	.sleep_ms = bridge_sleep_ms,
	.now_ms   = NULL,
	.log      = bridge_log,
	.ctx      = NULL,
};

/* -----------------------------------------------------------------------
 * Per-channel port callbacks.
 *
 *   wire_rx  : payload decoded off the mux wire (host <- MCXW72) -> from_mux.
 *   local_rx : raw bytes read from the channel PTY (host -> MCXW72) -> from_local.
 * ----------------------------------------------------------------------- */
static void ble_wire_rx(const uint8_t *buf, size_t len, void *user)
{
	(void)user;
	hci_bridge_from_mux(&g_hci, buf, len);
}

static void ble_local_rx(const uint8_t *buf, size_t len, void *user)
{
	(void)user;
	hci_bridge_from_local(&g_hci, buf, len);
}

static void thread_wire_rx(const uint8_t *buf, size_t len, void *user)
{
	(void)user;
	spinel_bridge_from_mux(&g_spinel, buf, len);
}

static void thread_local_rx(const uint8_t *buf, size_t len, void *user)
{
	(void)user;
	spinel_bridge_from_local(&g_spinel, buf, len);
}

/* -----------------------------------------------------------------------
 * Signal handling: ask the port poll loop to stop.
 * ----------------------------------------------------------------------- */
static void on_sig(int s)
{
	(void)s;
	uart_mux_port_stop();
}

/*
 * SIGUSR1: cycle to the next built-in egress scheduler profile (bring-up /
 * tuning control). SIGUSR2: dump the per-channel egress counters. Both only
 * call async-signal-safe port helpers (cycle sets a pending index applied by
 * the poll loop; dump writes to stderr).
 */
static void on_sigusr1(int s)
{
	(void)s;
	uart_mux_port_sched_cycle_profile();
}

static void on_sigusr2(int s)
{
	(void)s;
	uart_mux_port_sched_dump_stats();
}


/* -----------------------------------------------------------------------
 * Offline replay mode: decode a hex dump of the UART RX stream and write the
 * recovered per-channel frames to <out>_ble.bin / <out>_thread.bin. This uses
 * the exact same shared core+bridges as the live path (HOST role, so the
 * controller->host 0x29 fixup runs identically), validating the wire format
 * bit-for-bit against the Zephyr side WITHOUT touching any UART/PTY.
 * ----------------------------------------------------------------------- */
static int hexnib(int ch)
{
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	}
	if (ch >= 'a' && ch <= 'f') {
		return ch - 'a' + 10;
	}
	if (ch >= 'A' && ch <= 'F') {
		return ch - 'A' + 10;
	}
	return -1;
}

static int g_replay_ble_fd = -1;
static int g_replay_thr_fd = -1;
static struct uart_mux g_replay_core;

static void replay_hci_to_local(void *ctx, const uint8_t *buf, size_t len)
{
	(void)ctx;
	if (g_replay_ble_fd >= 0) {
		(void)!write(g_replay_ble_fd, buf, len);
	}
}

static void replay_spinel_to_local(void *ctx, const uint8_t *buf, size_t len)
{
	(void)ctx;
	if (g_replay_thr_fd >= 0) {
		(void)!write(g_replay_thr_fd, buf, len);
	}
}

/* Replay io: the core only needs deliver() to route decoded payloads into the
 * bridges; write_raw is never exercised on an offline decode. */
static void replay_deliver(void *ctx, uint8_t channel,
			   const uint8_t *payload, size_t len)
{
	(void)ctx;
	switch (channel) {
	case UART_MUX_CH_BLE:
		hci_bridge_from_mux(&g_hci, payload, len);
		break;
	case UART_MUX_CH_THREAD:
		spinel_bridge_from_mux(&g_spinel, payload, len);
		break;
	default:
		break;
	}
}

static int replay_write_raw(void *ctx, const uint8_t *buf, size_t len)
{
	(void)ctx;
	(void)buf;
	(void)len;
	return 0;
}

static const struct uart_mux_io replay_io = {
	.write_raw = replay_write_raw,
	.deliver   = replay_deliver,
	.lock      = NULL,
	.unlock    = NULL,
	.log       = NULL,
	.ctx       = NULL,
};

static int run_replay(const char *inpath, const char *outbase)
{
	FILE *fp = fopen(inpath, "rb");

	if (!fp) {
		perror("open replay file");
		return 1;
	}

	char ble_path[256], thr_path[256];

	snprintf(ble_path, sizeof ble_path, "%s_ble.bin",    outbase);
	snprintf(thr_path, sizeof thr_path, "%s_thread.bin", outbase);
	g_replay_ble_fd = open(ble_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	g_replay_thr_fd = open(thr_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (g_replay_ble_fd < 0 || g_replay_thr_fd < 0) {
		perror("open out");
		fclose(fp);
		return 1;
	}

	uint8_t *bytes = NULL;
	size_t cap = 0, nb = 0;
	int c, hi = -1, in_comment = 0, prev = 0;

	while ((c = fgetc(fp)) != EOF) {
		if (in_comment) {
			if (c == '\n') {
				in_comment = 0;
			}
			continue;
		}
		if (c == '#') {
			in_comment = 1;
			continue;
		}
		if (c == '/' && prev == '/') {
			in_comment = 1;
			prev = 0;
			continue;
		}
		prev = c;
		int v = hexnib(c);

		if (v < 0) {
			hi = -1;
			continue;
		}
		if (hi < 0) {
			hi = v;
		} else {
			uint8_t byte = (uint8_t)((hi << 4) | v);

			hi = -1;
			if (nb >= cap) {
				cap = cap ? cap * 2 : 4096;
				bytes = realloc(bytes, cap);
			}
			bytes[nb++] = byte;
		}
	}
	fclose(fp);
	fprintf(stderr, "[muxd] replay: parsed %zu bytes from %s\n", nb, inpath);

	/* Build the shared stack with replay to_local sinks. HOST role so the
	 * controller->host 0x29 fixup runs exactly as on the live path. */
	struct bridge_ops rhci = hci_ops;
	struct bridge_ops rspin = spinel_ops;

	rhci.to_local = replay_hci_to_local;
	rspin.to_local = replay_spinel_to_local;

	uart_mux_init(&g_replay_core, &replay_io);
	hci_bridge_init(&g_hci, HCI_BRIDGE_HOST, &rhci);
	spinel_bridge_init(&g_spinel, &rspin);

	uart_mux_rx_bytes(&g_replay_core, bytes, nb);

	free(bytes);
	close(g_replay_ble_fd);
	close(g_replay_thr_fd);
	fprintf(stderr, "[muxd] replay: wrote %s and %s\n", ble_path, thr_path);
	return 0;
}

/* -----------------------------------------------------------------------
 * main
 * ----------------------------------------------------------------------- */
static void usage(const char *p)
{
	fprintf(stderr,
		"Usage: %s --uart <dev> [--baud N] [--no-flow] [-v]\n"
		"       %s --replay <hexfile> [--out <base>] [-v]\n"
		"  --uart <dev>      physical UART to MCXW72 (e.g. /dev/ttyLP4)\n"
		"  --baud N          baud rate (default 921600)\n"
		"  --no-flow         disable RTS/CTS (3-wire bring-up only)\n"
		"  --sched N         egress scheduler profile (0..3; default 0):\n"
		"                      0 STRICT BLE>Thread, 1 WRR 3:1, 2 STRICT Thread>BLE,\n"
		"                      3 COEX-SAFE WRR BLE:Thread 2:1 (recommended for coex)\n"
		"  --frag N          override per-turn fragment size in bytes (0 = baud auto)\n"
		"  --quota N         override STRICT anti-starvation quota (0 = pure strict)\n"
		"  --replay <file>   offline: decode a hex dump of the UART RX stream\n"
		"  --out <base>      replay output base name (default 'replay')\n"
		"  -v                verbose\n", p, p);

}

int main(int argc, char **argv)
{
	const char *uart_path = NULL;
	const char *replay_path = NULL;
	const char *out_base = "replay";
	int baud = 921600, flow = 1;
	/* Egress scheduler CLI overrides. -1 means "leave the init default", so
	 * omitting these flags keeps the historical behaviour (profile 0, auto
	 * fragment, profile-defined quota). */
	int sched = -1, frag = -1, quota = -1;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--uart") && i + 1 < argc) {
			uart_path = argv[++i];
		} else if (!strcmp(argv[i], "--baud") && i + 1 < argc) {
			baud = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--sched") && i + 1 < argc) {
			sched = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--frag") && i + 1 < argc) {
			frag = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--quota") && i + 1 < argc) {
			quota = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--replay") && i + 1 < argc) {
			replay_path = argv[++i];
		} else if (!strcmp(argv[i], "--out") && i + 1 < argc) {
			out_base = argv[++i];
		} else if (!strcmp(argv[i], "--no-flow")) {
			flow = 0;
		} else if (!strcmp(argv[i], "-v")) {
			g_verbose = 1;
		} else {
			usage(argv[0]);
			return 2;
		}
	}


	if (replay_path) {
		fprintf(stderr, "[muxd] replay mode (shared core+bridges)\n");
		return run_replay(replay_path, out_base);
	}

	if (!uart_path) {
		usage(argv[0]);
		return 2;
	}

	signal(SIGINT,  on_sig);
	signal(SIGTERM, on_sig);
	signal(SIGPIPE, SIG_IGN);
	/* Runtime egress scheduler control during bring-up/tuning:
	 *   kill -USR1 <pid>  -> cycle STRICT BLE>Thread / WRR / STRICT Thread>BLE
	 *   kill -USR2 <pid>  -> dump per-channel egress stats to stderr */
	signal(SIGUSR1, on_sigusr1);
	signal(SIGUSR2, on_sigusr2);


	uart_mux_port_set_verbose(g_verbose);

	/* Open the physical UART and bind the shared core (port_linux.c). */
	if (uart_mux_port_init(uart_path, baud, flow) != 0) {
		return 1;
	}

	/* Apply any CLI egress scheduler overrides. uart_mux_port_init() has
	 * already installed profile 0 (STRICT BLE>Thread), so select the
	 * requested profile first, then layer frag/quota tweaks on top. */
	if (sched >= 0) {
		if (uart_mux_port_sched_select_profile(sched) != 0) {
			fprintf(stderr,
				"[muxd] --sched %d out of range (expected 0..3)\n",
				sched);
			uart_mux_port_cleanup();
			return 1;
		}
	}
	if (frag >= 0 || quota >= 0) {
		uart_mux_port_sched_tune(frag, quota);
	}

	/* Bring up the SHARED bridges. HOST role: the local endpoint is the
	 * host stack (BlueZ / OT RCP driver). Same C files as the Zephyr NBU
	 * side. */
	hci_bridge_init(&g_hci, HCI_BRIDGE_HOST, &hci_ops);
	spinel_bridge_init(&g_spinel, &spinel_ops);

	/* Create the per-channel PTYs and wire both RX directions. */
	char ble_name[128], thr_name[128];

	if (uart_mux_port_add_channel(UART_MUX_CH_BLE, ble_wire_rx,
				      ble_local_rx, NULL,
				      ble_name, sizeof(ble_name)) != 0) {
		uart_mux_port_cleanup();
		return 1;
	}
	if (uart_mux_port_add_channel(UART_MUX_CH_THREAD, thread_wire_rx,
				      thread_local_rx, NULL,
				      thr_name, sizeof(thr_name)) != 0) {
		uart_mux_port_cleanup();
		return 1;
	}

	fprintf(stderr, "[muxd] BLE/HCI PTY -> %s\n", ble_name);
	fprintf(stderr, "[muxd] THREAD  PTY -> %s\n", thr_name);
	fprintf(stderr,
		"[muxd] ready.\n"
		"  BlueZ     : btattach -B %s -P h4 &\n"
		"  OpenThread: otbr-agent ... 'spinel+hdlc+uart://%s?uart-baudrate=%d' &\n",
		ble_name, thr_name, baud);

	uart_mux_port_run();

	fprintf(stderr, "\n[muxd] stopping.\n");
	uart_mux_port_cleanup();
	return 0;
}
