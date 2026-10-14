/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * uart_mux.c - OS-agnostic UART channel multiplexer core (frozen wire v1).
 *
 * Pure C99, ZERO OS dependency. The same source is compiled on Zephyr (async
 * UART DMA + workqueue port) and on Linux (poll + PTY port). All OS specifics
 * are injected via the struct uart_mux_io callbacks. The framing, RX state
 * machine, TX assembly and FCS logic are therefore bit-identical on both.
 *
 * ON-WIRE FRAME FORMAT (identical Zephyr <-> Linux):
 *   [SYNC 0x7E][CHANNEL 1B][FLAGS 1B][LENGTH 2B LE][PAYLOAD][FCS16 2B LE][SYNC 0x7E]
 *   FCS16: CRC-16/CCITT reflected, poly 0x8408, init 0xFFFF, no final XOR,
 *          computed over CHANNEL + FLAGS + LENGTH + PAYLOAD.
 */

#include <string.h>

#include "uart_mux_int.h"

/* FCS16 seed. Frozen at 0xFFFF to match the Linux daemon default. */
#define UART_MUX_FCS_INIT 0xFFFFU

/* -----------------------------------------------------------------------
 * FCS16: CRC-16/CCITT reflected (poly 0x8408). Bit-serial so the exact same
 * source compiles everywhere with no table dependency. Matches the Zephyr
 * table variant and the Linux muxd bit-serial variant byte-for-byte.
 * ----------------------------------------------------------------------- */
uint16_t uart_mux_fcs16(uint16_t crc, const uint8_t *data, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (int b = 0; b < 8; b++) {
			crc = (crc & 0x0001U) ? (uint16_t)((crc >> 1) ^ 0x8408U)
					      : (uint16_t)(crc >> 1);
		}
	}
	return crc;
}

/* -----------------------------------------------------------------------
 * Helpers for the optional io callbacks.
 * ----------------------------------------------------------------------- */
static inline int ch_valid(uint8_t b)
{
	return (b >= UART_MUX_CH_MIN && b <= UART_MUX_CH_MAX);
}

static inline void core_lock(struct uart_mux *c)
{
	if (c->io->lock != NULL) {
		c->io->lock(c->io->ctx);
	}
}

static inline void core_unlock(struct uart_mux *c)
{
	if (c->io->unlock != NULL) {
		c->io->unlock(c->io->ctx);
	}
}

static inline void core_log(struct uart_mux *c, const char *msg)
{
	if (c->io->log != NULL) {
		c->io->log(c->io->ctx, msg);
	}
}

/* -----------------------------------------------------------------------
 * Init
 * ----------------------------------------------------------------------- */
void uart_mux_init(struct uart_mux *c, const struct uart_mux_io *io)
{
	memset(c, 0, sizeof(*c));
	c->io = io;
	c->st = UMX_WAIT_SOF;
}

/* -----------------------------------------------------------------------
 * TX: assemble one frame into c->tx_buf and push it via io->write_raw.
 *
 * The whole build+transmit is serialized by io->lock/unlock so two callers
 * (e.g. BLE ch=0x02 and Thread ch=0x01) never interleave into the shared
 * tx_buf. On Linux the single-threaded poll loop can pass NULL lock/unlock.
 * ----------------------------------------------------------------------- */
int uart_mux_send(struct uart_mux *c, uint8_t channel,
		       const uint8_t *payload, size_t len)
{
	uint16_t fcs;
	uint8_t  len_lo, len_hi;
	size_t   p = 0;
	int      ret;

	if (payload == NULL || len == 0) {
		return -1;
	}
	if (!ch_valid(channel)) {
		return -1;
	}
	if (len > UART_MUX_PAYLOAD_MAX) {
		return -1;
	}

	len_lo = (uint8_t)(len & 0xFFU);
	len_hi = (uint8_t)((len >> 8) & 0xFFU);

	core_lock(c);

	/* SYNC + CH + FLAGS + LEN(2) + PAYLOAD + FCS(2) + SYNC */
	c->tx_buf[p++] = UART_MUX_SYNC;
	c->tx_buf[p++] = channel;
	c->tx_buf[p++] = UART_MUX_FLAGS_VER;
	c->tx_buf[p++] = len_lo;
	c->tx_buf[p++] = len_hi;
	memcpy(&c->tx_buf[p], payload, len);
	p += len;

	/* FCS16 over CHANNEL + FLAGS + LENGTH + PAYLOAD == tx_buf[1 .. p-1]. */
	fcs = uart_mux_fcs16(UART_MUX_FCS_INIT, &c->tx_buf[1], p - 1);
	c->tx_buf[p++] = (uint8_t)(fcs & 0xFFU);
	c->tx_buf[p++] = (uint8_t)((fcs >> 8) & 0xFFU);
	c->tx_buf[p++] = UART_MUX_SYNC; /* trailing SYNC opens next frame */

	ret = c->io->write_raw(c->io->ctx, c->tx_buf, p);

	core_unlock(c);
	return ret;
}

