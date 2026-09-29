/*
 * muxd - HCI/Spinel UART multiplexer daemon for MCXW72 (Linux host side)
 *
 * One physical UART (MCXW72 <-> Linux host) carries multiple logical channels.
 * On the Linux host, muxd creates two PTYs:
 *   - CHANNEL BLE    (0x02) -> PTY for BlueZ
 *   - CHANNEL Thread (0x01) -> PTY for OpenThread
 * Control/Diag/OTA channels are handled internally (logged / ignored for now).
 *
 * ON-WIRE FRAME FORMAT (matches the Zephyr uart_mux module byte-for-byte):
 *
 *   +------+---------+-------+-----------------+-------------------+-----------------+------+
 *   | SYNC | CHANNEL | FLAGS | LENGTH (16b LE) | PAYLOAD (LEN)     | FCS16 (16b LE)  | SYNC |
 *   | 0x7E | 1 B     | 1 B   | 2 B (0..2048)   | opaque bytes      | CRC-16/CCITT    | 0x7E |
 *   +------+---------+-------+-----------------+-------------------+-----------------+------+
 *            \____________ covered by FCS16 (CHANNEL+FLAGS+LENGTH+PAYLOAD) ____________/
 *
 *   - SYNC = 0x7E at BOTH start and end of every frame.
 *   - CHANNEL: 0x01=Thread, 0x02=BLE, 0x03=Control(internal), 0x04=Diag, 0x05=OTA.
 *   - FLAGS:   bit0 = VER, bits1-7 reserved (sent as 0).
 *   - LENGTH:  16-bit little-endian payload size, max 2048.
 *   - FCS16:   CRC-16/CCITT, poly 0x8408 (reflected), little-endian on the wire,
 *              computed over CHANNEL + FLAGS + LENGTH + PAYLOAD (fields 2..5).
 *   - PAYLOAD (Thread) = a complete native HDLC Spinel frame (forwarded verbatim).
 *   - PAYLOAD (BLE)    = a complete HCI H4 frame (type octet + data).
 *   - On FCS/format error: resync by scanning for the next SYNC (0x7E).
 *
 * Build:  cc -O2 -Wall -Wextra -o muxd muxd.c
 * Usage:  ./muxd --uart /dev/ttyLP4 --baud 1000000 [--no-flow] [--crc-init0] [-v]
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pty.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>


/* ---- shared frame constants (KEEP IN SYNC WITH Zephyr uart_mux) ---- */
#define MUX_SYNC          0x7E

#define MUX_CH_THREAD     0x01   /* -> Spinel PTY */
#define MUX_CH_BLE        0x02   /* -> HCI PTY    */
#define MUX_CH_CONTROL    0x03   /* internal      */
#define MUX_CH_DIAG       0x04   /* internal      */
#define MUX_CH_OTA        0x05   /* internal      */

#define MUX_FLAGS_VER     0x01   /* bit0 = protocol version */

#define MUX_PAYLOAD_MAX   2048
#define MUX_HCI_MAX       512    /* framing bound for the H4 assembler */
#define MUX_SPINEL_MAX    MUX_PAYLOAD_MAX
/* SYNC + CH + FLAGS + LEN(2) + PAYLOAD + FCS(2) + SYNC */
#define MUX_FRAME_MAX     (1 + 1 + 1 + 2 + MUX_PAYLOAD_MAX + 2 + 1)

static int g_verbose = 0;
static int g_crc_init = 0xFFFF;              /* default init; --crc-init0 -> 0x0000 */
static volatile sig_atomic_t g_stop = 0;

/*
 * Flow-control-free scan->initiating race mitigation (host side).
 *
 * On a 3-wire link (no RTS/CTS) BlueZ can push LE (Ext) Create Connection to
 * the wire immediately after LE Set Ext Scan Enable=0. The MCXW72 NBU needs a
 * few ms to leave the scanning state; if the create-connection arrives too
 * early it misses the transition and no Connection Complete is produced (the
 * host then times out and cancels). Running "btmon &" accidentally fixes this
 * by adding host-side latency. We reproduce that deterministically: delay
 * g_ble_settle_ms before forwarding a create-connection H4 command to the
 * UART. Default 25 ms; tune or disable with --ble-settle N (0 = off).
 */
