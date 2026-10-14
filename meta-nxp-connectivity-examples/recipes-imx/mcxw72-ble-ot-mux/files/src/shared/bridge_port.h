/*
 * Copyright 2026 NXP
 * SPDX-License-Identifier: Apache-2.0
 *
 * bridge_port.h - OS abstraction shared by the HCI and Spinel bridges.
 *
 * The BLE HCI bridge (hci_bridge.c) and the OpenThread Spinel bridge
 * (spinel_bridge.c) are the EXACT SAME C source on Zephyr and on Linux. They
 * never call any OS API directly; every OS specific action goes through this
 * struct bridge_ops, which each port (Zephyr companion app, Linux muxd) fills
 * in. This keeps the protocol logic (H4 framing, adv/ctrl prioritization, the
 * NBU 0x29 conn-complete fixup, the create-connection settle delay, and the
 * HDLC framing) bit-identical across both operating systems.
 *
 *   mux_send : push a fully framed payload onto a logical mux channel. Maps to
 *              uart_mux_port_send() on Zephyr and to the mux TX path on Linux.
 *   to_local : hand a decoded, direction-correct frame to the local endpoint.
 *              On the Zephyr NBU side this feeds the controller (bt_send) or
 *              the OT NCP (otNcpHdlcReceive); on the Linux host side it writes
 *              to the HCI / Spinel PTY.
 *   sleep_ms : block the caller for the given milliseconds (create-conn
 *              settle). k_msleep on Zephyr, nanosleep on Linux.
 *   now_ms   : monotonic milliseconds, used for non-blocking settle timing on
 *              the single-threaded Linux poll loop (may be NULL if sleep_ms is
 *              used instead).
 *   log      : optional ASCII diagnostic sink; NULL to disable.
 */

#ifndef BRIDGE_PORT_H_
#define BRIDGE_PORT_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct bridge_ops {
	int  (*mux_send)(void *ctx, uint8_t channel,
			 const uint8_t *buf, size_t len);
	void (*to_local)(void *ctx, const uint8_t *buf, size_t len);
	void (*sleep_ms)(void *ctx, uint32_t ms);
	uint32_t (*now_ms)(void *ctx);
	void (*log)(void *ctx, const char *msg);
	void *ctx;
};

#ifdef __cplusplus
}
#endif

#endif /* BRIDGE_PORT_H_ */
