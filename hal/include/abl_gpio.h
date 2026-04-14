#ifndef ABL_GPIO_H
#define ABL_GPIO_H

#include "abl_common.h"

typedef enum {
    ABL_GPIO_MODE_INPUT,
    ABL_GPIO_MODE_OUTPUT,
    ABL_GPIO_MODE_ALT_FUNCTION,
    ABL_GPIO_MODE_ANALOG
} abl_gpio_mode_t;

typedef enum {
    ABL_GPIO_PULL_NONE,
    ABL_GPIO_PULL_UP,
    ABL_GPIO_PULL_DOWN
} abl_gpio_pull_t;

typedef enum {
    ABL_GPIO_STATE_NONE,
    ABL_GPIO_STATE_HIGH,
    ABL_GPIO_STATE_LOW
} abl_gpio_state_t;

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
} abl_gpio_pin_t;

/**
 * @brief Инициализирует GPIO пин
 */
abl_status_t abl_gpio_init(abl_gpio_pin_t* pin, abl_gpio_mode_t mode, abl_gpio_pull_t pull, abl_gpio_state_t state);

/**
 * @brief Записывает значение на GPIO пин
 */
abl_status_t abl_gpio_write(const abl_gpio_pin_t* pin, bool state);

/**
 * @brief Переключает состояние GPIO пина
 */
abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin);

/**
 * @brief Считывает состояние GPIO пина
 */
abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state);

#endif /* ABL_GPIO_H */
