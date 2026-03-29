# Makefile — MSYS2 UCRT64 + SDL3
CC      := gcc
CFLAGS  := -g -O0 -Wall -Wextra -std=c17 -I/ucrt64/include
LDFLAGS := -L/ucrt64/lib
LDLIBS  := -lSDL3 -lSDL3_image -lSDL3_ttf

SRC := src/main.c src/image_ops.c src/histogram.c src/ui.c
BIN := app.exe

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

run: $(BIN)
	./$(BIN) assets/lena.png

gdb: $(BIN)
	gdb ./$(BIN)

clean:
	rm -f $(BIN) *.o
