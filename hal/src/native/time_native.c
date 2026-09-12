/**
 * @brief Native (host) tick (L1) — monotonic clock.
 */

#include "abl_time.h"

#include <time.h>

abl_status_t abl_time_init(void)
{
    return ABL_STATUS_OK;   /* clock_gettime() needs no setup */
}

uint32_t abl_time_uptime_ms(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL));
}
