# MCXW72 BLE + OpenThread RCP EAR2

## Overview

This EAR2 package enables the MCXW72 to operate as an RCP connected to a Linux host over a single physical UART with HW flow control.

The UART transports both:

- **BLE HCI (H4)**, consumed by BlueZ (`btattach`)
- **OpenThread Spinel (HDLC)**, consumed by the OpenThread host in RCP mode

The provided `muxd` userspace daemon multiplexes and demultiplexes these logical interfaces over the physical UART. It opens the physical UART and creates two pseudo-terminals (PTYs). No kernel multiplexer module is required.

## EAR2 Package Contents

- `zephyr.elf`: MCXW72 application firmware for boards using the internal FRO32K (no external 32 kHz crystal). Program using J-Link.
- `mcxw72_nbu_dyn_full.bin`: MCXW72 dynamic NBU firmware.
- `mux daemon`: C implementation of the Linux host UART multiplexer daemon.

## MCXW72 Firmware Programming

### Application firmware

Program the application firmware `zephyr.elf` for boards relying on the internal FRO32K using J-Link.
```bash
loadfile zephyr.elf
r
g
exit
```

### NBU firmware

Program `mcxw72_nbu_dyn_full.bin` by following the official Zephyr FRDM-MCXW72 NBU flashing procedure:

