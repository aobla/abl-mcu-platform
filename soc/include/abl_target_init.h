#ifndef ABL_TARGET_INIT_H
#define ABL_TARGET_INIT_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Инициализация целевой платформы
 * 
 * Выполняет основную инициализацию микроконтроллера:
 * - Настройка тактирования
 * - Настройка системного таймера
 * - Инициализация периферии
 */
void abl_target_init(void);

/**
 * @brief Инициализация GPIO пинов
 * 
 * Инициализирует GPIO пины в соответствии с конфигурацией
 */
void abl_target_gpio_init(void);

/**
 * @brief Задержка в миллисекундах
 *
 * @param ms Количество миллисекунд
 */
void abl_target_delay_ms(uint32_t ms);

/**
 * @brief Сгенерированная функция инициализации GPIO (из YAML)
 */
void generated_gpio_init(void);

#endif // ABL_TARGET_INIT_H