static int g_ble_settle_ms = 50;

/* HCI opcodes that trigger the scan->initiating settle delay. */
#define HCI_OP_LE_CREATE_CONN      0x200d
#define HCI_OP_LE_EXT_CREATE_CONN  0x2043

static void sleep_ms(int ms)
{
    if (ms <= 0) {
        return;
    }
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}


static void on_sig(int s){ (void)s; g_stop = 1; }

static void logv(const char *fmt, ...) {
    if (!g_verbose) return;
    va_list ap; va_start(ap, fmt);
    vfprintf(stderr, fmt, ap); va_end(ap);
}

/* CRC-16/CCITT reflected, poly 0x8408 (reversed 0x1021). MUST match Zephyr side.
 * init selectable (0xFFFF default, or 0x0000 with --crc-init0). No final xor/reflect. */
static uint16_t fcs16(const uint8_t *d, size_t n)
{
    uint16_t crc = (uint16_t)g_crc_init;
    for (size_t i = 0; i < n; i++) {
        crc ^= d[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x0001) ? (uint16_t)((crc >> 1) ^ 0x8408)
                                 : (uint16_t)(crc >> 1);
    }
    return crc;
}
/* ---- UART setup: raw mode + optional RTS/CTS ---- */
static speed_t baud_to_speed(int baud)
{
    switch (baud) {
        case 115200:  return B115200;
        case 230400:  return B230400;
        case 460800:  return B460800;
        case 921600:  return B921600;
        case 1000000: return B1000000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 3000000: return B3000000;
        case 4000000: return B4000000;
        default:      return 0;
    }
}

static int uart_open(const char *path, int baud, int flow)
{
    int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) { perror("open uart"); return -1; }

    struct termios tio;
    if (tcgetattr(fd, &tio) < 0) { perror("tcgetattr"); close(fd); return -1; }
    cfmakeraw(&tio);

    speed_t sp = baud_to_speed(baud);
    if (sp == 0) { fprintf(stderr, "unsupported baud %d\n", baud); close(fd); return -1; }
    cfsetispeed(&tio, sp);
    cfsetospeed(&tio, sp);

    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~(size_t)CSIZE; tio.c_cflag |= CS8;
    tio.c_cflag &= ~(size_t)PARENB;
    tio.c_cflag &= ~(size_t)CSTOPB;
    if (flow) tio.c_cflag |= CRTSCTS; else tio.c_cflag &= ~(size_t)CRTSCTS;
    tio.c_cc[VMIN] = 0; tio.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tio) < 0) { perror("tcsetattr"); close(fd); return -1; }
    tcflush(fd, TCIOFLUSH);
    return fd;
}

/* ---- PTY setup: create a master, return master fd, print the slave name ---- */
static int pty_open(char *slave_name, size_t slen, const char *label)
{
    int m = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m < 0) { perror("posix_openpt"); return -1; }
    if (grantpt(m) < 0)  { perror("grantpt");  close(m); return -1; }
    if (unlockpt(m) < 0) { perror("unlockpt"); close(m); return -1; }
    if (ptsname_r(m, slave_name, slen) != 0) { perror("ptsname_r"); close(m); return -1; }

    /* raw on the slave so BlueZ / ot see a clean byte pipe */
    int s = open(slave_name, O_RDWR | O_NOCTTY);
    if (s >= 0) {
        struct termios tio;
        if (tcgetattr(s, &tio) == 0) { cfmakeraw(&tio); tcsetattr(s, TCSANOW, &tio); }
        close(s);
    }
    fprintf(stderr, "[muxd] %-6s PTY -> %s\n", label, slave_name);
    return m;
}


