/**
 * @brief Native (host) delay primitives (L1).
 *
 * Unlike the MCU ports (busy-wait), the host port really sleeps: the native
 * target is a simulator for logic, not a real-time target, and a real sleep
 * keeps CPU usage sane and makes timing visible.
 */

#include "abl_delay.h"

#include <time.h>

static void sleep_ns(uint64_t ns)
{
    struct timespec ts;
    ts.tv_sec  = (time_t)(ns / 1000000000ULL);
    ts.tv_nsec = (long)(ns % 1000000000ULL);

    while (nanosleep(&ts, &ts) == -1) {
        /* interrupted by a signal: ts holds the remaining time */
    }
}

void abl_delay_us(uint32_t us)
{
    sleep_ns((uint64_t)us * 1000ULL);
}

void abl_delay_ms(uint32_t ms)
{
    sleep_ns((uint64_t)ms * 1000000ULL);
}
