#ifndef ABL_GPIO_H
#define ABL_GPIO_H

#include "abl_common.h"

/* ─── Types ─────────────────────────────────────────────────────────────── */

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

/** Output slew rate (ignored by platforms that have no such control). */
typedef enum {
    ABL_GPIO_SPEED_LOW,
    ABL_GPIO_SPEED_MEDIUM,
    ABL_GPIO_SPEED_HIGH,
    ABL_GPIO_SPEED_VERY_HIGH
} abl_gpio_speed_t;

/** Output driver type. */
typedef enum {
    ABL_GPIO_OTYPE_PUSH_PULL,
    ABL_GPIO_OTYPE_OPEN_DRAIN
} abl_gpio_otype_t;

/** Interrupt trigger selection. */
typedef enum {
    ABL_GPIO_IRQ_NONE,
    ABL_GPIO_IRQ_RISING,
    ABL_GPIO_IRQ_FALLING,
    ABL_GPIO_IRQ_BOTH
} abl_gpio_irq_trigger_t;

/**
 * @brief Generic GPIO pin handle.
 *
 * The meaning of `port` and `pin` is platform-specific:
 *   STM32: port = GPIO_TypeDef* (e.g. GPIOA), pin = pin number (0..15)
 *   AVR:   port = (void*)(uintptr_t)port_index (0=PORTB, 1=PORTC, ...), pin = bit
 *   ESP32: port = (void*)(uintptr_t)gpio_num, pin = 0 (unused)
 */
typedef struct {
    void*    port;
    uint16_t pin;
} abl_gpio_pin_t;

/**
 * @brief Full pin configuration.
 *
 * Zero-initialize the structure and fill in the fields you need; the defaults
 * are: push-pull, low speed, no alternate function.
 */
typedef struct {
    abl_gpio_mode_t  mode;   /*!< direction / function */
    abl_gpio_pull_t  pull;   /*!< internal pull resistor */
    abl_gpio_state_t state;  /*!< initial output level (output mode only) */
    abl_gpio_speed_t speed;  /*!< output slew rate */
    abl_gpio_otype_t otype;  /*!< push-pull or open-drain */
    uint8_t          af;     /*!< alternate function number (0 = not applicable) */
} abl_gpio_cfg_t;

/** Interrupt callback: called from ISR context. */
typedef void (*abl_gpio_irq_cb_t)(const abl_gpio_pin_t* pin, void* arg);

/* ─── Configuration ─────────────────────────────────────────────────────── */

/** Configure a pin (full configuration; preferred entry point). */
abl_status_t abl_gpio_configure(const abl_gpio_pin_t* pin, const abl_gpio_cfg_t* cfg);

/** Convenience wrapper: mode + pull + initial state, defaults for the rest. */
abl_status_t abl_gpio_init(const abl_gpio_pin_t* pin, abl_gpio_mode_t mode,
                           abl_gpio_pull_t pull, abl_gpio_state_t state);

/* ─── Basic I/O ─────────────────────────────────────────────────────────── */

/** Drive an output pin. */
abl_status_t abl_gpio_write(const abl_gpio_pin_t* pin, bool state);

/** Toggle an output pin. */
abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin);

/** Read the current level of a pin. */
abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state);

/* ─── Interrupts ────────────────────────────────────────────────────────── */

/**
 * @brief Attach an interrupt callback to a pin.
 *
 * The callback runs in ISR context: it must be short and must not sleep.
 * Configure the pin (input + pull) separately before enabling its interrupt.
 *
 * @return ABL_STATUS_OK on success, ABL_STATUS_ERROR if the pin cannot be used
 *         as an interrupt source (also when the line is already taken by a pin
 *         of another port — a hardware limitation of shared EXTI lines).
 */
abl_status_t abl_gpio_irq_attach(const abl_gpio_pin_t* pin,
                                 abl_gpio_irq_trigger_t trigger,
                                 abl_gpio_irq_cb_t cb, void* arg);

/** Detach the interrupt callback and disable the pin's interrupt. */
abl_status_t abl_gpio_irq_detach(const abl_gpio_pin_t* pin);

#endif /* ABL_GPIO_H */