/* -----------------------------------------------------------------------
 * RX: byte-fed state machine. Complete, FCS-valid frames are handed to
 * io->deliver. Mirrors the Linux muxd resync semantics exactly:
 *   - back-to-back SYNC bytes tolerated in UMX_CHANNEL
 *   - only a known channel id (0x01..0x05) opens a frame after SYNC
 *   - oversized/zero-length -> hard resync (UMX_WAIT_SOF)
 *   - FCS mismatch -> hard resync (do NOT trust the trailing SYNC)
 *   - on valid frame -> UMX_ESYNC: the trailing SYNC also opens the next
 *     frame (goes straight to UMX_CHANNEL)
 * ----------------------------------------------------------------------- */
void uart_mux_rx_bytes(struct uart_mux *c,
			    const uint8_t *buf, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		uint8_t b = buf[i];

		switch (c->st) {
		case UMX_WAIT_SOF:
			if (b == UART_MUX_SYNC) {
				c->st = UMX_CHANNEL;
			}
			break;

		case UMX_CHANNEL:
			if (b == UART_MUX_SYNC) {
				break; /* tolerate back-to-back SYNC */
			}
			if (!ch_valid(b)) {
				core_log(c, "bad ch after SYNC, resync");
				c->rx_resync++;
				c->st = UMX_WAIT_SOF;
				break;
			}
			c->ch = b;
			c->st = UMX_FLAGS;
			break;

		case UMX_FLAGS:
			c->flags = b;
			c->st = UMX_LEN0;
			break;

		case UMX_LEN0:
			c->len = b; /* low byte */
			c->st = UMX_LEN1;
			break;

		case UMX_LEN1:
			c->len |= (uint16_t)b << 8; /* high byte */
			if (c->len == 0 || c->len > UART_MUX_PAYLOAD_MAX) {
				core_log(c, "bad length, hard resync");
				c->rx_resync++;
				c->st = UMX_WAIT_SOF;
				break;
			}
			c->got = 0;
			c->st = UMX_PAYLOAD;
			break;

		case UMX_PAYLOAD:
			c->payload[c->got++] = b;
			if (c->got >= c->len) {
				c->st = UMX_FCS0;
			}
			break;

		case UMX_FCS0:
			c->fcs_rx = b;
			c->st = UMX_FCS1;
			break;

		case UMX_FCS1: {
			uint16_t fcs_calc;
			uint8_t hdr[4];

			c->fcs_rx |= (uint16_t)b << 8;

			hdr[0] = c->ch;
			hdr[1] = c->flags;
			hdr[2] = (uint8_t)(c->len & 0xFFU);
			hdr[3] = (uint8_t)((c->len >> 8) & 0xFFU);

			fcs_calc = uart_mux_fcs16(UART_MUX_FCS_INIT, hdr, 4);
			fcs_calc = uart_mux_fcs16(fcs_calc, c->payload, c->len);

			if (fcs_calc != c->fcs_rx) {
				core_log(c, "FCS mismatch, hard resync");
				c->rx_fcs_err++;
				c->st = UMX_WAIT_SOF;
				break;
			}

			c->rx_frames_ok++;
			if (c->io->deliver != NULL) {
				c->io->deliver(c->io->ctx, c->ch,
					       c->payload, c->len);
			}
			/* Trailing SYNC expected next; it also opens the next
			 * frame. */
			c->st = UMX_ESYNC;
			break;
		}

		case UMX_ESYNC:
			if (b == UART_MUX_SYNC) {
				c->st = UMX_CHANNEL;
			} else {
				core_log(c, "missing trailing SYNC, resync");
				c->rx_resync++;
				c->st = UMX_WAIT_SOF;
			}
			break;

		default:
			c->st = UMX_WAIT_SOF;
			break;
		}
	}
}
