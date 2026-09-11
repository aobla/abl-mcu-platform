/**
 * @brief ESP32 delay primitives (L1).
 *
 * Busy-wait only (see the contract): sleeping that yields the CPU belongs to
 * the runtime backend (vTaskDelay), not here.
 */

#include "abl_delay.h"

#include "esp_rom_sys.h"

void abl_delay_us(uint32_t us)
{
    esp_rom_delay_us(us);
}

void abl_delay_ms(uint32_t ms)
{
    while (ms-- > 0U) {
        esp_rom_delay_us(1000U);
    }
}
