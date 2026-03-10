//!main.c
/* 
 * Copyright (C) 2024 Manichev Stanislav (https://github.com/Stas126572)
 * Licensed under GNU GPL v3.0.
 * 
 * ДОПОЛНИТЕЛЬНОЕ УСЛОВИЕ (Section 7): 
 * Любые изменения в этом коде должны сопровождаться сохранением 
 * оригинального уведомления об авторстве и ссылки на репозиторий.
 * Удаление визуальных уведомлений об авторстве в интерфейсе программы запрещено.
 */

#include <fcntl.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
uint8_t *RAM;
uint8_t *HISTORY;
struct Chip8 {
  uint32_t stack[16];
  uint16_t PC, I, startPoint, keys;
  uint8_t del_time, soun_time, SP, flags;
  uint8_t regs[16];
};
struct Chip8 chip8;
uint64_t screen_rows[32];
const char (*disasm_table)[8];
uint8_t bp_bitmap[4096] = {0};
uint16_t changed_times = 0;
#define IS_SP(a) (chip8.startPoint == (a))
#define GET_BP_LEVEL(a) (bp_bitmap[(a)] & 0x0F)
#define SET_BP_LEVEL(a, level) (bp_bitmap[(a)] = ((uint8_t)level))
#define SET_STOP_BP_LEVEL(level) (chip8.flags = ((uint8_t)(level)))
#define GET_STOP_BP_LEVEL (chip8.flags)

