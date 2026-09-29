# MCXW72 BLE + OpenThread RCP EAR1

## Overview

This EAR1 package enables the MCXW72 to operate as an RCP connected to a Linux host over a single physical UART.

The UART transports both:

- **BLE HCI (H4)**, consumed by BlueZ (`btattach`)
- **OpenThread Spinel (HDLC)**, consumed by the OpenThread host in RCP mode

The provided `muxd` userspace daemon multiplexes and demultiplexes these logical interfaces over the physical UART. It opens the physical UART and creates two pseudo-terminals (PTYs). No kernel multiplexer module is required.

## EAR1 Package Contents

- `zephyr.elf`: MCXW72 application firmware. Program using J-Link.
- `mcxw72_nbu_dyn_full.bin`: MCXW72 dynamic NBU firmware.
- `muxd.c`: C implementation of the Linux host UART multiplexer daemon.
- `muxd.py`: Python implementation of the Linux host UART multiplexer daemon.

## MCXW72 Firmware Programming

### Application firmware

Program the supplied `zephyr.elf` using J-Link.
```bash
loadfile zephyr.elf
r
g
exit
```

### NBU firmware

Program `mcxw72_nbu_dyn_full.bin` by following the official Zephyr FRDM-MCXW72 NBU flashing procedure:

[Zephyr FRDM-MCXW72 NBU flashing instructions](https://docs.zephyrproject.org/latest/boards/nxp/frdm_mcxw72/doc/index.html#nbu-flashing)

## Physical Wiring (3-Wire Crossover)

| Linux host UART | Connection | MCXW72 FRDM, LPUART0 (mikroBUS J5) |
|---|:---:|---|
| UART TX | `-->` | J5-RX, UART0_RX |
| UART RX | `<--` | J5-TX, UART0_TX |
| GND | `<-->` | J5-GND, GND |

The Linux UART device name depends on the customer platform.

## Linux Host: C and Python Implementations

This package ships two interchangeable implementations of the daemon. Both implement the same on-wire frame format and communicate with the same `uart_mux` firmware on the MCXW72.

| Implementation | File | When to use |
|---|---|---|
| **C** | `muxd`, built from `muxd.c` | Recommended for the target. It includes additional options such as `--no-flow`, `--crc-init0`, and `--replay`. A native or cross-compilation C toolchain is required. |
| **Python** | `muxd.py` | Suitable for quick bring-up. It requires no compilation and uses only Python standard-library modules: `termios`, `os`, `pty`, and `fcntl`. Supported options include `--uart`, `--baud`, and `-v`. |

Both implementations create two PTYs and print the BLE and OpenThread attachment information.

```text
                         +----------------------- muxd -----------------------+
Physical UART <--------> | UART                                         |    |
(to MCXW72)              |   | demultiplex by channel ID                |    |
                         |   +--> /dev/pts/X (HCI)    <-- BlueZ         |    |
                         |   +--> /dev/pts/Y (Spinel) <-- OpenThread    |    |
                         +----------------------------------------------------+
```

### Build the C Implementation

```bash
cc -O2 -Wall -Wextra -o muxd muxd.c
```

### Run the C Implementation

```bash
./muxd --uart /dev/<uart-device> --baud 921600 --no-flow
```

For verbose logging:

```bash
./muxd --uart /dev/<uart-device> --baud 921600 --no-flow -v
```

### Run the Python Implementation

No build is required.

```bash
python3 muxd.py --uart /dev/<uart-device> --baud 921600
```

For verbose logging:

```bash
python3 muxd.py --uart /dev/<uart-device> --baud 921600 -v
```

Like the C implementation, the Python implementation creates two PTYs.

```text
BLE HCI PTY:   /dev/pts/X  -> btattach -B /dev/pts/X -P h4
OT Spinel PTY: /dev/pts/Y  -> spinel+hdlc+uart:///dev/pts/Y?uart-baudrate=921600
```

The PTY numbers are dynamically assigned. Use the paths printed by `muxd`.

The Python implementation uses an FCS initial value of `0xFFFF` and a plain UART without hardware flow control, matching the 3-wire bring-up configuration.

The C-only options `--no-flow`, `--crc-init0`, and `--replay` are not implemented in Python. Use the C implementation when these options are required.

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

For this EAR1 package, the default FCS configuration used by the supplied `muxd` implementations is:

| Parameter | Value |
|---|---|
| Algorithm | CRC-16/CCITT reflected |
| Polynomial | `0x8408`, reflected form of `0x1021` |
| Initial value | `0xFFFF` |
| Final XOR | None |
| FCS wire byte order | Little-endian |

The C implementation also provides `--crc-init0` for use with an MCXW72 firmware configuration that initializes the FCS to `0x0000`.

### Buffer Sizes

Keep these values aligned with the Zephyr configuration:

| Constant | Value | Rationale |
|---|---:|---|
| `MUX_PAYLOAD_MAX` | 2048 | Matches the maximum frame `LENGTH`. |
| `MUX_HCI_MAX` | 512 | H4 assembler bound for BLE HCI and ISO traffic, including margin. |

## EAR1 QA Validation Status

The EAR1 release was evaluated using the following QA test scenarios.

### Matter Commissioning

- Commissioning success rate: **approximately 30%**

### Matter Controller Operation

After successful commissioning:

- Device remains in the Matter fabric: **Working**
- Read and write operations: **Working**
- Fast and large attribute reads: **Working**
- Latency performance: **Improved by approximately 25% compared with the previous release**

### UART Interface Testing

The following UART test scenarios were executed successfully:

- Continuous traffic
- Bidirectional traffic
- Stress traffic

### Observation

- After commissioning is completed, Matter functionality remains unaffected because BLE is not used during subsequent Matter operation.
