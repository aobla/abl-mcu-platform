#include "abl_delay.h"
#include <util/delay.h>

void abl_delay_ms(uint32_t ms) {
    // _delay_ms требует константу времени компиляции, поэтому используем цикл
    for (uint32_t i = 0; i < ms; i++) {
        _delay_ms(1);
    }
}

void abl_delay_us(uint32_t us) {
    // _delay_us также требует константу
    while (us--) {
        _delay_us(1);
    }
}
