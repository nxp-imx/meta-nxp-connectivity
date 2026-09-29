#!/usr/bin/env python3
# Copyright 2024 NXP
# SPDX-License-Identifier: Apache-2.0
#
# muxd.py - Linux userspace demux daemon for uart_mux firmware.
# NO external dependencies: uses only Python stdlib (termios, os, fcntl).
#
# Frame format (from uart_mux.h - AUTHORITATIVE):
#   [0x7E][CHANNEL 1B][FLAGS=0x01 1B][LEN 2B LE][PAYLOAD LEN B][FCS16 2B LE][0x7E]
#
# Channel IDs:
#   0x01 = UART_MUX_CH_THREAD  (OT Spinel/HDLC)
#   0x02 = UART_MUX_CH_BLE     (HCI H4)
#
# FCS: CRC-16/CCITT reflected, poly 0x8408, init 0xFFFF, no final XOR,
#      computed over CHANNEL + FLAGS + LEN[0] + LEN[1] + PAYLOAD.
#
# Usage:
#   python3 muxd.py --uart /dev/ttyLP4 --baud 115200 [-v]
#
#   Creates two PTYs and prints their paths:
#     BLE HCI PTY:    /dev/pts/X 
#     OT Spinel PTY:  /dev/pts/Y

import argparse
import fcntl
import os
import pty
import struct
import sys
import termios
import threading

# ---------------------------------------------------------------------------
# Protocol constants
# ---------------------------------------------------------------------------
SYNC       = 0x7E
FLAGS_VER1 = 0x01

CH_THREAD  = 0x01   # OT Spinel/HDLC
CH_BLE     = 0x02   # BLE HCI H4
VALID_CH   = {CH_THREAD, CH_BLE}

MAX_PAYLOAD = 2048

# ---------------------------------------------------------------------------
# CRC-16/CCITT reflected (poly 0x8408, init 0xFFFF)
# ---------------------------------------------------------------------------
def _build_crc_table():
    table = []
    for i in range(256):
        crc = i
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0x8408
            else:
                crc >>= 1
        table.append(crc)
    return table

_CRC_TABLE = _build_crc_table()

def fcs16(data: bytes, init: int = 0xFFFF) -> int:
    crc = init
    for b in data:
        crc = (crc >> 8) ^ _CRC_TABLE[(crc ^ b) & 0xFF]
    return crc

# ---------------------------------------------------------------------------
# Frame builder
# ---------------------------------------------------------------------------
def build_frame(ch: int, payload: bytes) -> bytes:
    flags = FLAGS_VER1
    ln = len(payload)
    hdr = bytes([ch, flags, ln & 0xFF, (ln >> 8) & 0xFF])
    crc = fcs16(hdr + payload)
    return bytes([SYNC]) + hdr + payload + struct.pack('<H', crc) + bytes([SYNC])

