/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * port_linux.h - Linux port API over the shared uart_mux library.
 *
 * This is the exact mirror of port_zephyr.h. The application (hci/spinel
 * bridges in muxd.c) uses only this API; it never touches the uart_mux
 * internals or any OS primitive directly. Where the Zephyr port binds the
 * shared core to the async UART DMA backend and delivers per-channel payloads
 * to registered consumers, the Linux port binds it to a raw UART fd and to one
 * PTY per logical channel, so the same bridge/application code can be shared
 * across both operating systems.
 *
 * The three core operations map 1:1 to the Zephyr port:
 *
 *   uart_mux_port_init      : open + configure the physical UART (Zephyr: bind
 *                             the async DMA backend and start RX).
 *   uart_mux_port_add_channel: create a PTY endpoint for a logical channel and
 *                             register its RX consumer (Zephyr: register a
 *                             per-channel RX callback).
 *   uart_mux_port_send      : assemble and transmit a frame on a channel.
 *
 * Two Linux-only helpers cover the parts that have no Zephyr equivalent: the
 * outbound delivery to a PTY consumer (uart_mux_port_local_out, the Linux
 * counterpart of writing to the local endpoint) and the poll() event loop
 * (uart_mux_port_run), which on Zephyr is provided by the RTOS.
 */

#ifndef PORT_LINUX_H_
#define PORT_LINUX_H_

#include <stddef.h>
#include <stdint.h>

#include "uart_mux_int.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * Egress (host -> MCXW72) channel scheduler.
 *
 * Historically uart_mux_port_send() assembled a frame and wrote it to the
 * physical UART immediately, first-come first-served, with no arbitration
 * between the BLE (latency critical, e.g. audio connection events) and the
 * Thread (bursty Spinel) channels. A long Spinel frame that started first
 * therefore delayed the next BLE frame by its whole on-wire time.
 *
 * The scheduler decouples "enqueue" from "transmit": uart_mux_port_send() now
 * queues the payload on a per-channel egress FIFO, and a drainer in the poll
 * loop picks the next channel to service according to a runtime-modifiable
 * policy, transmitting at most frag_bytes per turn so a large Thread frame can
 * be preempted between fragments by a freshly queued BLE frame. Both bridges
 * reassemble a byte stream on their from_mux path, so fragmenting a payload
 * across several mux frames is transparent to them.
 *
 * The policy lives in a struct (never hard-coded in the picker) so it can be
 * changed at runtime: the default is strict priority BLE > Thread with a
 * Thread anti-starvation quota, but a WRR mode and an inverted (Thread first)
 * order are also selectable. uart_mux_port_sched_reevaluate() is a no-op hook
 * reserved for a future closed-loop auto-adaptation (level 2) that would flip
 * the policy based on queue depth / frame age / an in-flight BLE connect.
 * ----------------------------------------------------------------------- */

/* Scheduler arbitration mode. */
enum mux_sched_mode {
	MUX_SCHED_STRICT = 0, /* strict priority by prio_order[] */
	MUX_SCHED_WRR    = 1, /* weighted round-robin by weight[] */
};

/* Number of channel slots (index by channel id, 0 unused). */
#define MUX_SCHED_MAX_CH (UART_MUX_CH_MAX + 1)

/*
 * Runtime egress policy. Consulted by the picker on every turn; may be
 * replaced live via uart_mux_port_sched_set_policy() (or cycled with
 * uart_mux_port_sched_cycle_profile()).
 */
struct mux_sched_policy {
	enum mux_sched_mode mode;

	/* STRICT: channels served in this order (highest priority first).
	 * Terminated by a 0 entry. */
	uint8_t  prio_order[MUX_SCHED_MAX_CH];

	/* WRR: per-channel weight (frames per round). 0 = channel excluded. */
	uint16_t weight[MUX_SCHED_MAX_CH];

	/* Max payload bytes transmitted per turn before re-picking (egress
	 * fragmentation). Smaller = finer preemption, more framing overhead. */
	uint16_t frag_bytes;

	/*
	 * STRICT anti-starvation: after this many consecutive higher-priority
	 * frames are served while a lower-priority channel has pending data,
	 * force one lower-priority frame through. 0 disables (pure strict).
	 */
	uint16_t starve_quota;
};

/* Per-channel egress counters (diagnostics / future auto policy input). */
struct mux_sched_ch_stats {
	uint64_t tx_frames;  /* fully transmitted logical payloads */
	uint64_t tx_frags;   /* mux frames emitted (>= tx_frames) */
	uint64_t tx_bytes;   /* payload bytes transmitted */
	uint64_t drops;      /* payloads dropped (egress queue full) */
	uint32_t q_depth;    /* current queued payloads */
	uint32_t q_max;      /* high-water mark of q_depth */
	/*
	 * Head-of-line latency: microseconds a payload waited between being
	 * enqueued by uart_mux_port_send() and its FIRST fragment reaching the
	 * wire. This is the objective per-channel quality metric of the
	 * scheduler (e.g. BLE HOL must stay well under a connInterval). The
	 * iMX93 timestamps every enqueue with a monotonic clock; the cost is
	 * negligible on the host and never touches the MCXW72.
	 */
	uint64_t hol_us_last; /* last measured HOL latency */
	uint64_t hol_us_max;  /* worst-case HOL latency seen */
	uint64_t hol_samples; /* number of HOL measurements */
	uint64_t hol_us_sum;  /* sum of HOL latencies (for mean) */
};

