# runtime: freertos backend (planned, Step 9)

Бэкенд ещё не подключён. Контракт `abl_runtime.h` спроектирован так, чтобы
отображение на FreeRTOS было прямым:

| Контракт | FreeRTOS |
|---|---|
| `abl_sleep_ms` / `abl_sleep_us` | `vTaskDelay` / `vTaskDelayUntil`, `esp_rom_delay_us` |
| `abl_uptime_ms` | `xTaskGetTickCount` (или `esp_timer_get_time`) |
| `abl_task_create` | `xTaskCreate` (приоритет/стек добавляются в контракт) |
| `abl_runtime_run` | `vTaskStartScheduler` |
| mutex / semaphore / queue | `xSemaphore*` / `xQueue*` (+ `_fromISR` варианты) |
| critical | `taskENTER_CRITICAL` / `portENTER_CRITICAL` |

## Почему нельзя просто вызвать `HAL_Delay` на STM32+FreeRTOS

SysTick принадлежит планировщику, `uwTick` не растёт, и задержка превращается в
вечное ожидание (R9). Поэтому **весь** sleep идёт через контракт рантайма, а
`abl_delay_*` (HAL) остаётся примитивом точного busy-wait для bit-banging.

## Когда появится

Шаг 9 (вместе с обвязкой ESP-IDF) и подключением FreeRTOS для STM32 —
потребуется добавить исходники FreeRTOS в манифест (`manifest.yml`).