# ---------------------------------------------------------------------------
# Frame parser (state machine)
# ---------------------------------------------------------------------------
class FrameParser:
    ST_HUNT    = 0
    ST_CHANNEL = 1
    ST_FLAGS   = 2
    ST_LEN0    = 3
    ST_LEN1    = 4
    ST_DATA    = 5
    ST_CRC0    = 6
    ST_CRC1    = 7

    def __init__(self, on_frame, verbose=False):
        self._on_frame = on_frame
        self._verbose  = verbose
        self._state    = self.ST_HUNT
        self._ch       = 0
        self._flags    = 0
        self._ln       = 0
        self._buf      = bytearray()
        self._crc_got  = 0

    def feed(self, data: bytes):
        for b in data:
            self._step(b)

    def _step(self, b):
        s = self._state
        if s == self.ST_HUNT:
            if b == SYNC:
                self._state = self.ST_CHANNEL
        elif s == self.ST_CHANNEL:
            if b == SYNC:
                return  # back-to-back SYNCs: stay
            if b in VALID_CH:
                self._ch    = b
                self._state = self.ST_FLAGS
            else:
                if self._verbose:
                    print(f"[muxd] unknown ch={b:#04x}, resyncing", file=sys.stderr)
                self._state = self.ST_HUNT
        elif s == self.ST_FLAGS:
            self._flags = b
            self._state = self.ST_LEN0
        elif s == self.ST_LEN0:
            self._ln    = b
            self._state = self.ST_LEN1
        elif s == self.ST_LEN1:
            self._ln |= (b << 8)
            if self._ln == 0 or self._ln > MAX_PAYLOAD:
                print(f"[muxd] bad LEN={self._ln}, resyncing", file=sys.stderr)
                self._state = self.ST_HUNT
                return
            self._buf   = bytearray()
            self._state = self.ST_DATA
        elif s == self.ST_DATA:
            self._buf.append(b)
            if len(self._buf) == self._ln:
                self._state = self.ST_CRC0
        elif s == self.ST_CRC0:
            self._crc_got = b
            self._state   = self.ST_CRC1
        elif s == self.ST_CRC1:
            self._crc_got |= (b << 8)
            self._validate()
            # trailing SYNC opens the next frame immediately
            if b == SYNC:
                self._state = self.ST_CHANNEL
            else:
                self._state = self.ST_HUNT

    def _validate(self):
        hdr     = bytes([self._ch, self._flags,
                         self._ln & 0xFF, (self._ln >> 8) & 0xFF])
        crc_exp = fcs16(hdr + bytes(self._buf))
        if self._crc_got != crc_exp:
            print(f"[muxd] FCS error ch={self._ch:#04x} "
                  f"got={self._crc_got:#06x} exp={crc_exp:#06x}, resyncing",
                  file=sys.stderr)
            self._state = self.ST_HUNT
            return
        if self._verbose:
            print(f"[muxd] RX ch={self._ch:#04x} len={self._ln} "
                  f"data={bytes(self._buf)[:16].hex()}", file=sys.stderr)
        self._on_frame(self._ch, bytes(self._buf))
        self._state = self.ST_HUNT

# ---------------------------------------------------------------------------
# Raw UART open via termios (no pyserial)
# ---------------------------------------------------------------------------
_BAUD_MAP = {
    50:      termios.B50,
    75:      termios.B75,
    110:     termios.B110,
    134:     termios.B134,
    150:     termios.B150,
    200:     termios.B200,
    300:     termios.B300,
    600:     termios.B600,
    1200:    termios.B1200,
    1800:    termios.B1800,
    2400:    termios.B2400,
    4800:    termios.B4800,
    9600:    termios.B9600,
    19200:   termios.B19200,
    38400:   termios.B38400,
    57600:   termios.B57600,
    115200:  termios.B115200,
    230400:  termios.B230400,
    460800:  termios.B460800,
    500000:  termios.B500000,
    576000:  termios.B576000,
    921600:  termios.B921600,
    1000000: termios.B1000000,
    1152000: termios.B1152000,
    1500000: termios.B1500000,
    2000000: termios.B2000000,
}

def open_uart(path: str, baud: int) -> int:
    """Open a serial port in raw mode, return its file descriptor."""
    baud_const = _BAUD_MAP.get(baud)
    if baud_const is None:
        raise ValueError(f"Unsupported baud rate {baud}. "
                         f"Supported: {sorted(_BAUD_MAP.keys())}")

    fd = os.open(path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)

    # Switch to blocking I/O
    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, flags & ~os.O_NONBLOCK)

    # Configure termios: raw 8N1, no flow control
    attrs = termios.tcgetattr(fd)
    # iflag: disable all input processing
    attrs[0] = 0  # IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON = 0
    # oflag: disable output processing
    attrs[1] = 0
    # cflag: 8 bits, no parity, 1 stop, local, enable receiver, no flow
    attrs[2] = termios.CS8 | termios.CLOCAL | termios.CREAD
    # lflag: raw (no echo, no signals, no canonical)
    attrs[3] = 0
    # Special characters
    attrs[4] = baud_const   # ispeed
    attrs[5] = baud_const   # ospeed
    # VMIN=1, VTIME=0: block until at least 1 byte
    attrs[6][termios.VMIN]  = 1
    attrs[6][termios.VTIME] = 0

    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    termios.tcflush(fd, termios.TCIOFLUSH)
    return fd

