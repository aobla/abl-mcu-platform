#ifndef ABL_DELAY_H
#define ABL_DELAY_H

#include <stdint.h>

/**
 * @brief Задержка в миллисекундах
 * 
 * @param ms Количество миллисекунд
 */
void abl_delay_ms(uint32_t ms);

/**
 * @brief Задержка в микросекундах
 * 
 * @param us Количество микросекунд
 */
void abl_delay_us(uint32_t us);

#endif // ABL_DELAY_H
