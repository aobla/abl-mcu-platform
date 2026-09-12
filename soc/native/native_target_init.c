/**
 * @brief Native (host) bring-up (L1).
 *
 * There is no clock tree and no real tick on the host: bring-up only prepares
 * the tick source and initialises the generated GPIO table. The main()
 * trampoline lives in native_main.c (D7).
 */

#include "abl_target_init.h"
#include "abl_time.h"

void abl_target_init(void)
{
    (void)abl_time_init();
    abl_target_gpio_init();
}

void abl_target_gpio_init(void)
{
    generated_gpio_init();
}