/*
 * wire_rx : a complete, FCS-valid payload was decoded off the mux wire for this
 *           channel (host <- MCXW72). The application feeds it to the bridge

 *           from_mux path. Mirror of the Zephyr per-channel RX callback.
 * local_rx: raw bytes were read from this channel's local PTY endpoint
 *           (host -> MCXW72). The application feeds them to the bridge
 *           from_local path. On Zephyr this input comes from the BLE controller
 *           / OT stack directly and has no port callback.
 */
typedef void (*uart_mux_port_rx_cb_t)(const uint8_t *buf, size_t len,
				      void *user);

/* Enable verbose diagnostics on stderr (call before init). */
void uart_mux_port_set_verbose(int on);

/*
 * Build a system wall-clock (CLOCK_REALTIME) timestamp prefix of the form
 * "[YYYY-MM-DD HH:MM:SS.mmm] " into buf, using the SAME format as the sched
 * dump header and the coex orchestrator line prefix. Callers prepend this to
 * their own log lines so muxd.log, coex.log and otbr-agent.log all carry the
 * same absolute timestamp axis and can be correlated after a run. On any
 * failure buf is set to an empty string.
 */
void uart_mux_port_timestamp(char *buf, size_t n);



/*
 * Open and configure the physical UART to the MCXW72, then bind the shared
 * uart_mux to it. flow != 0 enables RTS/CTS. Returns 0 on success.
 */
int uart_mux_port_init(const char *uart_path, int baud, int flow);

/*
 * Create a PTY endpoint bound to a logical mux channel (channel in 0x01..0x05)
 * and register its two RX consumers:
 *
 *   wire_rx : invoked for every complete, FCS-valid payload decoded off the
 *             mux wire (host <- MCXW72). The application feeds it to the
 *             bridge from_mux path.
 *   local_rx: invoked for every chunk of raw bytes read from this channel's
 *             local PTY endpoint (host -> MCXW72). The application feeds it to
 *             the bridge from_local path.
 *
 * Either callback may be NULL. The slave device path (e.g. /dev/pts/3) is
 * copied into name_out. Returns 0 on success.
 */
int uart_mux_port_add_channel(uint8_t channel,
			      uart_mux_port_rx_cb_t wire_rx,
			      uart_mux_port_rx_cb_t local_rx,
			      void *user, char *name_out, size_t name_len);

/* Assemble and transmit a frame on the given channel (mux TX to the wire). */
int uart_mux_port_send(uint8_t channel, const uint8_t *buf, size_t len);

/*
 * Queue a decoded frame for delivery to the local PTY consumer of a channel.
 * This is the Linux counterpart of the Zephyr "to_local" endpoint: it never
 * blocks the UART RX path on a slow PTY reader. Returns 0 on success.
 */
int uart_mux_port_local_out(uint8_t channel, const uint8_t *buf, size_t len);

/* Run the poll() event loop until uart_mux_port_stop() is requested. */
int uart_mux_port_run(void);

/* Ask the poll loop to stop (safe from a signal handler). */
void uart_mux_port_stop(void);

/* Close all file descriptors opened by the port. */
void uart_mux_port_cleanup(void);

/* -----------------------------------------------------------------------
 * Egress scheduler API.
 * ----------------------------------------------------------------------- */

/*
 * Replace the live egress policy. The picker reads the policy on every turn, so
 * the change takes effect on the next scheduling decision. Passing NULL is a
 * no-op. The struct is copied, the caller keeps ownership.
 */
void uart_mux_port_sched_set_policy(const struct mux_sched_policy *pol);

/* Copy the live egress policy out (for inspection / tweak-then-set). */
void uart_mux_port_sched_get_policy(struct mux_sched_policy *out);

/*
 * Select a built-in scheduler profile by index and apply it immediately
 * (synchronous, not from a signal handler). Profiles:
 *   0 STRICT BLE>Thread (quota 4, 256 B frag)
 *   1 WRR BLE:Thread 3:1 (256 B frag)
 *   2 STRICT Thread>BLE (quota 4, 256 B frag)
 *   3 COEX-SAFE WRR BLE:Thread 2:1 (auto ~1 ms frag) - recommended for coex
 * Returns 0 on success, -1 if idx is out of range. Intended for CLI selection
 * at launch (e.g. --sched 3).
 */
int uart_mux_port_sched_select_profile(int idx);

/*
 * Override the live policy fragment size and/or STRICT anti-starvation quota
 * on top of the currently selected profile. A negative argument leaves that
 * field untouched; frag_bytes == 0 keeps the baud-derived auto (~1 ms)
 * fragment. Call AFTER uart_mux_port_sched_select_profile() if both are used.
 */
void uart_mux_port_sched_tune(int frag_bytes, int starve_quota);


/*
 * Cycle to the next built-in scheduler profile and return its index. Intended
 * to be driven from a signal handler (e.g. SIGUSR1) during bring-up/tuning.
 * Async-signal-safe: only sets a pending index consumed by the poll loop.
 */
int uart_mux_port_sched_cycle_profile(void);

/*
 * No-op hook reserved for future closed-loop auto-adaptation (level 2): called
 * once per poll iteration so a later implementation can flip the policy based
 * on queue depth, head-of-line frame age or an in-flight BLE connect. Today it
 * only applies a pending profile requested via cycle_profile() and refreshes
 * the exported stats.
 */
void uart_mux_port_sched_reevaluate(void);

/* Copy a channel's egress counters out. Returns 0 on success. */
int uart_mux_port_sched_get_stats(uint8_t channel,
				  struct mux_sched_ch_stats *out);

/* Log a one-line summary of the egress scheduler stats to stderr. */
void uart_mux_port_sched_dump_stats(void);

#ifdef __cplusplus
}
#endif


#endif /* PORT_LINUX_H_ */
