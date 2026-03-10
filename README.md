(😊 -stat0) stas@stas-W9x0LU:~/project_version2_Simple/Disasm>cat README.md
# Chip8 Debugger (TUI)

### RU / [EN](#en)

Это интерактивный отладчик для виртуальной машины Chip8, написанный на C с использованием ncurses.

**Возможности:**
* Три режима: выполнение (running), пошаговый (step-by-step) и редактирование.
* Три окна: листинг программы, значения регистров и экран самой VM.
* Система уровней точек останова (Breakpoints).

**Управление:**
- `ВВЕРХ/ВНИЗ`: Навигация по коду.
- `A` / `D`: Изменить уровень точки останова (BP level) на строке.
- `Z` / `X`: Изменить глобальный порог остановки.
- `G`: Запуск / Пауза.
- `Space` (Пробел): Один шаг.
- `S`: Перейти к точке старта.
- `P`: Установить текущую строку как точку старта.
- `C`: Редактировать байт-код по курсору.
- `E`: Изменить значение регистра.
- `U` / `R`: Undo / Redo (Отмена/Повтор действий).
- `Q`: Выход.
- Клавиши Chip8 передаются напрямую в виртуальную машину.

**Сборка и запуск:**
Зависимости: `gcc`, `make`, `libncurses-dev`.
```bash
make chip8_emu
./chip8_emu /путь/к/вашему/рому
```
EN
An interactive TUI debugger for the Chip8 virtual machine, built with C and ncurses.
Key Bindings:
UP/DOWN: Navigate through the code.
A / D: Adjust breakpoint level on the current line.
Z / X: Adjust the global "stop threshold".
G: Run / Pause execution.
Space: Execute a single step.
S: Jump to the start point.
P: Set current line as the new start point.
C: Edit hex code at cursor.
E: Edit register values.
U / R: Undo / Redo support.
Q: Quit.
```bash
make chip8_emu
./chip8_emu /путь/к/вашему/рому
```

Licensed under GPLv3
