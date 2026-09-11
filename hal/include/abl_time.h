#ifndef ABL_TIME_H
#define ABL_TIME_H

#include "abl_common.h"

/**
 * @brief Initialize the platform tick source (if required).
 *
 * Platforms whose tick is already running after bring-up (for example SysTick
 * on STM32, started by HAL_Init() and advanced by the SoC SysTick handler)
 * return ABL_STATUS_OK without doing anything.
 */
abl_status_t abl_time_init(void);

/**
 * @brief Monotonic uptime in milliseconds since boot.
 *
 * HAL-level primitive: the runtime contract (abl_runtime.h) exposes it to
 * applications and drivers as abl_uptime_ms().
 */
uint32_t abl_time_uptime_ms(void);

#endif /* ABL_TIME_H */
