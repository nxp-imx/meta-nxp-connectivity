/*
 * Copyright 2024 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * hci_bridge.h - OS-agnostic BLE HCI H4 bridge (shared Zephyr / Linux).
 *
 * This is the EXACT SAME C source compiled on the Zephyr NBU companion app and
 * in the Linux muxd daemon. It owns the H4 framing, the adv/ctrl transmit
 * prioritization, the MCXW72 NBU 0x29 -> 0x0a connection-complete fixup and the
 * create-connection settle delay. All OS actions go through struct bridge_ops.
 *
 * Direction model (symmetric, one instance per side):
 *   - "from_local" bytes are H4 frames produced by the local endpoint
 *     (Zephyr: the controller; Linux: the host). They are H4-framed here, then
 *     classified and pushed to the mux BLE channel via ops->mux_send.
 *   - "from_mux" bytes are payloads decoded off the mux BLE channel. They are
 *     H4-framed here, the NBU fixup / settle delay applied, then handed to the
 *     local endpoint via ops->to_local.
 *
 * Both directions share the same H4 length parser (h4_feed) so the framing is
 * bit-identical. The prioritization (ctrl-first + bounded adv drop-oldest) and
 * the NBU fixup only apply on the controller->host path; a role flag selects
 * the behaviour without changing the source.
 */

#ifndef HCI_BRIDGE_H_
#define HCI_BRIDGE_H_

#include <stddef.h>
#include <stdint.h>

#include "bridge_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Which end of the link this bridge instance represents. */
enum hci_bridge_role {
	HCI_BRIDGE_NBU,   /* Zephyr side: local endpoint is the controller. */
	HCI_BRIDGE_HOST,  /* Linux side:  local endpoint is the host stack.  */
};

/* Maximum single H4 frame we reassemble (HCI event/ACL bounded on this link). */
#ifndef HCI_BRIDGE_FRAME_MAX
#define HCI_BRIDGE_FRAME_MAX 512
#endif

/* Bounded adv-report ring (drop-oldest). Kept small: a stale adv report is
 * harmless to drop, a late Command Complete kills pairing. */
#ifndef HCI_BRIDGE_ADV_Q_SIZE
#define HCI_BRIDGE_ADV_Q_SIZE 8
#endif

struct hci_adv_slot {
	uint8_t  buf[HCI_BRIDGE_FRAME_MAX];
	uint16_t len;
};

struct hci_bridge {
	const struct bridge_ops *ops;
	enum hci_bridge_role role;

	/* H4 reassembly state for the from_mux stream. */
	uint8_t  fr[HCI_BRIDGE_FRAME_MAX];
	uint16_t fr_len;
	uint16_t fr_need;   /* total expected frame length once header known */

	/* Bounded adv-report ring (controller->host path only). */
	struct hci_adv_slot adv_q[HCI_BRIDGE_ADV_Q_SIZE];
	uint32_t adv_head;
	uint32_t adv_count;

	/* stats */
	uint32_t adv_dropped;
	uint32_t ctrl_frames;
	uint32_t adv_frames;
};

/* Initialise a bridge instance for the given role and OS ops. */
void hci_bridge_init(struct hci_bridge *b, enum hci_bridge_role role,
		     const struct bridge_ops *ops);

/*
 * Feed raw bytes decoded off the mux BLE channel. Complete H4 frames are
 * reassembled and delivered to the local endpoint via ops->to_local (with NBU
 * fixup + settle delay applied on the host-directed path).
 */
void hci_bridge_from_mux(struct hci_bridge *b, const uint8_t *buf, size_t len);

/*
 * Submit one complete H4 frame produced by the local endpoint for transmission
 * over the mux BLE channel. On the NBU role this applies the adv/ctrl
 * prioritization and the 0x29 fixup before ops->mux_send.
 */
void hci_bridge_from_local(struct hci_bridge *b,
			   const uint8_t *frame, size_t len);

/*
 * Pump one adv report out of the bounded ring (NBU role). Call after every
 * ctrl-queue drain so a freshly submitted Command Complete always wins the
 * next wire slot. Returns 1 if an adv report was sent, 0 if the ring is empty.
 */
int hci_bridge_pump_adv(struct hci_bridge *b);

#ifdef __cplusplus
}
#endif

#endif /* HCI_BRIDGE_H_ */
