/*
 * Copyright 2024 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * hci_bridge.c - OS-agnostic BLE HCI H4 bridge (shared Zephyr / Linux).
 *
 * The EXACT SAME source file is compiled by the Zephyr NBU companion app and by
 * the Linux muxd daemon. It touches NO OS API: everything (mux TX, delivery to
 * the local endpoint, sleeping for the settle delay, logging) goes through the
 * injected struct bridge_ops. The H4 length parser, the adv/ctrl transmit
 * prioritization, the MCXW72 NBU 0x29 -> 0x0a connection-complete fixup and the
 * create-connection settle delay therefore stay byte-identical on both OSes.
 *
 * H4 packet framing (first byte is the packet type indicator):
 *   0x01 CMD : opcode(2) + plen(1) + params(plen)
 *   0x02 ACL : handle(2) + dlen(2 LE) + data(dlen)
 *   0x03 SCO : handle(2) + dlen(1)    + data(dlen)
 *   0x04 EVT : code(1)   + plen(1)    + params(plen)
 *   0x05 ISO : handle(2) + dlen(2 LE, 14-bit) + data(dlen)
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "hci_bridge.h"

/* H4 packet type indicators. */
#define H4_CMD 0x01
#define H4_ACL 0x02
#define H4_SCO 0x03
#define H4_EVT 0x04
#define H4_ISO 0x05

/* HCI event / LE meta codes. */
#define HCI_EVT_CODE_LE_META               0x3e
#define HCI_LE_SUBEVENT_ENH_CONN_COMPLETE  0x0a
#define HCI_LE_SUBEVENT_EXT_ADV_REPORT     0x0d
#define HCI_LE_SUBEVENT_NBU_CONN_COMPLETE  0x29
#define HCI_ENH_CONN_COMPLETE_PARAMLEN     0x1f

/* HCI command opcodes that need the settle delay. */
#define HCI_OP_LE_CREATE_CONN     0x200d
#define HCI_OP_LE_EXT_CREATE_CONN 0x2043

/*
 * LE Create Connection Cancel (0x200e). chip-tool (and BlueZ) send this to
 * abort an initiating attempt that has not yet produced a Connection Complete,
 * e.g. when the user stops the pairing. We instrument it on both directions to
 * verify, on a retry that fails, that the cancel actually reaches the NBU and
 * completes cleanly (Command Complete with status). A cancel that never
 * completes would leave the NBU initiator state machine stuck and explain the
 * "works once, then fails on retry" behaviour on the shared radio.
 */
#define HCI_OP_LE_CREATE_CONN_CANCEL 0x200e

/* HCI Command Complete event code (used to trace the cancel completion). */
#define HCI_EVT_CODE_CMD_COMPLETE    0x0e


/* Minimum reliable NBU scan->initiating settle on this 3-wire link (ms). */
#define HCI_CREATE_CONN_SETTLE_MS 50U

/* Mux channel id for BLE HCI (matches uart_mux.h UART_MUX_CH_BLE). */
#define HCI_MUX_CHANNEL 0x02U

/* -----------------------------------------------------------------------
 * bridge_ops helpers (all NULL-safe).
 * ----------------------------------------------------------------------- */
static inline int ops_mux_send(struct hci_bridge *b,
			       const uint8_t *buf, size_t len)
{
	if (b->ops->mux_send != NULL) {
		return b->ops->mux_send(b->ops->ctx, HCI_MUX_CHANNEL, buf, len);
	}
	return -1;
}

static inline void ops_to_local(struct hci_bridge *b,
				const uint8_t *buf, size_t len)
{
	if (b->ops->to_local != NULL) {
		b->ops->to_local(b->ops->ctx, buf, len);
	}
}

static inline void ops_sleep_ms(struct hci_bridge *b, uint32_t ms)
{
	if (b->ops->sleep_ms != NULL) {
		b->ops->sleep_ms(b->ops->ctx, ms);
	}
}

static inline void ops_log(struct hci_bridge *b, const char *msg)
{
	if (b->ops->log != NULL) {
		b->ops->log(b->ops->ctx, msg);
	}
}

/*
 * Formatted log helper. Builds the message into a local ASCII buffer with
 * snprintf (available identically on Zephyr and Linux) and routes it through
 * the injected ops->log sink, so it stays OS-agnostic. No-op when no sink is
 * registered.
 */
