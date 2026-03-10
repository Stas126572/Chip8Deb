/* 
 * Copyright (C) 2024 Manichev Stanislav (https://github.com/Stas126572)
 * Licensed under GNU GPL v3.0.
 * 
 * ДОПОЛНИТЕЛЬНОЕ УСЛОВИЕ (Section 7): 
 * Любые изменения в этом коде должны сопровождаться сохранением 
 * оригинального уведомления об авторстве и ссылки на репозиторий.
 * Удаление визуальных уведомлений об авторстве в интерфейсе программы запрещено.
 */
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
