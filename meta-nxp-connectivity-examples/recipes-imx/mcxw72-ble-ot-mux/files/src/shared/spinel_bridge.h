/*
 * Copyright 2024 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * spinel_bridge.h - OS-agnostic OpenThread Spinel/HDLC bridge (shared
 * Zephyr / Linux).
 *
 * This is the EXACT SAME C source compiled on the Zephyr NBU companion app and
 * in the Linux muxd daemon. Spinel frames are HDLC-encoded (RFC 1662 byte
 * stuffing, flag 0x7E, escape 0x7D, xor 0x20). On this link the Spinel stream
 * is carried verbatim inside the mux Thread channel payload, so the bridge is
 * essentially a pass-through with HDLC frame delimiting for diagnostics and a
 * clean re-frame boundary. All OS actions go through struct bridge_ops.
 *
 * Direction model (symmetric):
 *   - from_local : Spinel bytes produced by the local endpoint (Zephyr: OT NCP;
 *                  Linux: host RCP driver). Pushed onto the mux Thread channel.
 *   - from_mux   : Spinel bytes decoded off the mux Thread channel. Delivered
 *                  to the local endpoint (Zephyr: otNcpHdlcReceive; Linux: PTY).
 *
 * The HDLC framer is used to delimit complete Spinel frames so a single
 * delivery to the local endpoint carries whole frames (both OT NCP and the
 * host RCP driver are tolerant of partial HDLC, but whole-frame delivery keeps
 * the two sides bit-identical and simplifies logging).
 */

#ifndef SPINEL_BRIDGE_H_
#define SPINEL_BRIDGE_H_

#include <stddef.h>
#include <stdint.h>

#include "bridge_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum single HDLC-decoded Spinel frame length on this link. */
#ifndef SPINEL_BRIDGE_FRAME_MAX
#define SPINEL_BRIDGE_FRAME_MAX 2048
#endif

struct spinel_bridge {
	const struct bridge_ops *ops;

	/* HDLC reassembly state for the from_mux stream. */
	uint8_t  fr[SPINEL_BRIDGE_FRAME_MAX];
	size_t   fr_len;
	int      in_frame;   /* between opening and closing 0x7E flags */
	int      escaped;    /* previous byte was the 0x7D escape */

	/* stats */
	uint32_t rx_frames;
	uint32_t tx_frames;
	uint32_t rx_overflow;
};

/* Initialise the bridge with the OS ops. */
void spinel_bridge_init(struct spinel_bridge *b, const struct bridge_ops *ops);

/*
 * Feed raw bytes decoded off the mux Thread channel. Complete HDLC frames
 * (including the delimiting flags) are delivered to the local endpoint via
 * ops->to_local.
 */
void spinel_bridge_from_mux(struct spinel_bridge *b,
			    const uint8_t *buf, size_t len);

/*
 * Submit Spinel bytes produced by the local endpoint for transmission over the
 * mux Thread channel. Forwarded verbatim (the OT NCP layer already emits fully
 * HDLC-framed Spinel).
 */
void spinel_bridge_from_local(struct spinel_bridge *b,
			      const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SPINEL_BRIDGE_H_ */
