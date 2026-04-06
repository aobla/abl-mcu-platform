#include "hal/delay.h"
#include <stm32f4xx_hal.h>

void hal_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

void hal_delay_us(uint32_t us) {
    // Используем микроконтроллерный таймер для точной задержки
    // Для STM32F4 с HCLK = 168MHz, 1 цикл ≈ 6ns
    volatile uint32_t cycles = us * (SystemCoreClock / 1000000U);
    while (cycles--) {
        __NOP();
    }
}
