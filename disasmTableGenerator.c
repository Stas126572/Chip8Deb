//!DisasmTableGenerator
/* 
 * Copyright (C) 2024 Manichev Stanislav (https://github.com/Stas126572)
 * Licensed under GNU GPL v3.0.
 * 
 * ДОПОЛНИТЕЛЬНОЕ УСЛОВИЕ (Section 7): 
 * Любые изменения в этом коде должны сопровождаться сохранением 
 * оригинального уведомления об авторстве и ссылки на репозиторий.
 * Удаление визуальных уведомлений об авторстве в интерфейсе программы запрещено.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h> 

static const char H[] = "0123456789ABCDEF";

int main() {
  FILE *f = fopen("disasm.bin", "wb");
  if (!f)
    return 1;

  for (uint32_t op = 0; op < 65536; op++) {
    char e[8] = "        ";

    uint8_t n1 = (op >> 12) & 0xF;
    uint8_t n2 = (op >> 8) & 0xF;
    uint8_t n3 = (op >> 4) & 0xF;
    uint8_t n4 = op & 0xF;
    uint8_t kk = op & 0xFF;
    uint16_t nnn = op & 0xFFF;

    switch (n1) {
    case 0x0:
      if (op == 0x00E0) {
        memcpy(e, "CLS", 3);
        goto write;
      }
      if (op == 0x00EE) {
        memcpy(e, "RET", 3);
        goto write;
      }
      memcpy(e, "SYS", 3);
      goto addr;
    case 0x1:
      memcpy(e, "JP ", 3);
      goto addr;
    case 0x2:
      memcpy(e, "CAL", 3);
      goto addr;
    case 0x3:
      memcpy(e, "SE ", 3);
      goto v_kk;
    case 0x4:
      memcpy(e, "SNE", 3);
      goto v_kk;
    case 0x5:
      memcpy(e, "SE ", 3);
      goto v_v;
    case 0x6:
      memcpy(e, "LD ", 3);
      goto v_kk;
    case 0x7:
      memcpy(e, "ADD", 3);
      goto v_kk;
    case 0x8:
      switch (n4) {
      case 0x0:
        memcpy(e, "LD ", 3);
        break;
      case 0x1:
        memcpy(e, "OR ", 3);
        break;
      case 0x2:
        memcpy(e, "AND", 3);
        break;
      case 0x3:
        memcpy(e, "XOR", 3);
        break;
      case 0x4:
        memcpy(e, "ADD", 3);
        break;
      case 0x5:
        memcpy(e, "SUB", 3);
        break;
      case 0x6:
        memcpy(e, "SHR", 3);
        break;
      case 0x7:
        memcpy(e, "SBN", 3);
        break;
      case 0xE:
        memcpy(e, "SHL", 3);
        break;
      default:
        memcpy(e, "???", 3);
        break;
      }
      goto v_v;
    case 0x9:
      memcpy(e, "SNE", 3);
      goto v_v;
    case 0xA:
      memcpy(e, "LD ", 3);
      goto i_nnn;
    case 0xB:
      memcpy(e, "JP ", 3);
      goto v0_nnn;
    case 0xC:
      memcpy(e, "RND", 3);
      goto v_kk;
    case 0xD:
      memcpy(e, "DRW", 3);
      goto v_v_n;
    case 0xE:
      if (kk == 0x9E)
        memcpy(e, "SKP", 3);
      else
        memcpy(e, "SKN", 3);
      goto vx;
    case 0xF:
      switch (kk) {
      case 0x07:
      case 0x0A:
      case 0x15:
      case 0x18:
      case 0x29:
      case 0x33:
      case 0x55:
      case 0x65:
        memcpy(e, "LD ", 3);
        break;
      case 0x1E:
        memcpy(e, "ADD", 3);
        break;
      default:
        memcpy(e, "???", 3);
        break;
      }
      goto f_group;
    default:
      memcpy(e, "DB ", 3);
      goto db;
    }

  addr:
    e[3] = H[(nnn >> 8) & 0xF];
    e[4] = H[(nnn >> 4) & 0xF];
    e[5] = H[nnn & 0xF];
    goto write;
  v_kk:
    e[3] = H[n2];
    e[5] = H[(kk >> 4) & 0xF];
    e[6] = H[kk & 0xF];
    goto write;
  v_v:
    e[3] = H[n2];
    e[5] = H[n3];
    goto write;
  v_v_n:
    e[3] = H[n2];
    e[5] = H[n3];
    e[7] = H[n4];
    goto write;
  vx:
    e[3] = H[n2];
    goto write;
  i_nnn:
    e[3] = 'I';
    e[5] = H[(nnn >> 8) & 0xF];
    e[6] = H[(nnn >> 4) & 0xF];
    e[7] = H[nnn & 0xF];
    goto write;
  v0_nnn:
    e[3] = '0';
    e[5] = H[(nnn >> 8) & 0xF];
    e[6] = H[(nnn >> 4) & 0xF];
    e[7] = H[nnn & 0xF];
    goto write;
  f_group:
    if (kk == 0x1E) {
      e[3] = 'I';
      e[5] = H[n2];
    } else if (kk == 0x55 || kk == 0x65) {
      e[3] = 'I';
      e[5] = H[n2];
    } else {
      e[3] = H[n2];
      e[5] = H[kk >> 4];
      e[6] = H[kk & 0xF];
    }
    goto write;
  db:
    e[3] = H[n1];
    e[4] = H[n2];
    e[5] = H[n3];
    e[6] = H[n4];

  write:
    fwrite(e, 1, 8, f);
  }
  fclose(f);
  return 0;
}
//END
