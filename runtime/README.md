# runtime

Контракт рантайма (D6) и его бэкенды.

- `include/abl/abl_runtime.h` — контракт: sleep, task, mutex, semaphore, queue, timer, critical.
- `src/bare/` — кооперативный суперлуп (AVR, лёгкие STM32).
- `src/freertos/` — FreeRTOS (основной рантайм STM32/ESP32).

Выбор бэкенда — через конфиг проекта (`runtime: bare | freertos`).
