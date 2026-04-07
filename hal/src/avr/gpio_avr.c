#include "hal/gpio.h"
#include <avr/io.h>
#include <util/delay.h>

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pin;
} avr_port_regs;

static const avr_port_regs port_table[] = {
#if defined(PORTB)
    { &DDRB, &PORTB, &PINB },
#endif
#if defined(PORTC)
    { &DDRC, &PORTC, &PINC },
#endif
#if defined(PORTD)
    { &DDRD, &PORTD, &PIND },
#endif
#if defined(PORTE)
    { &DDRE, &PORTE, &PINE },
#endif
#if defined(PORTF)
    { &DDRF, &PORTF, &PINF },
#endif
};

static avr_port_regs* get_port_regs(void* port) {
    uintptr_t idx = (uintptr_t)port;
    if (idx >= sizeof(port_table) / sizeof(port_table[0])) {
        return NULL;
    }
    return (avr_port_regs*)&port_table[idx];
}

hal_status_t hal_gpio_init(hal_gpio_pin_t* pin, hal_gpio_mode_t mode, hal_gpio_pull_t pull) {
    if (!pin) {
        return HAL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);

    switch (mode) {
        case HAL_GPIO_MODE_OUTPUT:
            *regs->ddr |= pin_mask;
            break;

        case HAL_GPIO_MODE_INPUT:
            *regs->ddr &= ~pin_mask;
            if (pull == HAL_GPIO_PULL_UP) {
                *regs->port |= pin_mask;
            } else {
                *regs->port &= ~pin_mask;
            }
            break;

        case HAL_GPIO_MODE_ALT_FUNCTION:
        case HAL_GPIO_MODE_ANALOG:
            *regs->ddr &= ~pin_mask;
            break;

        default:
            return HAL_STATUS_ERROR;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_gpio_write(hal_gpio_pin_t* pin, bool state) {
    if (!pin) {
        return HAL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);

    if (state) {
        *regs->port |= pin_mask;
    } else {
        *regs->port &= ~pin_mask;
    }

    return HAL_STATUS_OK;
}

hal_status_t hal_gpio_toggle(hal_gpio_pin_t* pin) {
    if (!pin) {
        return HAL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);
    *regs->pin |= pin_mask;

    return HAL_STATUS_OK;
}

hal_status_t hal_gpio_read(hal_gpio_pin_t* pin, bool* state) {
    if (!pin || !state) {
        return HAL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);
    *state = ((*regs->pin & pin_mask) != 0);

    return HAL_STATUS_OK;
}
