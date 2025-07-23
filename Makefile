CC = clang
CFLAGS = -Iinclude -Wall -Wextra -std=c17 `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
LDFLAGS = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0`
SRC := $(shell find src -name '*.c') third_party/cJSON.c
OUT = mocha-settings

all: build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)