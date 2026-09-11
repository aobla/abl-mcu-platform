/**
 * @brief AVR millisecond tick (L1) on Timer0.
 *
 * Timer0 runs in CTC mode with a /64 prescaler; the compare match ISR
 * increments the uptime counter.
 *   OCR0A = F_CPU / (prescaler * 1000) - 1
 * ATmega328P @16 MHz: 16000000 / 64000 - 1 = 249 (fits the 8-bit OCR0A).
 *
 * Note: Timer0 is claimed by the platform, an application must not reprogram it.
 */

#include "abl_time.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#if !defined(F_CPU)
#error "F_CPU must be defined to configure the AVR tick"
#endif

#define ABL_AVR_TICK_COMPARE ((uint8_t)((F_CPU / 64000UL) - 1UL))

static volatile uint32_t s_uptime_ms;

ISR(TIMER0_COMPA_vect)
{
    s_uptime_ms++;
}

abl_status_t abl_time_init(void)
{
    TCCR0A = (uint8_t)(1U << WGM01);                  /* CTC mode */
    TCCR0B = (uint8_t)((1U << CS01) | (1U << CS00));  /* prescaler /64 */
    OCR0A  = ABL_AVR_TICK_COMPARE;
    TIMSK0 = (uint8_t)(1U << OCIE0A);
    return ABL_STATUS_OK;
}

uint32_t abl_time_uptime_ms(void)
{
    const uint8_t sreg = SREG;
    cli();
    const uint32_t value = s_uptime_ms;
    SREG = sreg;
    return value;
}
