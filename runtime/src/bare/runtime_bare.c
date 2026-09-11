/**
 * @brief Bare-metal runtime backend (D6): cooperative round-robin.
 *
 * There is no preemption: tasks are called one after another from a single
 * loop, so a task that blocks (for example in abl_sleep_ms) stalls the whole
 * loop. This is fine for a single task and is exactly why drivers must stay
 * non-blocking (R5).
 *
 * Sleep is implemented on top of the HAL delay primitives; uptime comes from
 * the HAL tick source. No vendor APIs are used here.
 */

#include "abl_runtime.h"
#include "abl_delay.h"
#include "abl_time.h"

#define ABL_BARE_MAX_TASKS 4

typedef struct {
    abl_task_fn_t fn;
    const char   *name;
    void         *arg;
} abl_bare_task_t;

static abl_bare_task_t s_tasks[ABL_BARE_MAX_TASKS];
static uint32_t        s_task_count;

/* ─── Time ──────────────────────────────────────────────────────────────── */

uint32_t abl_uptime_ms(void)
{
    return abl_time_uptime_ms();
}

void abl_sleep_ms(uint32_t ms)
{
    abl_delay_ms(ms);
}

void abl_sleep_us(uint32_t us)
{
    abl_delay_us(us);
}

/* ─── Tasks ─────────────────────────────────────────────────────────────── */

abl_status_t abl_task_create(abl_task_fn_t fn, const char *name, void *arg)
{
    if (fn == 0 || s_task_count >= ABL_BARE_MAX_TASKS) {
        return ABL_STATUS_ERROR;
    }

    s_tasks[s_task_count].fn   = fn;
    s_tasks[s_task_count].name = name;
    s_tasks[s_task_count].arg  = arg;
    s_task_count++;

    return ABL_STATUS_OK;
}

void abl_runtime_run(void)
{
    for (;;) {
        for (uint32_t i = 0; i < s_task_count; i++) {
            s_tasks[i].fn(s_tasks[i].arg);
        }
    }
}
