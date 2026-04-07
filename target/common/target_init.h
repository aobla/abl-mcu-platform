#ifndef TARGET_INIT_H
#define TARGET_INIT_H

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
void target_init(void);

/**
 * @brief Инициализация GPIO пинов
 * 
 * Инициализирует GPIO пины в соответствии с конфигурацией
 */
void target_gpio_init(void);

/**
 * @brief Задержка в миллисекундах
 *
 * @param ms Количество миллисекунд
 */
void target_delay_ms(uint32_t ms);

/**
 * @brief Сгенерированная функция инициализации GPIO (из YAML)
 */
void generated_gpio_init(void);

#endif // TARGET_INIT_H