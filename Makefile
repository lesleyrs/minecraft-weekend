UNAME_S = $(shell uname -s)

CC = clang
CFLAGS = -std=c11 -O3 -flto -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -Ilib -fbracket-depth=1024 -D_GNU_SOURCE
LDFLAGS = -lm -lglfw

# GLFW required frameworks on OSX
ifeq ($(UNAME_S), Darwin)
	LDFLAGS += -framework OpenGL -framework IOKit -framework CoreVideo -framework Cocoa
endif

# ifeq ($(UNAME_S), Linux)
# 	LDFLAGS += -ldl -lpthread
# endif

SRC  = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
SRC += lib/gles2.c lib/noise1234.c
OBJ  = $(SRC:.c=.o)
BIN = bin

.PHONY: all clean

all: dirs game

dirs:
	mkdir -p ./$(BIN)

run: all
	$(BIN)/game

game: $(OBJ)
	$(CC) -o $(BIN)/game $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)
