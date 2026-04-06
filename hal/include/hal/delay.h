#ifndef HAL_DELAY_H
#define HAL_DELAY_H

#include <stdint.h>

/**
 * @brief Задержка в миллисекундах
 * 
 * @param ms Количество миллисекунд
 */
void hal_delay_ms(uint32_t ms);

/**
 * @brief Задержка в микросекундах
 * 
 * @param us Количество микросекунд
 */
void hal_delay_us(uint32_t us);

#endif // HAL_DELAY_H