static void hci_logf(struct hci_bridge *b, const char *fmt, ...)
{
	char line[128];
	va_list ap;

	if (b->ops->log == NULL) {
		return;
	}
	va_start(ap, fmt);
	vsnprintf(line, sizeof(line), fmt, ap);
	va_end(ap);
	b->ops->log(b->ops->ctx, line);
}

/*
 * Decode the HCI event code / LE meta subevent of a controller->host frame for
 * tracing. Returns a short human-readable tag; unknown events fall back to a
 * hex form written into the caller-provided scratch buffer.
 */
static const char *hci_evt_tag(const uint8_t *f, size_t len,
			       char *scratch, size_t sn)
{
	if (len < 2 || f[0] != H4_EVT) {
		return "non-evt";
	}
	if (f[1] == HCI_EVT_CODE_LE_META && len >= 4) {
		switch (f[3]) {
		case HCI_LE_SUBEVENT_ENH_CONN_COMPLETE:
			return "LE Enh Conn Complete (0x0a)";
		case HCI_LE_SUBEVENT_NBU_CONN_COMPLETE:
			return "NBU Conn Complete (0x29)";
		case HCI_LE_SUBEVENT_EXT_ADV_REPORT:
			return "LE Ext Adv Report (0x0d)";
		default:
			snprintf(scratch, sn, "LE meta subevent 0x%02x", f[3]);
			return scratch;
		}
	}
	snprintf(scratch, sn, "evt 0x%02x", f[1]);
	return scratch;
}

/* -----------------------------------------------------------------------
 * H4 frame length computation.
 *
 * Returns the TOTAL H4 frame length (including the type byte) once enough
 * header bytes are present, 0 if more header bytes are still needed, or a
 * negative value on an unknown type (caller should resync).
 * ----------------------------------------------------------------------- */
static int h4_total_len(const uint8_t *f, size_t got)
{
	if (got < 1) {
		return 0;
	}

	switch (f[0]) {
	case H4_CMD:
		if (got < 4) {
			return 0;
		}
		return 1 + 2 + 1 + f[3];
	case H4_ACL:
		if (got < 5) {
			return 0;
		}
		return 1 + 2 + 2 + ((int)f[3] | ((int)f[4] << 8));
	case H4_SCO:
		if (got < 4) {
			return 0;
		}
		return 1 + 2 + 1 + f[3];
	case H4_EVT:
		if (got < 3) {
			return 0;
		}
		return 1 + 1 + 1 + f[2];
	case H4_ISO:
		if (got < 5) {
			return 0;
		}
		return 1 + 2 + 2 + (((int)f[3] | ((int)f[4] << 8)) & 0x3fff);
	default:
		return -1;
	}
}

/* True if this ctrl->host H4 event is an LE Extended Advertising Report. */
static int is_le_ext_adv_report(const uint8_t *f, size_t len)
{
	return (len >= 4 && f[0] == H4_EVT &&
		f[1] == HCI_EVT_CODE_LE_META &&
		f[3] == HCI_LE_SUBEVENT_EXT_ADV_REPORT);
}

/*
 * Rewrite the MCXW72 NBU vendor connection-complete subevent (0x29) into a
 * standard LE Enhanced Connection Complete v1 (0x0a) in place. Returns the
 * (possibly reduced) frame length to forward to the host.
 */
static size_t fixup_nbu_conn_complete(struct hci_bridge *b,
				      uint8_t *f, size_t len)
{
	size_t std_len;

	if (len < 4 || f[0] != H4_EVT || f[1] != HCI_EVT_CODE_LE_META) {
		return len;
	}
	if (f[3] != HCI_LE_SUBEVENT_NBU_CONN_COMPLETE) {
		return len;
	}

	std_len = 3 + HCI_ENH_CONN_COMPLETE_PARAMLEN;
	if (len < std_len) {
		ops_log(b, "NBU conn-complete 0x29 too short, not rewriting");
		return len;
	}

	f[3] = HCI_LE_SUBEVENT_ENH_CONN_COMPLETE; /* 0x29 -> 0x0a */
	f[2] = HCI_ENH_CONN_COMPLETE_PARAMLEN;    /* normalize paramlen */
	/* f[4] is the LE Enh Conn Complete status; handle is f[5..6]. Trace it
	 * so we can see the exact connection outcome the host will register. */
	hci_logf(b, "*** NBU 0x29 -> 0x0a Enh Conn Complete: status=0x%02x "
		 "handle=0x%04x (paramlen %u->%u) ***",
		 f[4], (uint16_t)f[5] | ((uint16_t)f[6] << 8),
		 (unsigned)len - 3, (unsigned)HCI_ENH_CONN_COMPLETE_PARAMLEN);
	return std_len;
}

