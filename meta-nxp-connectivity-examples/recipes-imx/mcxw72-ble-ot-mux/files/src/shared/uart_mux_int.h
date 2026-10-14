/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * uart_mux_int.h - OS-agnostic UART channel multiplexer core (frozen wire v1).
 *
 * This is pure C99 with ZERO OS dependency. The exact same source file is
 * compiled on Zephyr (backend = async UART DMA + workqueue) and on Linux
 * (backend = poll() over a PTY / raw UART). All OS specifics (byte I/O,
 * locking, timing, logging) are injected through the uart_mux_io callback
 * struct, so the framing, RX state machine, TX assembly and FCS logic stay
 * bit-identical across both operating systems.
 *
 * ON-WIRE FRAME FORMAT (identical Zephyr <-> Linux):
 *   [SYNC 0x7E][CHANNEL 1B][FLAGS 1B][LENGTH 2B LE][PAYLOAD][FCS16 2B LE][SYNC 0x7E]
 *   FCS16: CRC-16/CCITT reflected, poly 0x8408, init 0xFFFF, no final XOR,
 *          computed over CHANNEL + FLAGS + LENGTH + PAYLOAD.
 */

#ifndef UART_MUX_H_
#define UART_MUX_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- shared frame constants ---- */
#define UART_MUX_SYNC        0x7EU
#define UART_MUX_FLAGS_VER   0x01U   /* bit0 = protocol version */

/* Logical channel ids. */
#define UART_MUX_CH_THREAD   0x01U   /* Spinel / HDLC */
#define UART_MUX_CH_BLE      0x02U   /* HCI H4        */
#define UART_MUX_CH_CONTROL  0x03U   /* internal      */
#define UART_MUX_CH_DIAG     0x04U   /* internal      */
#define UART_MUX_CH_OTA      0x05U   /* internal      */
#define UART_MUX_CH_MIN      UART_MUX_CH_THREAD
#define UART_MUX_CH_MAX      UART_MUX_CH_OTA

#ifndef UART_MUX_PAYLOAD_MAX
#define UART_MUX_PAYLOAD_MAX 2048
#endif

/* SYNC + CH + FLAGS + LEN(2) + PAYLOAD + FCS(2) + SYNC */
#define UART_MUX_FRAME_MAX   (1 + 1 + 1 + 2 + UART_MUX_PAYLOAD_MAX + 2 + 1)

/*
 * OS abstraction. The port (Zephyr or Linux) fills this in.
 *
 *   write_raw : push assembled bytes to the physical link. On Zephyr this is
 *               the async DMA TX (blocking until UART_TX_DONE); on Linux it is
 *               write() to the UART fd. Return 0 on success, negative on error.
 *   deliver   : hand a fully decoded, FCS-valid payload to the channel
 *               consumer (Zephyr: registered ch cb / Linux: PTY output queue).
 *   lock/unlock: serialize TX assembly. Zephyr uses a k_mutex; Linux (single
 *               threaded poll loop) can pass NULL (no-op).
 *   log       : optional diagnostic sink; NULL to disable.
 */
struct uart_mux_io {
	int  (*write_raw)(void *ctx, const uint8_t *buf, size_t len);
	void (*deliver)(void *ctx, uint8_t channel,
			const uint8_t *payload, size_t len);
	void (*lock)(void *ctx);
	void (*unlock)(void *ctx);
	void (*log)(void *ctx, const char *msg);
	void *ctx;
};

/* RX state machine states. */
enum uart_mux_rx_state {
	UMX_WAIT_SOF,
	UMX_CHANNEL,
	UMX_FLAGS,
	UMX_LEN0,
	UMX_LEN1,
	UMX_PAYLOAD,
	UMX_FCS0,
	UMX_FCS1,
	UMX_ESYNC,
};

struct uart_mux {
	const struct uart_mux_io *io;

	/* RX */
	enum uart_mux_rx_state st;
	uint8_t  ch;
	uint8_t  flags;
	uint16_t len;
	uint16_t got;
	uint16_t fcs_rx;
	uint16_t fcs_acc;
	uint8_t  payload[UART_MUX_PAYLOAD_MAX];

	/* TX scratch (guarded by io->lock) */
	uint8_t  tx_buf[UART_MUX_FRAME_MAX];

	/* stats */
	uint32_t rx_frames_ok;
	uint32_t rx_fcs_err;
	uint32_t rx_resync;
};

/* FCS16 helper (also used stand-alone by ports / tests). */
uint16_t uart_mux_fcs16(uint16_t crc, const uint8_t *data, size_t len);

/* Initialise the core with an OS io backend. */
void uart_mux_init(struct uart_mux *c, const struct uart_mux_io *io);

/* TX: assemble a frame for `channel` and push it via io->write_raw.
 * Returns 0 on success, negative errno-style code otherwise. */
int uart_mux_send(struct uart_mux *c, uint8_t channel,
		       const uint8_t *payload, size_t len);

/* RX: feed raw received bytes into the state machine. Complete, FCS-valid
 * frames are handed to io->deliver. Safe to call with arbitrary chunk sizes. */
void uart_mux_rx_bytes(struct uart_mux *c,
			    const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* UART_MUX_H_ */