/* ================= TX: build a mux frame and write it to the UART ================= */
static int uart_write_all(int fd, const uint8_t *buf, size_t len)
{
    size_t off = 0;
    while (off < len && !g_stop) {
        ssize_t w = write(fd, buf + off, len - off);
        if (w < 0) {
            if (errno == EAGAIN || errno == EINTR) { usleep(200); continue; }
            perror("uart write"); return -1;
        }
        off += (size_t)w;
    }
    return 0;
}

static int mux_send(int uart_fd, uint8_t channel, const uint8_t *payload, size_t len)
{
    if (len > MUX_PAYLOAD_MAX) { fprintf(stderr, "tx payload too big %zu\n", len); return -1; }
    uint8_t f[MUX_FRAME_MAX];
    size_t p = 0;
    f[p++] = MUX_SYNC;
    f[p++] = channel;
    f[p++] = MUX_FLAGS_VER;                 /* FLAGS: version bit set */
    f[p++] = (uint8_t)(len & 0xFF);
    f[p++] = (uint8_t)((len >> 8) & 0xFF);
    memcpy(&f[p], payload, len); p += len;
    /* FCS16 over CHANNEL+FLAGS+LENGTH+PAYLOAD == f[1 .. p-1] */
    uint16_t crc = fcs16(&f[1], p - 1);
    f[p++] = (uint8_t)(crc & 0xFF);         /* little-endian on the wire */
    f[p++] = (uint8_t)((crc >> 8) & 0xFF);
    f[p++] = MUX_SYNC;                      /* trailing SYNC */
    logv("[muxd] TX ch=0x%02x len=%zu fcs=%04x\n", channel, len, crc);
    return uart_write_all(uart_fd, f, p);
}

/* ================= RX: state machine decoding mux frames from the UART ============ */
enum rx_state { S_SYNC, S_CH, S_FLAGS, S_L0, S_L1, S_PAYLOAD, S_F0, S_F1, S_ESYNC };

struct rx_ctx {
    enum rx_state st;
    uint8_t  ch;
    uint8_t  flags;
    uint16_t len;
    uint16_t got;
    uint16_t fcs_rx;
    uint8_t  payload[MUX_PAYLOAD_MAX];
};

static int rx_dispatch(struct rx_ctx *c, int hci_fd, int spinel_fd)
{
    /* recompute FCS over CHANNEL+FLAGS+LEN(2)+PAYLOAD */
    uint8_t hdr[4] = { c->ch, c->flags,
                       (uint8_t)(c->len & 0xFF), (uint8_t)((c->len >> 8) & 0xFF) };
    uint16_t crc = (uint16_t)g_crc_init;
    { const uint8_t *d = hdr; for (size_t i=0;i<4;i++){ crc^=d[i]; for(int b=0;b<8;b++) crc=(crc&1)?(uint16_t)((crc>>1)^0x8408):(uint16_t)(crc>>1);} }
    { for (uint16_t i=0;i<c->len;i++){ crc^=c->payload[i]; for(int b=0;b<8;b++) crc=(crc&1)?(uint16_t)((crc>>1)^0x8408):(uint16_t)(crc>>1);} }

    if (crc != c->fcs_rx) {
        fprintf(stderr, "[muxd] RX FCS error ch=0x%02x len=%u (calc=%04x rx=%04x) -> resync\n",
                c->ch, c->len, crc, c->fcs_rx);
        return -1;
    }
    switch (c->ch) {
    case MUX_CH_BLE:
        logv("[muxd] RX BLE len=%u -> hci pty\n", c->len);
        if (uart_write_all(hci_fd, c->payload, c->len) < 0)
            fprintf(stderr, "[muxd] write hci pty failed\n");
        break;
    case MUX_CH_THREAD:
        logv("[muxd] RX Thread len=%u -> spinel pty\n", c->len);
        if (uart_write_all(spinel_fd, c->payload, c->len) < 0)
            fprintf(stderr, "[muxd] write spinel pty failed\n");
        break;
    case MUX_CH_CONTROL:
    case MUX_CH_DIAG:
    case MUX_CH_OTA:
        logv("[muxd] RX internal ch=0x%02x len=%u (ignored)\n", c->ch, c->len);
        break;
    default:
        fprintf(stderr, "[muxd] RX unknown ch=0x%02x\n", c->ch);
        break;
    }
    return 0;
}

