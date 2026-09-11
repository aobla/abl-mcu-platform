#include "abl_time.h"

/* TODO (Step 8): implement a millisecond tick (Timer0 overflow ISR).
 * The AVR port is not buildable yet, so this is an honest stub. */
#warning "abl_time: AVR tick source is not implemented yet (Step 8)"

abl_status_t abl_time_init(void)
{
    return ABL_STATUS_ERROR;
}

uint32_t abl_time_uptime_ms(void)
{
    return 0;
}
