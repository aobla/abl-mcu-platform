#ifndef SOC_CLOCK_H
#define SOC_CLOCK_H

/**
 * @brief Инициализация тактирования конкретного SoC.
 *
 * Реализуется в SoC-дефиниции: soc/<family>/<variant>/clock.c
 * Вызывается общим bring-up (soc/stm32/stm32_target_init.c) после HAL_Init().
 */
void soc_clock_init(void);

#endif /* SOC_CLOCK_H */