static void rx_feed(struct rx_ctx *c, const uint8_t *d, size_t n, int hci_fd, int spinel_fd)
{
    for (size_t i = 0; i < n; i++) {
        uint8_t b = d[i];
        switch (c->st) {
        case S_SYNC:
            if (b == MUX_SYNC) c->st = S_CH;
            break;
        case S_CH:
            if (b == MUX_SYNC) break;          /* tolerate back-to-back SYNC */
            /* Only accept a known channel id; otherwise this SYNC was spurious
             * (e.g. an orphan trailing SYNC followed by noise) -> keep hunting. */
            if (b == MUX_CH_THREAD || b == MUX_CH_BLE || b == MUX_CH_CONTROL ||
                b == MUX_CH_DIAG   || b == MUX_CH_OTA) {
                c->ch = b; c->st = S_FLAGS;
            } else {
                c->st = S_SYNC;
            }
            break;
        case S_FLAGS:
            c->flags = b; c->st = S_L0;
            break;
        case S_L0:
            c->len = b; c->st = S_L1;
            break;
        case S_L1:
            c->len |= (uint16_t)b << 8;
            if (c->len > MUX_PAYLOAD_MAX) {
                fprintf(stderr, "[muxd] RX bad len %u -> resync\n", c->len);
                c->st = S_SYNC;
            } else {
                c->got = 0;
                c->st = (c->len == 0) ? S_F0 : S_PAYLOAD;
            }
            break;
        case S_PAYLOAD:
            c->payload[c->got++] = b;
            if (c->got >= c->len) c->st = S_F0;
            break;
        case S_F0:
            c->fcs_rx = b; c->st = S_F1;
            break;
        case S_F1:
            c->fcs_rx |= (uint16_t)b << 8;
            if (rx_dispatch(c, hci_fd, spinel_fd) == 0) {
                /* FCS valid: a trailing SYNC is expected next (and may also open
                 * the next frame). */
                c->st = S_ESYNC;
            } else {
                /* FCS invalid: do NOT trust the trailing SYNC; hard resync. */
                c->st = S_SYNC;
            }
            break;
        case S_ESYNC:
            /* trailing SYNC (0x7E); it also serves as the opening SYNC of the
             * next frame -> go straight to CHANNEL. Anything else -> resync. */
            if (b == MUX_SYNC) {
                c->st = S_CH;
            } else {
                fprintf(stderr, "[muxd] RX missing trailing SYNC (got 0x%02x) -> resync\n", b);
                /* this byte cannot be a valid opening (must be SYNC) -> drop */
                c->st = S_SYNC;
            }
            break;
        }
    }
}
/* ===== H4 framer: split the BlueZ PTY byte stream into complete HCI H4 frames ===== */
/* H4 types: 0x01 CMD, 0x02 ACL, 0x03 SCO, 0x04 EVT, 0x05 ISO */
struct h4_ctx {
    uint8_t buf[MUX_HCI_MAX];
    size_t  have;      /* bytes accumulated */
    size_t  need;      /* full frame size once header parsed, 0 = unknown */
};

static size_t h4_header_len(uint8_t type)
{
    switch (type) {
        case 0x01: return 1 + 3;   /* type + opcode(2) + plen(1) */
        case 0x02: return 1 + 4;   /* type + handle(2) + dlen(2) */
        case 0x03: return 1 + 3;   /* type + handle(2) + dlen(1) */
        case 0x04: return 1 + 2;   /* type + code(1) + plen(1)   */
        case 0x05: return 1 + 4;   /* type + handle(2) + dlen(2, 12b used) */
        default:   return 0;       /* unknown -> desync */
    }
}

