# runtime: freertos backend

Реализован на Шаге 9, используется портом ESP32. Отображение контракта
`abl_runtime.h` на FreeRTOS:

| Контракт | FreeRTOS |
|---|---|
| `abl_sleep_ms` | `vTaskDelay(pdMS_TO_TICKS(ms))` |
| `abl_sleep_us` | busy-wait через HAL (`abl_delay_us`) — короче тика, уступать нечему |
| `abl_uptime_ms` | HAL-тик (`esp_timer` на ESP32 — точнее тика планировщика) |
| `abl_task_create` | `xTaskCreate` (стек/приоритет — пока дефолты) |
| `abl_runtime_run` | ESP-IDF: **планировщик не запускает** (он уже работает), только обслуживает задачу; голый FreeRTOS: `vTaskStartScheduler()` |
| mutex / semaphore / queue | `xSemaphore*` / `xQueue*` (+ `_fromISR`) — по мере потребителей |
| critical | `taskENTER_CRITICAL` / `portENTER_CRITICAL` |

## ⚠️ Не запускайте планировщик дважды (R13)

В **ESP-IDF** планировщик уже работает, когда вызывается `app_main()`: код запуска
создаёт main-задачу и стартует планировщик. Вызов `vTaskStartScheduler()` из
`app_main()` повторно создаёт idle/timer-задачи (`prvCreateIdleTasks()`) и портит
состояние планировщика.

**Симптом на ESP32-C3:** приложение запускается, но светодиод «еле мигает с
непонятной частотой» — задержки отрабатывают неверно, хотя код приложения
формально корректен. Эталонный пример IDF работает именно потому, что крутится
в `while(1)` внутри `app_main` и планировщик не трогает.

Решение — на этапе сборки:

```c
#if defined(ESP_PLATFORM) && !defined(ABL_RUNTIME_OWNS_SCHEDULER)
#define ABL_RUNTIME_OWNS_SCHEDULER 0   /* ESP-IDF владеет планировщиком */
#else
#define ABL_RUNTIME_OWNS_SCHEDULER 1   /* голый FreeRTOS: запускаем сами */
#endif
```

Для STM32+FreeRTOS значение по умолчанию — `1`, поведение прежнее.

## Почему нельзя просто вызвать `HAL_Delay` на STM32+FreeRTOS

SysTick принадлежит планировщику, `uwTick` не растёт, и задержка превращается в
вечное ожидание (R9). Поэтому **весь** sleep идёт через контракт рантайма, а
`abl_delay_*` (HAL) остаётся примитивом точного busy-wait для bit-banging.

## Известные TODO

- В контракте нет параметров стека/приоритета задачи — бэкенд использует дефолты.
  Расширение контракта (`abl_task_create_ex` или поля) — отдельное решение.
- Для STM32+FreeRTOS нужно добавить исходники FreeRTOS в `manifest.yml`; пока
  бэкенд собирается только в IDF-мире (ESP32).