/* -----------------------------------------------------------------------
 * Bounded adv-report ring (NBU role only, drop-oldest).
 * ----------------------------------------------------------------------- */
static void adv_q_push(struct hci_bridge *b, const uint8_t *f, size_t len)
{
	uint32_t tail;

	if (len > HCI_BRIDGE_FRAME_MAX) {
		return;
	}
	if (b->adv_count == HCI_BRIDGE_ADV_Q_SIZE) {
		/* Full: drop the oldest adv report to make room. */
		b->adv_head = (b->adv_head + 1) % HCI_BRIDGE_ADV_Q_SIZE;
		b->adv_count--;
		b->adv_dropped++;
	}
	tail = (b->adv_head + b->adv_count) % HCI_BRIDGE_ADV_Q_SIZE;
	memcpy(b->adv_q[tail].buf, f, len);
	b->adv_q[tail].len = (uint16_t)len;
	b->adv_count++;
}

int hci_bridge_pump_adv(struct hci_bridge *b)
{
	struct hci_adv_slot *slot;

	if (b->adv_count == 0) {
		return 0;
	}
	slot = &b->adv_q[b->adv_head];
	(void)ops_mux_send(b, slot->buf, slot->len);
	b->adv_head = (b->adv_head + 1) % HCI_BRIDGE_ADV_Q_SIZE;
	b->adv_count--;
	b->adv_frames++;
	return 1;
}

/* -----------------------------------------------------------------------
 * Init
 * ----------------------------------------------------------------------- */
void hci_bridge_init(struct hci_bridge *b, enum hci_bridge_role role,
		     const struct bridge_ops *ops)
{
	memset(b, 0, sizeof(*b));
	b->ops = ops;
	b->role = role;
}

/* -----------------------------------------------------------------------
 * from_local: one complete H4 frame produced by the local endpoint, to be
 * sent over the mux BLE channel.
 *
 * NBU role (controller->host): adv reports go to the bounded ring, everything
 * else is ctrl and is sent immediately (ctrl-first), after the 0x29 fixup.
 * HOST role (host->controller): forwarded verbatim.
 * ----------------------------------------------------------------------- */
void hci_bridge_from_local(struct hci_bridge *b,
			   const uint8_t *frame, size_t len)
{
	uint8_t tmp[HCI_BRIDGE_FRAME_MAX];
	size_t tx_len = len;

	if (len == 0 || len > HCI_BRIDGE_FRAME_MAX) {
		return;
	}

	if (b->role == HCI_BRIDGE_NBU) {
		/* Apply the NBU 0x29 -> 0x0a fixup on a private copy. */
		memcpy(tmp, frame, len);
		if (tmp[0] == H4_EVT) {
			tx_len = fixup_nbu_conn_complete(b, tmp, len);
		}

		if (is_le_ext_adv_report(tmp, tx_len)) {
			adv_q_push(b, tmp, tx_len);
			return;
		}

		/* High-priority ctrl frame: send now, ahead of adv backlog. */
		b->ctrl_frames++;
		(void)ops_mux_send(b, tmp, tx_len);
		return;
	}

	/* HOST role: verbatim. */
	(void)ops_mux_send(b, frame, len);
}

/* -----------------------------------------------------------------------
 * from_mux: raw bytes decoded off the mux BLE channel. Reassemble complete H4
 * frames and deliver them to the local endpoint.
 *
 * HOST role (controller->host stream arriving from the NBU over the mux):
 *   apply the 0x29 fixup so BlueZ registers the connection.
 * NBU role (host->controller stream arriving from the host over the mux):
 *   apply the create-connection settle delay before delivering to the
 *   controller.
 * ----------------------------------------------------------------------- */
