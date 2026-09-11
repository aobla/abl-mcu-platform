/**
 * @brief AVR critical sections (L1): SREG save/restore.
 */

#include "abl_critical.h"

#include <avr/interrupt.h>
#include <avr/io.h>

abl_critical_state_t abl_critical_enter(void)
{
    const uint8_t sreg = SREG;
    cli();
    return (abl_critical_state_t)sreg;
}

void abl_critical_exit(abl_critical_state_t state)
{
    SREG = (uint8_t)state;
}
