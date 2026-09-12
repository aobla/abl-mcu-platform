/**
 * @brief AVR bring-up (L1) for ATmega328P-class devices.
 *
 * The clock source is selected by the fuse bits, so bring-up only initialises
 * the generated GPIO table and provides the platform trampoline (D7).
 */

#include "abl_app.h"
#include "abl_target_init.h"
#include "abl_time.h"

#include <avr/io.h>

void abl_target_init(void)
{
    /* Clocking comes from the fuses: only the tick needs setting up. */
    (void)abl_time_init();
    abl_target_gpio_init();
}

void abl_target_gpio_init(void)
{
    generated_gpio_init();
}

/**
 * @brief Platform trampoline (D7): the C runtime calls main(), the application
 *        lives in the portable abl_main().
 */
int main(void)
{
    abl_main();
    return 0;
}