static void hci_frame_complete(struct hci_bridge *b, uint8_t *f, size_t len)
{
	size_t out_len = len;
	char scratch[48];

	if (b->role == HCI_BRIDGE_HOST) {
		/* Trace EVERY complete H4 frame arriving off the wire so we can
		 * confirm whether the conn-complete reaches the host bridge at
		 * all (type + len + first payload bytes). */
		hci_logf(b,
			 "from_mux HOST rx: type=0x%02x len=%u "
			 "b1=0x%02x b2=0x%02x b3=0x%02x",
			 f[0], (unsigned)len,
			 len > 1 ? f[1] : 0xff,
			 len > 2 ? f[2] : 0xff,
			 len > 3 ? f[3] : 0xff);
		/* Trace the connection-relevant event BEFORE the fixup so we
		 * can see exactly what the NBU emitted off the wire. */
		if (f[0] == H4_EVT && f[1] == HCI_EVT_CODE_LE_META &&
		    len >= 4 &&
		    (f[3] == HCI_LE_SUBEVENT_NBU_CONN_COMPLETE ||
		     f[3] == HCI_LE_SUBEVENT_ENH_CONN_COMPLETE)) {
			hci_logf(b,
				 "from_mux HOST: %s len=%u status=0x%02x "
				 "(pre-fixup)",
				 hci_evt_tag(f, len, scratch, sizeof(scratch)),
				 (unsigned)len, len >= 5 ? f[4] : 0xff);
		}
		/*
		 * Trace the Command Complete for LE Create Connection Cancel
		 * (0x200e) so we can confirm the abort completed in the NBU on a
		 * failed retry. Command Complete layout: f[1]=0x0e, f[3]=num
		 * cmd pkts, f[4..5]=opcode (LE), f[6]=status.
		 */
		if (f[0] == H4_EVT && f[1] == HCI_EVT_CODE_CMD_COMPLETE &&
		    len >= 7) {
			uint16_t cc_op = (uint16_t)f[4] | ((uint16_t)f[5] << 8);

			if (cc_op == HCI_OP_LE_CREATE_CONN_CANCEL) {
				hci_logf(b,
					 "*** from_mux HOST: Command Complete "
					 "Create Connection Cancel (0x200e) "
					 "status=0x%02x ***",
					 f[6]);
			}
		}
		if (f[0] == H4_EVT) {
			out_len = fixup_nbu_conn_complete(b, f, len);
		}
		ops_to_local(b, f, out_len);
		return;
	}

	/* NBU role: settle delay in front of (Ext) Create Connection. */
	if (f[0] == H4_CMD && len >= 3) {
		uint16_t opcode = (uint16_t)f[1] | ((uint16_t)f[2] << 8);

		if (opcode == HCI_OP_LE_EXT_CREATE_CONN ||
		    opcode == HCI_OP_LE_CREATE_CONN) {
			hci_logf(b,
				 "*** from_mux NBU: %s create-conn opcode=0x%04x"
				 ", settle %ums before controller ***",
				 opcode == HCI_OP_LE_EXT_CREATE_CONN ?
					 "Ext" : "Legacy",
				 opcode, (unsigned)HCI_CREATE_CONN_SETTLE_MS);
			ops_sleep_ms(b, HCI_CREATE_CONN_SETTLE_MS);
		} else if (opcode == HCI_OP_LE_CREATE_CONN_CANCEL) {
			/*
			 * The host aborts the current initiating attempt. Trace
			 * it so we can correlate the abort with the following
			 * Command Complete (controller->host) and confirm the
			 * NBU initiator state machine is torn down cleanly on a
			 * retry that failed to connect.
			 */
			hci_logf(b,
				 "*** from_mux NBU: LE Create Connection Cancel"
				 " (0x200e) -> controller ***");
		}
	}
	ops_to_local(b, f, len);
}


void hci_bridge_from_mux(struct hci_bridge *b, const uint8_t *buf, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		int total;

		/* Guard against overflow on a corrupt stream. */
		if (b->fr_len >= HCI_BRIDGE_FRAME_MAX) {
			ops_log(b, "H4 frame overflow, resync");
			b->fr_len = 0;
			b->fr_need = 0;
		}

		b->fr[b->fr_len++] = buf[i];

		if (b->fr_need == 0) {
			total = h4_total_len(b->fr, b->fr_len);
			if (total < 0) {
				ops_log(b, "bad H4 type, resync");
				b->fr_len = 0;
				continue;
			}
			if (total == 0) {
				continue; /* need more header bytes */
			}
			if (total > HCI_BRIDGE_FRAME_MAX) {
				ops_log(b, "H4 frame too large, resync");
				b->fr_len = 0;
				continue;
			}
			b->fr_need = (uint16_t)total;
		}

		if (b->fr_need != 0 && b->fr_len >= b->fr_need) {
			hci_frame_complete(b, b->fr, b->fr_need);
			b->fr_len = 0;
			b->fr_need = 0;
		}
	}
}
