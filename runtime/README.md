# runtime

Контракт рантайма (D6) и его бэкенды.

## Контракт

`include/abl_runtime.h` — то, чем пользуются приложение и драйверы **вместо**
RTOS/vendor-примитивов (R8, R9):

- `abl_uptime_ms()`, `abl_sleep_ms()`, `abl_sleep_us()` — время;
- `abl_task_create()`, `abl_runtime_run()` — задачи.

Остальное добавляется по мере появления потребителей: mutex/semaphore/queue — с
первым конкурентным потребителем, software timers — с первым периодическим,
critical sections — с первым ISR-драйвером (HAL-контракт `abl_critical`, Шаг 8).

## Бэкенды

| | `bare` (суперлуп) | `freertos` (план, Шаг 9) |
|---|---|---|
| sleep | busy wait (`abl_delay_*`) | `vTaskDelay` |
| `task_create` | запись в таблицу кооперативного цикла | `xTaskCreate` |
| `runtime_run` | round-robin по задачам | `vTaskStartScheduler` |
| uptime | HAL tick (`abl_time_uptime_ms`) | `xTaskGetTickCount` |

Выбор бэкенда — в app-конфиге проекта: `product.runtime: bare` (см. §14 конституции
и `src/freertos/README.md` для плана).

## Важно

- **Драйверы неблокирующие** (R5): блокирующий `abl_sleep_ms()` внутри задачи
  останавливает кооперативный цикл bare-бэкенда.
- **На STM32+FreeRTOS нельзя `HAL_Delay`** (SysTick принадлежит планировщику) —
  поэтому весь sleep идёт через контракт, а `abl_delay_*` (HAL) остаётся
  примитивом точного busy-wait для bit-banging (R9).
