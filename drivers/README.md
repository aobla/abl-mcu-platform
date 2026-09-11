# drivers

Переносимые драйверы (L3): датчики, дисплеи, протоколы.

## Правила

- Драйвер зависит **только от контрактов** (`abl::hal`), а не от конкретного
  порта или vendor-HAL (R1, R4).
- Драйверы неблокирующие (конечные автоматы), если контракт не требует иного (R5).
- Публичный заголовок — `include/abl_<name>.h`; всё остальное приватно.

## Как добавить драйвер

1. Создать каталог:

```
drivers/<name>/
├── CMakeLists.txt
├── include/abl_<name>.h      # публичный контракт драйвера
└── src/<name>.c
```

2. `CMakeLists.txt` — один вызов `abl_component()`:

```cmake
abl_component(<name>
    SOURCES  src/<name>.c
    INCLUDES include
    DEPS     abl::hal        # только контракты (R4)
)
```

3. Подключить в проекте (в его `CMakeLists.txt`):

```cmake
target_link_libraries(${_OUTPUT_NAME} PRIVATE abl::<name>)
```

Больше править нечего: `drivers/CMakeLists.txt` находит новые каталоги сам
(`CONFIGURE_DEPENDS`), чужие CMake-файлы не затрагиваются (D4).

## Статус

Каталог пока пуст: первый настоящий драйвер появится после контрактов шин
(`abl_spi` / `abl_i2c` / `abl_uart`) — Шаг 8+.
