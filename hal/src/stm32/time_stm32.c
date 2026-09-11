#include "abl_time.h"
#include "soc_hal.h"

abl_status_t abl_time_init(void)
{
    /* SysTick is started by HAL_Init() and uwTick is advanced by
     * SysTick_Handler() in the SoC bring-up, so nothing to do here. */
    return ABL_STATUS_OK;
}

uint32_t abl_time_uptime_ms(void)
{
    return HAL_GetTick();
}