# ---------------------------------------------------------------------------
# PTY helpers
# ---------------------------------------------------------------------------
def open_pty(name: str):
    master_fd, slave_fd = pty.openpty()
    slave_path = os.ttyname(slave_fd)
    # Set slave PTY to raw mode so clients get unmodified bytes
    attrs = termios.tcgetattr(slave_fd)
    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = termios.CS8 | termios.CLOCAL | termios.CREAD
    attrs[3] = 0
    attrs[6][termios.VMIN]  = 1
    attrs[6][termios.VTIME] = 0
    termios.tcsetattr(slave_fd, termios.TCSANOW, attrs)
    print(f"[muxd] {name} PTY: {slave_path}")
    return master_fd, slave_fd, slave_path

# ---------------------------------------------------------------------------
# Main daemon
# ---------------------------------------------------------------------------
def run(uart_path: str, baud: int, verbose: bool):
    print(f"[muxd] opening {uart_path} at {baud} baud (stdlib termios, no pyserial)")
    uart_fd = open_uart(uart_path, baud)

    ble_master, ble_slave, ble_path = open_pty("BLE HCI")
    ot_master,  ot_slave,  ot_path  = open_pty("OT Spinel")

    print(f"[muxd] BLE HCI  -> {ble_path}")
    print(f"[muxd]   use with: btattach -B {ble_path} -P h4")
    print(f"[muxd] OT Spinel-> {ot_path}")
    print(f"[muxd]   use with: otbr-agent-iwxxx -d 1 -I wpan0 -B mlan0 'spinel+hdlc+uart://{ot_path}?uart-baudrate={baud}' trel://mlan0")

    ch_to_pty = {CH_BLE: ble_master, CH_THREAD: ot_master}
    pty_to_ch = {ble_master: CH_BLE, ot_master: CH_THREAD}

    tx_lock = threading.Lock()

    def on_frame(ch, payload):
        fd = ch_to_pty.get(ch)
        if fd is not None:
            try:
                os.write(fd, payload)
            except OSError as e:
                print(f"[muxd] write to PTY ch={ch:#04x} failed: {e}",
                      file=sys.stderr)

    parser = FrameParser(on_frame, verbose=verbose)

    def send_to_device(ch, payload):
        frame = build_frame(ch, payload)
        if verbose:
            print(f"[muxd] TX ch={ch:#04x} len={len(payload)} "
                  f"data={payload[:16].hex()}", file=sys.stderr)
        with tx_lock:
            # Write in a loop in case of short writes
            view = memoryview(frame)
            sent = 0
            while sent < len(frame):
                n = os.write(uart_fd, view[sent:])
                sent += n

    def pty_reader(master_fd, ch):
        while True:
            try:
                data = os.read(master_fd, 4096)
                if data:
                    send_to_device(ch, data)
            except OSError:
                break

    for mfd, ch in pty_to_ch.items():
        t = threading.Thread(target=pty_reader, args=(mfd, ch), daemon=True)
        t.start()

    print("[muxd] running, Ctrl-C to stop")
    try:
        while True:
            data = os.read(uart_fd, 256)
            if data:
                if verbose:
                    print(f"[muxd] raw RX {len(data)}B: {data[:32].hex()}",
                          file=sys.stderr)
                parser.feed(data)
    except KeyboardInterrupt:
        print("[muxd] stopped")
    finally:
        os.close(uart_fd)

def main():
    ap = argparse.ArgumentParser(
        description="uart_mux demux daemon (no pyserial - stdlib termios only)")
    ap.add_argument("--uart", default="/dev/ttyLP4", help="UART device path")
    ap.add_argument("--baud", type=int, default=115200, help="Baud rate")
    ap.add_argument("-v", "--verbose", action="store_true",
                    help="Print raw RX bytes and each decoded frame")
    args = ap.parse_args()
    run(args.uart, args.baud, args.verbose)

if __name__ == "__main__":
    main()
