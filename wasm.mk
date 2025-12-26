UNAME_S = $(shell uname -s)

CC = clang --target=wasm32 --sysroot=../../wasmlite/libc -nodefaultlibs
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -Ilib -fbracket-depth=1024
CFLAGS += -D__STDC_WANT_LIB_EXT1__
CFLAGS += -fno-builtin-pow -fno-builtin-sin -fno-builtin-cos -fno-builtin-tan -fno-builtin-fmod -fno-builtin-fmodf -fno-builtin-cosf -fno-builtin-sinf -fno-builtin-powf
LDFLAGS = -lm -Wl,--export-table -Wl,--stack-first -Wl,-z,stack-size=8388608 -msimd128

DEBUG = 1

ifeq ($(DEBUG),0)
CFLAGS+=-O3 -flto #-ffast-math
LDFLAGS+=-lc
else
CFLAGS+=-g
LDFLAGS+=-lc-dbg
endif

SRC  = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
SRC += lib/noise1234.c
OBJ  = $(SRC:.c=.o)
BIN = bin
GAME = minecraft-weekend.wasm

.PHONY: all clean

all: dirs $(GAME)

dirs:
	mkdir -p ./$(BIN)

run: all
	esbuild --servedir=.

$(GAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
ifeq ($(DEBUG),0)
	wasm-strip $@ && wasm-opt $@ -o $@ -O3 --enable-sign-ext --enable-simd
else
	../../tools/emscripten/tools/wasm-sourcemap.py $@ -w $@ -p $(CURDIR) -s -u ./$@.map -o $@.map --dwarfdump=../../../emsdk/upstream/bin/llvm-dwarfdump
	# ../../tools/emscripten/tools/wasm-sourcemap.py $@ -w $@ -p $(CURDIR) -s -u ./$@.map -o $@.map --dwarfdump=/usr/bin/llvm-dwarfdump
endif

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)
