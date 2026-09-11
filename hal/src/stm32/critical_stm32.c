/**
 * @brief STM32 critical sections (L1): PRIMASK save/restore.
 */

#include "abl_critical.h"
#include "soc_hal.h"

abl_critical_state_t abl_critical_enter(void)
{
    const abl_critical_state_t state = (abl_critical_state_t)__get_PRIMASK();
    __disable_irq();
    return state;
}

void abl_critical_exit(abl_critical_state_t state)
{
    __set_PRIMASK(state);
}
