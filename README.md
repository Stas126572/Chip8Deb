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
### RU / [EN](#en)

This is an interactive debugger for the Chip8 virtual machine, written in C using ncurses.

**Features:**
* Three modes: running, step-by-step, and editing.
* Three windows: program listing, register values, and the VM screen.
* Breakpoint level system.

**Controls:**
- UP/DOWN: Navigate through code.
- A / D: Change the breakpoint level on a line.
- Z / X: Change the global breakpoint threshold.
- G: Run / Pause.
- Space: Step one step.
- S: Go to the starting point.
- `P`: Set the current line as the starting point.
- `C`: Edit bytecode at the cursor.
- `E`: Change a register value.
- `U` / `R`: Undo / Redo.
- `Q`: Exit.
- Chip8 keys are transmitted directly to the virtual machine.
```bash
make chip8_emu
./chip8_emu /путь/к/вашему/рому
```

Licensed under GPLv3
