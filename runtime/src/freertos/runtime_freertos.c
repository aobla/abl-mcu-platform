/**
 * @brief FreeRTOS runtime backend (D6).
 *
 * Maps the runtime contract onto FreeRTOS. Used by the ESP-IDF port (Step 9);
 * STM32 projects can use it once FreeRTOS is added to the manifest.
 *
 * IMPORTANT — scheduler ownership:
 *   On ESP-IDF the scheduler is ALREADY RUNNING when app_main() is called (the
 *   startup code creates the main task and starts the scheduler). Therefore this
 *   backend must NOT call vTaskStartScheduler(): a second call re-creates the
 *   idle/timer tasks and corrupts the scheduler state. On ESP-IDF
 *   abl_runtime_run() serves the current task instead (see ABL_RUNTIME_OWNS_SCHEDULER).
 *
 *   A plain FreeRTOS integration (STM32 + FreeRTOS, where nothing starts the
 *   scheduler for us) does need vTaskStartScheduler(). That case sets
 *   ABL_RUNTIME_OWNS_SCHEDULER=1 at build time.
 *
 * Note: abl_task_create() has no stack/priority parameters yet — the backend
 * uses defaults. Extending the contract is a separate decision.
 */

#include "abl_runtime.h"
#include "abl_delay.h"
#include "abl_time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ABL_FREERTOS_TASK_STACK_BYTES 4096U
#define ABL_FREERTOS_TASK_PRIORITY    5U

#if defined(ESP_PLATFORM) && !defined(ABL_RUNTIME_OWNS_SCHEDULER)
#define ABL_RUNTIME_OWNS_SCHEDULER 0
#elif !defined(ABL_RUNTIME_OWNS_SCHEDULER)
#define ABL_RUNTIME_OWNS_SCHEDULER 1
#endif

/* ─── Time ──────────────────────────────────────────────────────────────── */

uint32_t abl_uptime_ms(void)
{
    /* The HAL tick gives better resolution than the FreeRTOS tick on ESP32. */
    return abl_time_uptime_ms();
}

void abl_sleep_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void abl_sleep_us(uint32_t us)
{
    /* Shorter than a tick: yielding is impossible, so busy-wait on purpose. */
    abl_delay_us(us);
}

/* ─── Tasks ─────────────────────────────────────────────────────────────── */

abl_status_t abl_task_create(abl_task_fn_t fn, const char *name, void *arg)
{
    if (fn == 0) {
        return ABL_STATUS_ERROR;
    }

    if (xTaskCreate(fn, (name != 0) ? name : "abl", ABL_FREERTOS_TASK_STACK_BYTES, arg,
                    ABL_FREERTOS_TASK_PRIORITY, 0) != pdPASS) {
        return ABL_STATUS_ERROR;
    }

    return ABL_STATUS_OK;
}

void abl_runtime_run(void)
{
#if ABL_RUNTIME_OWNS_SCHEDULER
    vTaskStartScheduler();   /* never returns */
#endif

    /* Either the scheduler was already running (ESP-IDF) or it just took over:
     * serve this task's loop so a task registered on the current context keeps
     * running. Applications that only use abl_task_create() just yield here. */
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