[Zephyr FRDM-MCXW72 NBU flashing instructions](https://docs.zephyrproject.org/latest/boards/nxp/frdm_mcxw72/doc/index.html#nbu-flashing)

## Physical Wiring (MCXW72 LPUART1 with RTS/CTS)

The firmware runs uart_mux on LPUART1 at 921600 baud, 8N1, with hardware
RTS/CTS flow control.

| MCXW72 signal | Pin  | FRDM-MCXW72 | FRDM-IMX93 (P11) | i.MX93 signal |
|---------------|------|-------------|------------------|---------------|
| LPUART1_TX    | PTB2 | J6 SCK      | 28 (GPIO_IO01)   | LPUART5_RX    |
| LPUART1_RX    | PTB3 | J6 MOSI     | 27 (GPIO_IO00)   | LPUART5_TX    |
| LPUART1_RTS_b | PTB5 | J5 SCL      | 3 (GPIO_IO02)    | LPUART5_CTS_B |
| LPUART1_CTS_b | PTB4 | J5 SDA      | 5 (GPIO_IO03)    | LPUART5_RTS_B |
| GND           |      | J6 GND      | 6                | GND           |

- On MCXW727C, LPUART1 RTS/CTS only exist on PTB5/PTB4. `lpi2c1`
  (FXLS8974 accelerometer) and `lpspi1` are disabled in the firmware to free
  PTB2..PTB5.
- The i.MX93 device tree `imx93-11x11-frdm-mcxw72-ble-ot-rcp.dts` remaps
  LPUART5 to GPIO_IO00..IO03 with `uart-has-rtscts`. The port is `/dev/ttyLP4`.
  This device tree is not part of the `mcxw72-ble-ot-mux` recipe. It is
  provided by the kernel recipe in
  `meta-nxp-connectivity/meta-nxp-otbr/recipes-kernel/linux/files/dts/`.
- On a customer platform, the Linux UART device name depends on the hardware.

## MCXW72 Debug Console

The console, shell and logs are on LPUART0 at 115200 baud, 8N1, without flow
control. The J14 MCU-Link VCOM no longer carries the console: connect an
external 3.3 V USB-UART adapter (for example FTDI TTL-232R-3V3).

| MCXW72 signal | Pin   | FRDM-MCXW72 | USB-UART adapter |
|---------------|-------|-------------|------------------|
| LPUART0_TX    | PTA17 | J5 TX       | RX               |
| LPUART0_RX    | PTA16 | J5 RX       | TX               |
| GND           |       | J5 GND      | GND              |

- Cross TX and RX. Do not connect the adapter VCC pin: the board is powered
  from USB.
- Use 3.3 V logic levels only, a 5 V adapter can damage the MCXW72.

## Linux Host: C Implementation

This package ships a C implementation of the daemon. It implements the on-wire frame format and communicates with the `uart_mux` firmware on the MCXW72. RTS/CTS (CRTSCTS) is enabled by default and is required with this firmware, so do not pass `--no-flow`. Other options include `--crc-init0` and `--replay`. A native or cross-compilation C toolchain is required.

The daemon creates two PTYs and prints the BLE and OpenThread attachment information.

```text
                         +----------------------- muxd -----------------------+
Physical UART <--------> | UART                                         |    |
(to MCXW72)              |   | demultiplex by channel ID                |    |
                         |   +--> /dev/pts/X (HCI)    <-- BlueZ         |    |
                         |   +--> /dev/pts/Y (Spinel) <-- OpenThread    |    |
                         +----------------------------------------------------+
```

### Source Code Organization: Shared and Linux-Only

The protocol logic is written once and compiled on both sides of the UART. The
MCXW72 firmware (Zephyr) and the i.MX93 daemon (`muxd`, Linux) build the
**same C files** for framing, FCS, demultiplexing, H4 packet assembly and
Spinel/HDLC handling. Each side only adds a small OS port that supplies the
real I/O (UART driver, threads, PTYs) through callbacks.

```text
          MCXW72 (Zephyr)                          i.MX93 (Linux)
  +-------------------------------+      +-------------------------------+
  | zephyr.elf (prebuilt)         |      | muxd.c (CLI, poll() loop)     |
  |                               |      |                               |
  +-------------------------------+      +-------------------------------+
  | Zephyr OS port                |      | port_linux.c  (OS port)       |
  +===============================+      +===============================+
  | SHARED: uart_mux.c            |      | SHARED: uart_mux.c            |
  |         hci_bridge.c          | UART |         hci_bridge.c          |
  |         spinel_bridge.c       |<====>|         spinel_bridge.c       |
  +===============================+      +===============================+
```

#### Shared sources (identical on Zephyr and Linux)

These files must stay byte-for-byte identical. They contain no Zephyr or
Linux API calls: every OS service is reached through the function pointers
declared in `bridge_port.h` and the uart_mux I/O structure.

| File (in `src/shared/`) | Role |
|---|---|
| `uart_mux.c` | Frame encoder/decoder: SYNC, CHANNEL, FLAGS, LENGTH, FCS16, resynchronization |
| `uart_mux_int.h` | Internal uart_mux definitions (frame constants, channel IDs, I/O callbacks) |
| `hci_bridge.c` / `hci_bridge.h` | BLE channel: H4 packet assembly and HCI forwarding, NBU event fix-ups |
| `spinel_bridge.c` / `spinel_bridge.h` | Thread channel: HDLC Spinel frame delimiting (with `0x7D` escape tracking) |
| `bridge_port.h` | OS port interface: callbacks each OS must implement |

On the MCXW72 side, these shared sources are already compiled into the
supplied `zephyr.elf`. On the Linux side, the same sources are shipped in
`src/shared/` and compiled into `muxd`. Do not modify the files in
`src/shared/`: `muxd` must stay byte-for-byte compatible with the protocol
built into `zephyr.elf`, and a change on the Linux side only can break the
communication with the MCXW72.

#### Linux-only sources (i.MX93 host)

| File | Role |
|---|---|
| `src/muxd.c` | Daemon entry point: command line (`--uart`, `--baud`, `--no-flow`, `--crc-init0`, `--replay`, `-v`), `poll()` main loop |
| `src/port_linux.c` / `src/port_linux.h` | Linux OS port: opens and configures the UART (`termios`, CRTSCTS), creates the HCI and Spinel PTYs, implements the `bridge_port.h` callbacks |
| `Makefile` | Builds `muxd` from `src/muxd.c`, `src/port_linux.c` and the shared sources |

### Build the C Implementation

The C daemon is split into a thin wire-up (`src/muxd.c`), a Linux OS port
(`src/port_linux.c`), and the shared wire-protocol logic (`src/shared/`). Use
the supplied Makefile to build all of them:

```bash
make
```

### Run the C Implementation

```bash
./muxd --uart /dev/ttyLP4 --baud 921600
```

For verbose logging:

```bash
./muxd --uart /dev/ttyLP4 --baud 921600 -v
```

The daemon creates two PTYs.

```text
BLE HCI PTY:   /dev/pts/X  -> btattach -B /dev/pts/X -P h4
OT Spinel PTY: /dev/pts/Y  -> spinel+hdlc+uart:///dev/pts/Y?uart-baudrate=921600
```

The PTY numbers are dynamically assigned. Use the paths printed by `muxd`.

## On-Wire Frame Format

The UART multiplexing frame matches the Zephyr `uart_mux` module byte-for-byte.

### Frame Layout

| Order | Field | Size | Wire encoding | Description |
|---:|---|---:|---|---|
| 1 | **Start SYNC** | 1 byte | `0x7E` | Marks the beginning of the frame. |
| 2 | **CHANNEL** | 1 byte | Channel identifier | Selects the logical payload destination. |
| 3 | **FLAGS** | 1 byte | Bit field | Bit 0 carries the protocol version. Bits 1 to 7 are reserved. |
| 4 | **LENGTH** | 2 bytes | 16-bit little-endian | Number of bytes in `PAYLOAD`, from 0 to 2048. |
| 5 | **PAYLOAD** | 0 to 2048 bytes | Opaque data | Complete OpenThread Spinel/HDLC or BLE HCI H4 frame. |
| 6 | **FCS16** | 2 bytes | 16-bit little-endian | CRC calculated over `CHANNEL + FLAGS + LENGTH + PAYLOAD`. |
| 7 | **End SYNC** | 1 byte | `0x7E` | Marks the end of the frame and can also precede the next frame. |

### FCS Coverage

```text
Start SYNC | CHANNEL | FLAGS | LENGTH | PAYLOAD | FCS16 | End SYNC
             <-------- fields covered by FCS16 -------->
```

### Channel Identifiers

| Channel | Purpose | `muxd` behavior |
|---|---|---|
| `0x01` | OpenThread Spinel/HDLC | Routed to the Spinel PTY. |
| `0x02` | BLE HCI H4 | Routed to the HCI PTY. |
| `0x03` | Control, internal | Logged or ignored. |
| `0x04` | Diagnostic | Logged or ignored. |
| `0x05` | OTA | Logged or ignored. |

### Field Details

- **SYNC** is `0x7E` at both the start and the end of every frame.
- **FLAGS** is one byte. Bit 0 is the protocol version. Bits 1 to 7 are reserved and transmitted as zero.
- **LENGTH** is a 16-bit little-endian payload size with a maximum value of 2048 bytes.
- **FCS16** uses CRC-16/CCITT reflected with polynomial `0x8408`. It is transmitted little-endian and calculated over `CHANNEL + FLAGS + LENGTH + PAYLOAD`.
- **Delimiting** uses the explicit payload length. The trailing SYNC is validated and can also serve as the opening SYNC of the next frame.
- On an FCS or frame-format error, `muxd` resynchronizes. A SYNC is accepted as a frame start only when followed by a known channel identifier.
- A **Thread payload** is a complete native HDLC Spinel frame and is forwarded verbatim. The `0x7D` escape is tracked so that an escaped `0x7E` is not treated as an HDLC delimiter.
- A **BLE payload** is a complete HCI H4 frame, including the type octet and data. The C implementation assembles the BlueZ byte stream into complete H4 packets using the H4 type and length rules.

### FCS Configuration

For this EAR2 package, the default FCS configuration used by the supplied `muxd` implementation is:

| Parameter | Value |
|---|---|
| Algorithm | CRC-16/CCITT reflected |
| Polynomial | `0x8408`, reflected form of `0x1021` |
| Initial value | `0xFFFF` |
| Final XOR | None |
| FCS wire byte order | Little-endian |

The C implementation also provides `--crc-init0` for use with an MCXW72 firmware configuration that initializes the FCS to `0x0000`.

### Buffer Sizes

These limits are defined in the shared headers, so `muxd` and `zephyr.elf`
use the same values:

| Constant | Header | Value | Rationale |
|---|---|---:|---|
| `UART_MUX_PAYLOAD_MAX` | `uart_mux_int.h` | 2048 | Maximum frame `LENGTH`. |
| `SPINEL_BRIDGE_FRAME_MAX` | `spinel_bridge.h` | 2048 | Maximum Spinel/HDLC frame on the Thread channel. |
| `HCI_BRIDGE_FRAME_MAX` | `hci_bridge.h` | 512 | H4 assembler bound for BLE HCI and ISO traffic, including margin. |

## EAR2 QA Validation Status

The EAR2 release was evaluated using the following QA test scenarios.

### UART1 With Flow Control Interface Testing

The following UART test scenarios were executed successfully:

- Continuous traffic
- Bidirectional traffic
- Stress traffic

### MCXW72 Internal XTAL Testing

- test execution completes without any new blocking issues

### Matter Commissioning and Controller Operation

- not in scope for this release

