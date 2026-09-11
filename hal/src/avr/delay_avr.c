/**
 * @brief AVR delay primitives (L1): busy-wait via avr-libc.
 *
 * F_CPU must be defined (the soc component exports it from the board config).
 */

#include "abl_delay.h"

#include <util/delay.h>

void abl_delay_us(uint32_t us)
{
    /* _delay_us() needs a compile-time constant: use 100 us chunks. */
    while (us >= 100U) {
        _delay_us(100U);
        us -= 100U;
    }
    while (us > 0U) {
        _delay_us(1U);
        us--;
    }
}

void abl_delay_ms(uint32_t ms)
{
    while (ms >= 10U) {
        _delay_ms(10.0);
        ms -= 10U;
    }
    while (ms > 0U) {
        _delay_ms(1.0);
        ms--;
    }
}
