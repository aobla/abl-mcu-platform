#include "abl_time.h"

#include <esp_timer.h>

abl_status_t abl_time_init(void)
{
    /* esp_timer is initialized by the ESP-IDF startup code. */
    return ABL_STATUS_OK;
}

uint32_t abl_time_uptime_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}
