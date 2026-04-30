#include "abl_delay.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void abl_delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void abl_delay_us(uint32_t us) {
    /* ESP-IDF doesn't have a direct usleep, use busy wait for small delays */
    volatile uint32_t cycles = us * 80;  /* ~80 cycles per us at 240MHz */
    while (cycles--) {
        asm volatile("nop");
    }
}
