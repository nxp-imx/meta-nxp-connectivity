CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
CPPFLAGS += -Isrc -Isrc/shared
BIN      = muxd

# The SHARED sources are the EXACT SAME C files compiled by the Zephyr
# companion app (samples/net/openthread/ble_ot_rcp_mux). muxd.c is the
# thin wire-up and src/port_linux.c is the Linux OS port; all wire-protocol
# logic lives in src/shared/.
SHARED_SRC = \
	src/shared/uart_mux.c \
	src/shared/hci_bridge.c \
	src/shared/spinel_bridge.c

MUXD_SRC = src/muxd.c src/port_linux.c $(SHARED_SRC)

all: $(BIN)

$(BIN): $(MUXD_SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(MUXD_SRC)

clean:
	rm -f $(BIN)

.PHONY: all clean
