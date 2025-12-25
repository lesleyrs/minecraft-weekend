UNAME_S = $(shell uname -s)

CC = emcc
CFLAGS = -std=c11 -O3 -flto -Wall -Wextra -Wpedantic -Wstrict-aliasing
#-g
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -Ilib -fbracket-depth=1024 -D_GNU_SOURCE
LDFLAGS = -lm -Wl,--stack-first -Wl,-z,stack-size=8388608 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 --preload-file=res -sALLOW_MEMORY_GROWTH -sUSE_GLFW=3
#--shell-file=shell_minimal.html

SRC  = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
SRC += lib/noise1234.c
OBJ  = $(SRC:.c=.o)
BIN = bin
GAME = minecraft-weekend.html

.PHONY: all clean

all: dirs $(GAME)

dirs:
	mkdir -p ./$(BIN)

run: all
	esbuild --servedir=.

$(GAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)

