# templates

Шаблоны кодогенерации — **единственный источник** генерируемого кода
(ARCHITECTURE.md §9). Инлайн-шаблоны в скриптах запрещены: скрипт даёт данные и
валидацию, шаблон — текст.

| Шаблон | Генератор | Результат |
|---|---|---|
| `hardware_config.h.jinja` | `scripts/gen_config.py` | `generated/hardware_config.h` (частоты/память платы + фичи/параметры продукта) |
| `hardware_pins.h.jinja` | `scripts/gen_pinmux.py` | `generated/hardware_pins.h` (макросы пинов + хэндлы) |
| `generated_gpio_init.c.jinja` | `scripts/gen_pinmux.py` | `generated/generated_gpio_init.c` (таблица + `generated_gpio_init()`) |

## Как это работает

- Точка входа для обеих build-систем — `scripts/generate.py <board.yml> <app.yml>`
  (обычный CMake и обёртка ESP-IDF вызывают именно её).
- Общие хелперы (загрузка шаблонов/YAML, вывод в `<build>/generated`, макросы
  имён) — `scripts/abl_codegen.py`.
- Шаблоны ищутся относительно самого скрипта, а не рабочего каталога: сборка
  запускает генераторы из своего build-каталога.

## Как добавить генератор

1. Положить шаблон сюда: `templates/<что>.jinja`.
2. Добавить в `scripts/` функцию `generate(board_path, app_path)`, которая
   использует `abl_codegen.load_template()`/`emit()` и валидирует вход.
3. Вызвать её из `scripts/generate.py` и добавить выходной файл в
   `OUTPUT`/`DEPENDS` custom-команд (обычный CMake и `target/esp32/main/CMakeLists.txt`).