int step() {
  uint16_t opcode = (RAM[chip8.PC] << 8) | RAM[chip8.PC + 1];
  int jumped = 0;

  if (opcode == 0x0000)
    return 1;
  if (opcode == 0x00E0)
    memset(screen_rows, 0, sizeof(screen_rows));
  if (opcode == 0x00EE) {
    chip8.PC = (uint16_t)(chip8.stack[chip8.SP--]);
    jumped = 1;
  }

  if ((opcode & 0xF000) == 0x1000) {
    chip8.PC = opcode & 0x0FFF;
    jumped = 1;
  }
  if ((opcode & 0xF000) == 0x2000) {
    chip8.stack[++chip8.SP] = chip8.PC + 2;
    chip8.PC = opcode & 0x0FFF;
    jumped = 1;
  }

  if (((opcode & 0xF000) == 0x3000) &&
      (chip8.regs[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)))
    chip8.PC += 2;
  if (((opcode & 0xF000) == 0x4000) &&
      (chip8.regs[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)))
    chip8.PC += 2;

  if ((opcode & 0xF000) == 0x6000)
    chip8.regs[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
  if ((opcode & 0xF000) == 0x7000)
    chip8.regs[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
  if ((opcode & 0xA000) == 0xA000)
    chip8.I = opcode & 0x0FFF;

  // DRW
  if ((opcode & 0xF000) == 0xD000) {
    uint8_t x = chip8.regs[(opcode & 0x0F00) >> 8] % 64;
    uint8_t y = chip8.regs[(opcode & 0x00F0) >> 4] % 32;
    uint8_t n = opcode & 0x000F;
    chip8.regs[15] = 0;
    for (int i = 0; i < n && (y + i) < 32; i++) {
      uint64_t sprite = (uint64_t)RAM[chip8.I + i] << (56 - x);
      chip8.regs[15] = screen_rows[y + i] && sprite;
      screen_rows[y + i] ^= sprite;
    }
  }

  if ((opcode & 0xF0FF) == 0xE09E) {
    if (chip8.keys & (1 << chip8.regs[(opcode & 0x0F00) >> 8]))
      chip8.PC += 2;
  }
  if ((opcode & 0xF0FF) == 0xE0A1) {
    if (!(chip8.keys & (1 << chip8.regs[(opcode & 0x0F00) >> 8])))
      chip8.PC += 2;
  }
  if ((opcode & 0xF0FF) == 0xF00A) {

    if (chip8.keys == 0) {
      jumped = 1;
    } else {
      for (int i = 0; i < 16; i++) {
        if (chip8.keys & (1 << i)) {
          chip8.regs[(opcode & 0x0F00) >> 8] = i;
          break;
        }
      }
    }
  }

  if (!jumped)
    chip8.PC += 2;
  return 0;
}

void draw_dis(WINDOW *dis, uint16_t cur) {
  box(dis, 0, 0);
  mvwprintw(dis, 0, 2, "DISASM");
  for (int i = 0; i < 32; i++) {
    uint16_t addr = (chip8.PC + (i * 2)) & 0xFFF;
    uint16_t op = (RAM[addr] << 8) | RAM[addr + 1];

    wattron(dis, COLOR_PAIR(2));
    mvwprintw(dis, i + 1, 1, "%01X%c", GET_BP_LEVEL(addr),
              (IS_SP(addr) ? 'S' : ' '));
    wattroff(dis, COLOR_PAIR(2));

    if (addr == chip8.PC)
      wattron(dis, A_BOLD | COLOR_PAIR(2));
    if (addr == cur)
      wattron(dis, A_REVERSE);

    mvwprintw(dis, i + 1, 3, "%04X: %.8s | %04X", addr, disasm_table[op], op);
    wattroff(dis, A_REVERSE | A_BOLD | COLOR_PAIR(2));
  }
  wrefresh(dis);
}
void draw_reg(WINDOW *reg) {
  box(reg, 0, 0);
  mvwprintw(reg, 0, 2, " REGS ");
  for (int i = 0; i < 16; i++)
    mvwprintw(reg, i + 1, 2, "V%X: %02X", i, chip8.regs[i]);
  mvwprintw(reg, 18, 2, "BP: %02X", chip8.flags);
  mvwprintw(reg, 20, 2, "PC: %04X", chip8.PC);
  mvwprintw(reg, 21, 2, "II: %04X", chip8.I);
  mvwprintw(reg, 22, 2, "SP: %04X", chip8.SP);
  mvwprintw(reg, 23, 2, "sp: %04X", chip8.startPoint);
  mvwprintw(reg, 24, 2, "HS: %04X", (HISTORY[0] << 8) | (HISTORY[1]));
  wrefresh(reg);
}
void draw_scr(WINDOW *scr) {
  box(scr, 0, 0);
  for (int y = 0; y < 32; y++)
    for (int x = 0; x < 64; x++)
      if (screen_rows[y] & (1ULL << (63 - x)))
        mvwaddch(scr, y + 1, x + 1, '*');
  wrefresh(scr);
}

int main(int argc, char **argv) {
  if (argc < 2)
    return 1;

  int fd_t = open("disasm.bin", O_RDONLY);
  disasm_table = mmap(NULL, 65536 * 8, PROT_READ, MAP_PRIVATE, fd_t, 0);
  if (disasm_table == MAP_FAILED) {
    perror("MMAP(Disasm table): ");
    close(fd_t);
    return -1;
  }
  int fd_m = open(argv[1], O_RDWR);
  if (fd_m == -1) {
    perror("Open memory file desriptor: ");
    return -1;
  }
  RAM = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd_m, 0);
  if (RAM == MAP_FAILED) {
    perror("MMAP(Ram): ");
    close(fd_t);
    close(fd_m);
    return -1;
  }

  mkdir("./history_data", 0777);
  int fd_h = open(strcat("./history_data/history_", argv[1]), O_RDWR | O_CREAT, 0666);
  if (fd_h == -1) {
    perror("Open: ");
    return -1;
  }
  if ((ftruncate(fd_h, 2 * 1024 * 1024) == -1)) {
    perror("Fturncate: ");
    return 1;
  }
  HISTORY =
      mmap(NULL, 2 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd_h, 0);
  if (HISTORY == MAP_FAILED) {
    perror("MMAP(History): ");
    close(fd_t);
    close(fd_m);
    munmap((void *)disasm_table, 65536 * 8);
    munmap(RAM, 4096);
    close(fd_h);
    return -1;
  }

  initscr();
  start_color();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  timeout(100);
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);
  init_pair(3, COLOR_WHITE, COLOR_BLACK);

  int my, mx;
  getmaxyx(stdscr, my, mx);
  WINDOW *wd = newwin(34, 25, 0, 0);
  WINDOW *wr = newwin(34, 20, 0, 26);
  struct timeval last_timer_update, current_time;
  gettimeofday(&last_timer_update, NULL);
  chip8.del_time = 60;
  long long accumulator = 0;
  WINDOW *ws = newwin(34, 66, 0, 47);

  uint16_t cur = 0x200;
  chip8.startPoint = 0x200;
  chip8.PC = chip8.startPoint;
  int run = 0;

  int ch;
  while ((ch = getch()) != 'Q') {

    gettimeofday(&current_time, NULL);
    long long dt =
        (current_time.tv_sec - last_timer_update.tv_sec) * 1000000LL +
        (current_time.tv_usec - last_timer_update.tv_usec);
    last_timer_update = current_time;

    chip8.keys = 0;

    if (ch == '1')
      chip8.keys |= (1 << 0x1);
    if (ch == '2')
      chip8.keys |= (1 << 0x2);
    if (ch == '3')
      chip8.keys |= (1 << 0x3);
    if (ch == '4')
      chip8.keys |= (1 << 0xC);

    if (ch == 'q')
      chip8.keys |= (1 << 0x4);
    if (ch == 'w')
      chip8.keys |= (1 << 0x5);
    if (ch == 'e')
      chip8.keys |= (1 << 0x6);
    if (ch == 'r')
      chip8.keys |= (1 << 0xD);

    if (ch == 'a')
      chip8.keys |= (1 << 0x7);
    if (ch == 's')
      chip8.keys |= (1 << 0x8);
    if (ch == 'd')
      chip8.keys |= (1 << 0x9);
    if (ch == 'f')
      chip8.keys |= (1 << 0xE);

    if (ch == 'z')
      chip8.keys |= (1 << 0xA);
    if (ch == 'x')
      chip8.keys |= (1 << 0x0);
    if (ch == 'c')
      chip8.keys |= (1 << 0xB);
    if (ch == 'v')
      chip8.keys |= (1 << 0xF);

    if (ch == KEY_DOWN)
      cur = (cur + 2) & 0xFFF;
    if (ch == KEY_UP)
      cur = (cur - 2) & 0xFFF;
    if (ch == 'A')
      SET_BP_LEVEL(cur, GET_BP_LEVEL(cur) - 1);
    if (ch == 'D')
      SET_BP_LEVEL(cur, GET_BP_LEVEL(cur) + 1);
    if (ch == 'X')
      SET_STOP_BP_LEVEL(GET_STOP_BP_LEVEL - 1);
    if (ch == 'Z')
      SET_STOP_BP_LEVEL(GET_STOP_BP_LEVEL + 1);
    if (ch == 'S') {
      chip8.PC = chip8.startPoint;
      cur = chip8.startPoint;
    }
    if (ch == 'P') {

      HISTORY[0] += !(HISTORY[1] ^ 0xFF);
      HISTORY[1]++;
      uint32_t base = 2 + (((HISTORY[0] << 8) | HISTORY[1]) << 3);

      HISTORY[base] = RAM[cur];
      HISTORY[base + 1] = RAM[cur + 1];
      HISTORY[base + 2] = RAM[cur];
      HISTORY[base + 3] = RAM[cur + 1];

      HISTORY[base + 4] = cur >> 4;

      HISTORY[base + 5] = ((cur & 0x0F) << 4) | (cur >> 8);
      HISTORY[base + 6] = cur & 0xFF;

      HISTORY[base + 7] = GET_BP_LEVEL(chip8.PC) << 4;

      chip8.startPoint = cur;

      changed_times = 0;
    }
    if (ch == 'G')
      run = !run;
    if (ch == ' ') {

      run = 0;
      step();
    }
    if (ch == 'C') {
      timeout(-1);
      echo();
      curs_set(1);
      char b[5];
      mvprintw(33, 0, "EDIT %04X: ", cur);
      getnstr(b, 4);
      uint16_t v = (uint16_t)strtol(b, NULL, 16);

      HISTORY[0] += !(HISTORY[1] ^ 0xFF);
      HISTORY[1]++;
      uint32_t base = 2 + (((HISTORY[0] << 8) | HISTORY[1]) << 3);
      HISTORY[base] = RAM[cur];
      HISTORY[base + 1] = RAM[cur + 1];
      changed_times = 0;
      HISTORY[base + 2] = v >> 8;
      HISTORY[base + 3] = v & 0xFF;
      HISTORY[base + 4] = cur >> 4;
      HISTORY[base + 5] = ((cur & 0x0F) << 4) | (chip8.startPoint >> 8);
      HISTORY[base + 6] = chip8.startPoint & 0xFF;
      HISTORY[base + 7] = GET_BP_LEVEL(chip8.PC) << 4;

      RAM[cur] = v >> 8;
      RAM[cur + 1] = v & 0xFF;
      changed_times = 0;
      noecho();
      curs_set(0);
      timeout(0);
    }
    if (ch == 'U') {
      uint16_t count = (HISTORY[0] << 8) | HISTORY[1];

      uint32_t base = 2 + (count << 3);

      uint16_t old_cur = (HISTORY[base + 4] << 4) | (HISTORY[base + 5] >> 4);
      count--;
      RAM[old_cur] = HISTORY[base];
      RAM[old_cur + 1] = HISTORY[base + 1];

      uint32_t prev_base = 2 + (count << 3);
      chip8.startPoint =
          ((HISTORY[prev_base + 5] & 0x0F) << 8) | HISTORY[prev_base + 6];

      HISTORY[0] = count >> 8;
      HISTORY[1] = count & 0xFF;

      changed_times++;

      mvprintw(33, 0, "UNDO DONE at %04X", old_cur);
    }
    if (ch == 'R' && changed_times > 0) {
      uint16_t count = (HISTORY[0] << 8) | HISTORY[1];
      count++;
      uint32_t base = 2 + (count << 3);

      uint16_t redo_cur = ((uint16_t)HISTORY[base + 4] << 4) |
                          ((uint16_t)HISTORY[base + 5] >> 4);

      RAM[redo_cur] = HISTORY[base + 2];
      RAM[redo_cur + 1] = HISTORY[base + 3];

      chip8.startPoint = (((uint16_t)HISTORY[base + 5] & 0x0F) << 8) |
                         (uint16_t)HISTORY[base + 6];

      HISTORY[0] = count >> 8;
      HISTORY[1] = count & 0xFF;
      changed_times--;
    }

    if (ch == 'E') {
      timeout(-1);
      echo();
      curs_set(1);

      mvprintw(33, 0, "EDIT: V");
      char b[2];
      getnstr(b, 1);
      uint8_t reg = (uint8_t)strtol(b, NULL, 16);
      mvprintw(33, 8, ", SET TO: ");
      char v[3];
      getnstr(v, 2);
      uint8_t val = (uint8_t)strtol(v, NULL, 16);
      mvprintw(33, 20, "%s.", v);
      noecho();
      chip8.regs[reg] = val;
      curs_set(0);
      timeout(0);
    }
    if (run) {
      accumulator += dt;
      while (accumulator >= 16666) {
        if (chip8.del_time > 0)
          chip8.del_time--;
        if (chip8.soun_time > 0)
          chip8.soun_time--;
        accumulator -= 16666;
      }
      step();
      if (GET_BP_LEVEL(chip8.PC) > GET_STOP_BP_LEVEL)
        run = 0;
    }

    draw_dis(wd, cur);
    draw_reg(wr);
    draw_scr(ws);
  }

  delwin(wd);
  delwin(wr);
  delwin(ws);
  endwin();

  munmap((void *)disasm_table, 65536 * 8);
  msync(RAM, 4096, MS_SYNC);
  munmap(RAM, 4096);
  msync(HISTORY, 2 * 1024 * 1024, MS_SYNC);
  munmap(HISTORY, 2 * 1024 * 1024);

  close(fd_t);
  close(fd_m);
  close(fd_h);
  endwin();
  return 0;
}
//!END
