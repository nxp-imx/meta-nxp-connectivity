CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
LDFLAGS ?=
BIN      = muxd

all: $(BIN)

$(BIN): src/muxd.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(BIN)

.PHONY: all clean
