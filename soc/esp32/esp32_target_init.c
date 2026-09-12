/**
 * @brief ESP32 bring-up (L1).
 *
 * ESP-IDF initialises clocks, the tick and the scheduler before app_main(), so
 * bring-up only initialises the generated GPIO table. The app_main() trampoline
 * lives in the project: target/esp32/main/main.c (D7).
 */

#include "abl_target_init.h"
#include "abl_time.h"

void abl_target_init(void)
{
    (void)abl_time_init();   /* esp_timer is already running under ESP-IDF */
    abl_target_gpio_init();
}

void abl_target_gpio_init(void)
{
    generated_gpio_init();
}
