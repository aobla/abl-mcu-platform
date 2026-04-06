#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_GPIO_MODE_INPUT,
    HAL_GPIO_MODE_OUTPUT,
    HAL_GPIO_MODE_ALT_FUNCTION,
    HAL_GPIO_MODE_ANALOG
} hal_gpio_mode_t;

typedef enum {
    HAL_GPIO_PULL_NONE,
    HAL_GPIO_PULL_UP,
    HAL_GPIO_PULL_DOWN
} hal_gpio_pull_t;

typedef enum {
    HAL_OK = 0,
    HAL_ERROR,
    HAL_BUSY,
    HAL_TIMEOUT
} hal_status_t;

typedef struct {
    void* port;         // Platform-specific (GPIO_TypeDef*, gpio_num_t, etc.)
    uint16_t pin;
} hal_gpio_pin_t;

/**
 * @brief Инициализирует GPIO пин
 * @param pin Указатель на структуру пина
 * @param mode Режим работы пина
 * @param pull Режим подтяжки
 * @return Статус выполнения
 */
hal_status_t hal_gpio_init(hal_gpio_pin_t* pin, hal_gpio_mode_t mode, hal_gpio_pull_t pull);

/**
 * @brief Записывает значение на GPIO пин
 * @param pin Указатель на структуру пина
 * @param state Логическое состояние (true - высокий уровень, false - низкий)
 * @return Статус выполнения
 */
hal_status_t hal_gpio_write(hal_gpio_pin_t* pin, bool state);

/**
 * @brief Переключает состояние GPIO пина
 * @param pin Указатель на структуру пина
 * @return Статус выполнения
 */
hal_status_t hal_gpio_toggle(hal_gpio_pin_t* pin);

/**
 * @brief Считывает состояние GPIO пина
 * @param pin Указатель на структуру пина
 * @param state Указатель на переменную для сохранения состояния
 * @return Статус выполнения
 */
hal_status_t hal_gpio_read(hal_gpio_pin_t* pin, bool* state);

#endif // HAL_GPIO_H