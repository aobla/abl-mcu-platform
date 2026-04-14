#include "abl_delay.h"

/* HAL header — selected by compile-time define from toolchain */
#if defined(PLATFORM_STM32F1) || defined(PLATFORM_STM32F103)
#include <stm32f1xx_hal.h>
#elif defined(PLATFORM_STM32H7) || defined(PLATFORM_STM32H743)
#include <stm32h7xx_hal.h>
#else
#include <stm32f4xx_hal.h>
#endif

void abl_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

void abl_delay_us(uint32_t us) {
    volatile uint32_t cycles = us * (SystemCoreClock / 1000000U);
    while (cycles--) {
        __NOP();
    }
}
