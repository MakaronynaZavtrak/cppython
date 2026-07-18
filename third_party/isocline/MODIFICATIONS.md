# Модификации isocline

Источник: https://github.com/daanx/isocline (MIT License, (c) 2021 Daan Leijen)
Версия: <v.1.1.0/497>

## Внесённые изменения

1. `include/isocline.h` — добавлен тип `ic_is_complete_fun_t` и функция
   `ic_set_is_complete_fun()`.
2. `src/env.h` — в структуру `ic_env_s` добавлено поле `is_complete_fun`.
3. `src/isocline.c` — реализация `ic_set_is_complete_fun()`.
4. `src/editline.c`:
    - обработка `KEY_ENTER` — вызов колбэка `is_complete_fun` для решения
      "завершить ввод или вставить новую строку" (Python-семантика блоков);
    - `KEY_TAB` вставляет 4 пробела вместо вызова автодополнения;
    - `edit_backspace()` — умное удаление отступа до границы, кратной 4;
    - добавлена `editor_python_auto_indent()`, вызывается вместо
      `editor_auto_indent()` при вставке `\n`.