#include "hal/delay.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

void hal_delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void hal_delay_us(uint32_t us) {
    esp_timer_usleep(us);
}
