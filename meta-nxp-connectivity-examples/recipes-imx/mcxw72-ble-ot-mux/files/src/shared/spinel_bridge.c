/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * spinel_bridge.c - OS-agnostic OpenThread Spinel/HDLC bridge (shared
 * Zephyr / Linux).
 *
 * The EXACT SAME source file is compiled by the Zephyr NBU companion app and by
 * the Linux muxd daemon. It touches NO OS API: everything goes through the
 * injected struct bridge_ops. The HDLC framer therefore stays byte-identical
 * on both OSes.
 *
 * HDLC delimiting (RFC 1662 byte stuffing):
 *   flag   = 0x7E marks the boundary between frames
 *   escape = 0x7D; the following byte is XORed with 0x20 to recover the value
 *
 * We delimit whole frames so a single ops->to_local delivery carries a
 * complete HDLC frame (opening flag through closing flag), which keeps the two
 * sides bit-identical and simplifies logging. The Spinel payload itself is
 * never interpreted.
 */

#include <string.h>

#include "spinel_bridge.h"

#define HDLC_FLAG   0x7EU
#define HDLC_ESCAPE 0x7DU
#define HDLC_XOR    0x20U

/* Mux channel id for OpenThread Spinel (matches UART_MUX_CH_THREAD). */
#define SPINEL_MUX_CHANNEL 0x01U

static inline int ops_mux_send(struct spinel_bridge *b,
			       const uint8_t *buf, size_t len)
{
	if (b->ops->mux_send != NULL) {
		return b->ops->mux_send(b->ops->ctx, SPINEL_MUX_CHANNEL,
					buf, len);
	}
	return -1;
}

static inline void ops_to_local(struct spinel_bridge *b,
				const uint8_t *buf, size_t len)
{
	if (b->ops->to_local != NULL) {
		b->ops->to_local(b->ops->ctx, buf, len);
	}
}

static inline void ops_log(struct spinel_bridge *b, const char *msg)
{
	if (b->ops->log != NULL) {
		b->ops->log(b->ops->ctx, msg);
	}
}

void spinel_bridge_init(struct spinel_bridge *b, const struct bridge_ops *ops)
{
	memset(b, 0, sizeof(*b));
	b->ops = ops;
}

/* -----------------------------------------------------------------------
 * from_mux: reassemble HDLC frames and deliver complete frames to the local
 * endpoint. The delivered buffer includes the opening and closing 0x7E flags
 * so the local endpoint (OT NCP / host RCP driver) sees a well-formed frame.
 * ----------------------------------------------------------------------- */
static void spinel_frame_reset(struct spinel_bridge *b)
{
	b->fr_len = 0;
	b->in_frame = 0;
	b->escaped = 0;
}

void spinel_bridge_from_mux(struct spinel_bridge *b,
			    const uint8_t *buf, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		uint8_t byte = buf[i];

		if (byte == HDLC_FLAG) {
			if (b->in_frame && b->fr_len > 0) {
				/* Closing flag: append it and deliver. */
				if (b->fr_len < SPINEL_BRIDGE_FRAME_MAX) {
					b->fr[b->fr_len++] = HDLC_FLAG;
					b->rx_frames++;
					ops_to_local(b, b->fr, b->fr_len);
				} else {
					b->rx_overflow++;
					ops_log(b, "HDLC frame overflow, drop");
				}
			}
			/* Start a fresh frame with the (opening) flag. */
			spinel_frame_reset(b);
			b->in_frame = 1;
			b->fr[b->fr_len++] = HDLC_FLAG;
			continue;
		}

		if (!b->in_frame) {
			/* Bytes outside of any frame: ignore until a flag. */
			continue;
		}

		if (b->fr_len >= SPINEL_BRIDGE_FRAME_MAX) {
			b->rx_overflow++;
			ops_log(b, "HDLC frame overflow, resync");
			spinel_frame_reset(b);
			continue;
		}

		/* Preserve the raw HDLC bytes (including escapes) verbatim: the
		 * local endpoint runs its own HDLC decoder. We only track the
		 * escape state so an escaped 0x7E is not mistaken for a flag. */
		if (b->escaped) {
			b->fr[b->fr_len++] = byte;
			b->escaped = 0;
		} else if (byte == HDLC_ESCAPE) {
			b->fr[b->fr_len++] = byte;
			b->escaped = 1;
		} else {
			b->fr[b->fr_len++] = byte;
		}
	}
}

/* -----------------------------------------------------------------------
 * from_local: forward Spinel bytes verbatim onto the mux Thread channel. The
 * OT NCP layer already emits fully HDLC-framed Spinel, so no re-framing here.
 * ----------------------------------------------------------------------- */
void spinel_bridge_from_local(struct spinel_bridge *b,
			      const uint8_t *buf, size_t len)
{
	if (len == 0) {
		return;
	}
	b->tx_frames++;
	(void)ops_mux_send(b, buf, len);
}
