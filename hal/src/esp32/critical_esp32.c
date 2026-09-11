/**
 * @brief ESP32 critical sections (L1).
 *
 * TODO (Step 9): for task context ESP-IDF expects portENTER_CRITICAL() with a
 * spinlock; the interrupt-mask API used here is ISR-safe and sufficient for
 * the current skeleton.
 */

#include "abl_critical.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

abl_critical_state_t abl_critical_enter(void)
{
    return (abl_critical_state_t)portSET_INTERRUPT_MASK_FROM_ISR();
}

void abl_critical_exit(abl_critical_state_t state)
{
    portCLEAR_INTERRUPT_MASK_FROM_ISR(state);
}
