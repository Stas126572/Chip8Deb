CC = gcc
CFLAGS = -Wall -g3
LIBS = -lncurses
GENERATOR_OUT = disasmTableGenerator.out

.PHONY: all clean

all: chip8_emu

chip8_emu: disasm.bin main.c
        @$(CC) $(CFLAGS) main.c -o $@ $(LIBS)

disasm.bin: disasmTableGenerator.c
        @$(CC) $(CFLAGS) -O3 $< -o $(GENERATOR_OUT)
        @./$(GENERATOR_OUT)
        @$(RM) $(GENERATOR_OUT)

clean:
        @$(RM) chip8_emu disasm.bin $(GENERATOR_OUT)