static size_t h4_total_len(const uint8_t *b)
{
    switch (b[0]) {
        case 0x01: return 1 + 3 + b[3];
        case 0x02: return 1 + 4 + (b[3] | ((size_t)b[4] << 8));
        case 0x03: return 1 + 3 + b[3];
        case 0x04: return 1 + 2 + b[2];
        case 0x05: return 1 + 4 + ((b[3] | ((size_t)b[4] << 8)) & 0x0FFF);
        default:   return 0;
    }
}

/* feed bytes from BlueZ PTY; on each complete H4 frame -> mux_send(HCI) */
static void h4_feed(struct h4_ctx *h, const uint8_t *d, size_t n, int uart_fd)
{
    for (size_t i = 0; i < n; i++) {
        if (h->have == 0) {
            size_t hl = h4_header_len(d[i]);
            if (hl == 0) { fprintf(stderr, "[muxd] H4 bad type 0x%02x -> drop\n", d[i]); continue; }
            h->buf[h->have++] = d[i];
            h->need = 0;
            continue;
        }
        if (h->have < sizeof(h->buf)) h->buf[h->have++] = d[i];
        if (h->need == 0) {
            size_t hl = h4_header_len(h->buf[0]);
            if (h->have >= hl) {
                h->need = h4_total_len(h->buf);
                if (h->need == 0 || h->need > sizeof(h->buf)) {
                    fprintf(stderr, "[muxd] H4 bad len -> resync\n");
                    h->have = 0; continue;
                }
            }
        }
        if (h->need && h->have >= h->need) {
            /*
             * Scan->initiating race mitigation (no RTS/CTS available).
             * If this complete H4 frame is an LE (Ext) Create Connection
             * command (type=0x01 CMD, opcode 0x2043 / 0x200d), pause a few
             * ms before pushing it to the UART so the NBU can leave the
             * scanning state first. Mirrors the "btmon &" side effect but
             * deterministically and only for the one command that needs it.
             */
            if (g_ble_settle_ms > 0 && h->need >= 3 && h->buf[0] == 0x01) {
                uint16_t opcode = (uint16_t)h->buf[1] |
                                  ((uint16_t)h->buf[2] << 8);

                if (opcode == HCI_OP_LE_EXT_CREATE_CONN ||
                    opcode == HCI_OP_LE_CREATE_CONN) {
                    logv("[muxd] create-conn (0x%04x): settle %d ms before TX\n",
                         opcode, g_ble_settle_ms);
                    sleep_ms(g_ble_settle_ms);
                }
            }
            mux_send(uart_fd, MUX_CH_BLE, h->buf, h->need);
            h->have = 0; h->need = 0;
        }

    }
}

/* ===== HDLC framer for Spinel PTY (MODE_RAW): split by 0x7E-delimited frames ===== */
struct hdlc_ctx {
    uint8_t buf[MUX_SPINEL_MAX];
    size_t  have;
    int     in_frame;
    int     esc;
};

/* We forward the raw HDLC frame verbatim (flags included) but must NOT treat an
 * ESCAPED 0x7E (i.e. 0x7D 0x5E) as a frame delimiter. We track the escape byte to
 * find the true closing flag. Bytes are stored verbatim; no unescaping is done. */
static void hdlc_feed(struct hdlc_ctx *h, const uint8_t *d, size_t n, int uart_fd)
{
    for (size_t i = 0; i < n; i++) {
        uint8_t b = d[i];
        if (h->esc) {
            /* previous byte was 0x7D: this byte is part of an escaped octet */
            if (h->have < sizeof(h->buf)) h->buf[h->have++] = b;
            h->esc = 0;
            continue;
        }
        if (b == 0x7D) {                 /* escape byte: keep it, next is escaped */
            if (h->have < sizeof(h->buf)) h->buf[h->have++] = b;
            h->esc = 1; h->in_frame = 1;
            continue;
        }
        if (b == 0x7E) {                 /* true frame flag */
            if (h->in_frame && h->have > 0) {
                uint8_t tmp[MUX_SPINEL_MAX];
                size_t p = 0;
                tmp[p++] = 0x7E;
                if (h->have + 2 <= sizeof(tmp)) {
                    memcpy(&tmp[p], h->buf, h->have); p += h->have;
                    tmp[p++] = 0x7E;
                    mux_send(uart_fd, MUX_CH_THREAD, tmp, p);
                }
            }
            h->have = 0; h->in_frame = 1;
        } else {
            h->in_frame = 1;
            if (h->have < sizeof(h->buf)) h->buf[h->have++] = b;
        }
    }
}


