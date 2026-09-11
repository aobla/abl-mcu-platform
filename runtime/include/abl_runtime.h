#ifndef ABL_RUNTIME_H
#define ABL_RUNTIME_H

#include "abl_common.h"

/**
 * @brief Runtime contract (D6).
 *
 * Application code and drivers use the runtime for sleeping and scheduling, so
 * the same sources work on a bare-metal superloop (AVR, small STM32 projects)
 * and on FreeRTOS (the main runtime for STM32/ESP32).
 *
 * The backend is selected by the application:
 *   product.runtime: bare | freertos      (config/<product>.yml)
 *
 * Rules:
 *   - never call vendor/RTOS primitives directly from the app or drivers (R8, R9);
 *   - drivers stay non-blocking (R5): a blocking sleep inside a task stalls the
 *     cooperative loop on the bare backend.
 */

/* ─── Time ──────────────────────────────────────────────────────────────── */

/** Monotonic uptime in milliseconds (tick source comes from the HAL). */
uint32_t abl_uptime_ms(void);

/**
 * @brief Sleep for the given number of milliseconds.
 *
 * bare:     blocks (busy wait) — the cooperative loop is stalled.
 * freertos: yields the CPU (vTaskDelay).
 */
void abl_sleep_ms(uint32_t ms);

/** Same as abl_sleep_ms() but for microsecond-scale delays. */
void abl_sleep_us(uint32_t us);

/* ─── Tasks ─────────────────────────────────────────────────────────────── */

/** Task entry point. */
typedef void (*abl_task_fn_t)(void *arg);

/**
 * @brief Create a task.
 *
 * bare:     registers the function in the cooperative round-robin table.
 * freertos: creates a real task (xTaskCreate).
 *
 * @param fn    task entry point (must not be NULL)
 * @param name  task name for logs/debugging (may be NULL)
 * @param arg   opaque argument passed to the entry point
 *
 * @return ABL_STATUS_OK on success, ABL_STATUS_ERROR otherwise.
 */
abl_status_t abl_task_create(abl_task_fn_t fn, const char *name, void *arg);

/**
 * @brief Start executing tasks. Never returns.
 *
 * bare:     cooperative round-robin loop over the registered tasks.
 * freertos: starts the scheduler (vTaskStartScheduler).
 */
void abl_runtime_run(void);

/*
 * Planned, added together with their first consumer (see runtime/README.md):
 *   - mutex / semaphore / queue — with the first concurrent driver or task;
 *   - software timers          — with the first periodic consumer;
 *   - critical sections        — with the first ISR-shared driver (HAL contract
 *                                abl_critical, Step 8).
 */

#endif /* ABL_RUNTIME_H */
