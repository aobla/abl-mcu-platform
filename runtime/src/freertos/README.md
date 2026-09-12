# runtime: freertos backend

Реализован на Шаге 9, используется портом ESP32. Отображение контракта
`abl_runtime.h` на FreeRTOS:

| Контракт | FreeRTOS |
|---|---|
| `abl_sleep_ms` | `vTaskDelay(pdMS_TO_TICKS(ms))` |
| `abl_sleep_us` | busy-wait через HAL (`abl_delay_us`) — короче тика, уступать нечему |
| `abl_uptime_ms` | HAL-тик (`esp_timer` на ESP32 — точнее тика планировщика) |
| `abl_task_create` | `xTaskCreate` (стек/приоритет — пока дефолты) |
| `abl_runtime_run` | `vTaskStartScheduler()` |
| mutex / semaphore / queue | `xSemaphore*` / `xQueue*` (+ `_fromISR`) — по мере потребителей |
| critical | `taskENTER_CRITICAL` / `portENTER_CRITICAL` |

## Почему нельзя просто вызвать `HAL_Delay` на STM32+FreeRTOS

SysTick принадлежит планировщику, `uwTick` не растёт, и задержка превращается в
вечное ожидание (R9). Поэтому **весь** sleep идёт через контракт рантайма, а
`abl_delay_*` (HAL) остаётся примитивом точного busy-wait для bit-banging.

## Известные TODO

- В контракте нет параметров стека/приоритета задачи — бэкенд использует дефолты.
  Расширение контракта (`abl_task_create_ex` или поля) — отдельное решение.
- Для STM32+FreeRTOS нужно добавить исходники FreeRTOS в `manifest.yml`; пока
  бэкенд собирается только в IDF-мире (ESP32).