/* ============================== offline replay mode ============================== */
/* Reads a file of hex bytes (whitespace/newline/comma separated, '#'/'//' comments
 * allowed) representing the RAW UART byte stream MCXW72 -> host, feeds it through the
 * demux state machine, and writes the recovered payloads to:
 *   <out>_ble.bin    (CHANNEL BLE  -> HCI H4 payloads)
 *   <out>_thread.bin (CHANNEL Thread -> Spinel/HDLC payloads)
 * Useful for debugging framing/FCS/resync without hardware. */
static int hexnib(int ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

static int run_replay(const char *inpath, const char *outbase)
{
    FILE *fp = fopen(inpath, "rb");
    if (!fp) { perror("open replay file"); return 1; }

    char ble_path[256], thr_path[256];
    snprintf(ble_path, sizeof ble_path, "%s_ble.bin",    outbase);
    snprintf(thr_path, sizeof thr_path, "%s_thread.bin", outbase);
    int ble_fd = open(ble_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int thr_fd = open(thr_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (ble_fd < 0 || thr_fd < 0) { perror("open out"); fclose(fp); return 1; }

    /* parse hex bytes, skipping comments and non-hex separators */
    uint8_t *bytes = NULL; size_t cap = 0, nb = 0;
    int c, hi = -1, in_comment = 0, prev = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (in_comment) { if (c == '\n') in_comment = 0; continue; }
        if (c == '#') { in_comment = 1; continue; }
        if (c == '/' && prev == '/') { in_comment = 1; prev = 0; continue; }
        prev = c;
        int v = hexnib(c);
        if (v < 0) { hi = -1; continue; }           /* separator */
        if (hi < 0) { hi = v; }
        else {
            uint8_t byte = (uint8_t)((hi << 4) | v);
            hi = -1;
            if (nb >= cap) { cap = cap ? cap * 2 : 4096; bytes = realloc(bytes, cap); }
            bytes[nb++] = byte;
        }
    }
    fclose(fp);
    fprintf(stderr, "[muxd] replay: parsed %zu bytes from %s\n", nb, inpath);

    struct rx_ctx rx = { .st = S_SYNC };
    rx_feed(&rx, bytes, nb, ble_fd, thr_fd);       /* dispatch writes to the two files */

    free(bytes);
    close(ble_fd); close(thr_fd);
    fprintf(stderr, "[muxd] replay: wrote %s and %s\n", ble_path, thr_path);
    return 0;
}

/* ============================== main / poll loop ============================== */
static void usage(const char *p)
{
    fprintf(stderr,
      "Usage: %s --uart <dev> [--baud N] [--no-flow] [--crc-init0] [-v]\n"
      "       %s --replay <hexfile> [--out <base>] [--crc-init0] [-v]\n"
      "  --uart <dev>      physical UART to MCXW72 (e.g. /dev/ttyLP4)\n"
      "  --baud N          baud rate (default 1000000)\n"
      "  --no-flow         disable RTS/CTS (3-wire bring-up only)\n"
      "  --ble-settle N    ms to wait before forwarding LE (Ext) Create\n"
      "                    Connection to the NBU (default 25, 0 = off).\n"
      "                    Mitigates the scan->initiating race on 3-wire links.\n"
      "  --crc-init0       use FCS16 init 0x0000 instead of 0xFFFF\n"

      "  --replay <file>   offline: decode a hex dump of the UART RX stream\n"
      "  --out <base>      replay output base name (default 'replay')\n"
      "  -v                verbose\n", p, p);
}

int main(int argc, char **argv)
{
    const char *uart_path = NULL;
    const char *replay_path = NULL;
    const char *out_base = "replay";
    int baud = 1000000, flow = 1;

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "--uart") && i+1 < argc) uart_path = argv[++i];
        else if (!strcmp(argv[i], "--baud") && i+1 < argc) baud = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--replay") && i+1 < argc) replay_path = argv[++i];
        else if (!strcmp(argv[i], "--out") && i+1 < argc) out_base = argv[++i];
        else if (!strcmp(argv[i], "--no-flow"))  flow = 0;
        else if (!strcmp(argv[i], "--ble-settle") && i+1 < argc) g_ble_settle_ms = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--crc-init0")) g_crc_init = 0x0000;

        else if (!strcmp(argv[i], "-v")) g_verbose = 1;
        else { usage(argv[0]); return 2; }
    }

    /* Offline replay mode: no UART / no PTY, just decode a hex dump. */
    if (replay_path) {
        fprintf(stderr, "[muxd] replay mode, fcs_init=0x%04X\n", g_crc_init);
        return run_replay(replay_path, out_base);
    }

    if (!uart_path) { usage(argv[0]); return 2; }

    signal(SIGINT,  on_sig);
    signal(SIGTERM, on_sig);
    signal(SIGPIPE, SIG_IGN);

    int uart_fd = uart_open(uart_path, baud, flow);
    if (uart_fd < 0) return 1;
    fprintf(stderr, "[muxd] UART %s @ %d baud, flow=%s, fcs_init=0x%04X\n",
            uart_path, baud, flow?"RTS/CTS":"none", g_crc_init);

    char hci_name[128], spinel_name[128];
    int hci_fd    = pty_open(hci_name,    sizeof(hci_name),    "BLE/HCI");
    int spinel_fd = pty_open(spinel_name, sizeof(spinel_name), "THREAD");
    if (hci_fd < 0 || spinel_fd < 0) return 1;

    fprintf(stderr,
        "[muxd] ready.\n"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
        "  BlueZ     : btattach -B %s -P h4 &\n"
        "  OpenThread: otbr-agent-iwxxx -d 1 -I wpan0 -B mlan0 'spinel+hdlc+uart://%s?uart-baudrate=%d' trel://mlan0 &\n"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"        ,
        hci_name, spinel_name, baud);

    struct rx_ctx   rx  = { .st = S_SYNC };
    struct h4_ctx   h4  = { 0 };
    struct hdlc_ctx hd  = { 0 };
    uint8_t buf[4096];

    struct pollfd pfd[3];
    pfd[0].fd = uart_fd;   pfd[0].events = POLLIN;
    pfd[1].fd = hci_fd;    pfd[1].events = POLLIN;
    pfd[2].fd = spinel_fd; pfd[2].events = POLLIN;

    while (!g_stop) {
        int r = poll(pfd, 3, 500);
        if (r < 0) { if (errno == EINTR) continue; perror("poll"); break; }
        if (r == 0) continue;

        if (pfd[0].revents & POLLIN) {                 /* UART -> demux -> PTYs */
            ssize_t n = read(uart_fd, buf, sizeof(buf));
            if (n > 0) rx_feed(&rx, buf, (size_t)n, hci_fd, spinel_fd);
        }
        if (pfd[1].revents & POLLIN) {                 /* BlueZ PTY -> H4 -> mux(BLE) */
            ssize_t n = read(hci_fd, buf, sizeof(buf));
            if (n > 0) h4_feed(&h4, buf, (size_t)n, uart_fd);
        }
        if (pfd[2].revents & POLLIN) {                 /* ot -> HDLC -> mux(Thread) */
            ssize_t n = read(spinel_fd, buf, sizeof(buf));
            if (n > 0) hdlc_feed(&hd, buf, (size_t)n, uart_fd);
        }
        for (int k = 0; k < 3; k++)
            if (pfd[k].revents & (POLLERR | POLLNVAL))
                fprintf(stderr, "[muxd] poll error on fd %d\n", pfd[k].fd);
    }

    fprintf(stderr, "\n[muxd] stopping.\n");
    close(uart_fd); close(hci_fd); close(spinel_fd);
    return 0;
}
