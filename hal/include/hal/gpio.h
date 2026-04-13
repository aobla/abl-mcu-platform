#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "hal/common.h"

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
    HAL_GPIO_STATE_NONE,
    HAL_GPIO_STATE_HIGH,
    HAL_GPIO_STATE_LOW
} hal_gpio_state_t;

/**
 * @brief Generic GPIO pin handle
 *
 * The meaning of `port` and `pin` is platform-specific:
 *   STM32: port = GPIO_TypeDef* (e.g. GPIOA), pin = pin mask (1U << n)
 *   AVR:   port = (void*)(uintptr_t)port_index (0=PORTB, 1=PORTC, ...), pin = bit number
 *   ESP32: port = (void*)(uintptr_t)gpio_num, pin = 0 (unused)
 */
typedef struct {
    void*    port;
    uint16_t pin;
} hal_gpio_pin_t;

/**
 * @brief Инициализирует GPIO пин
 */
hal_status_t hal_gpio_init(hal_gpio_pin_t* pin, hal_gpio_mode_t mode, hal_gpio_pull_t pull, hal_gpio_state_t state);

/**
 * @brief Записывает значение на GPIO пин
 */
hal_status_t hal_gpio_write(const hal_gpio_pin_t* pin, bool state);

/**
 * @brief Переключает состояние GPIO пина
 */
hal_status_t hal_gpio_toggle(const hal_gpio_pin_t* pin);

/**
 * @brief Считывает состояние GPIO пина
 */
hal_status_t hal_gpio_read(const hal_gpio_pin_t* pin, bool* state);

#endif /* HAL_GPIO_H */
