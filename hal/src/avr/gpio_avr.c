#include "abl_gpio.h"
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

abl_status_t abl_gpio_init(abl_gpio_pin_t* pin, abl_gpio_mode_t mode, abl_gpio_pull_t pull) {
    if (!pin) {
        return ABL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return ABL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);

    switch (mode) {
        case ABL_GPIO_MODE_OUTPUT:
            *regs->ddr |= pin_mask;
            break;

        case ABL_GPIO_MODE_INPUT:
            *regs->ddr &= ~pin_mask;
            if (pull == ABL_GPIO_PULL_UP) {
                *regs->port |= pin_mask;
            } else {
                *regs->port &= ~pin_mask;
            }
            break;

        case ABL_GPIO_MODE_ALT_FUNCTION:
        case ABL_GPIO_MODE_ANALOG:
            *regs->ddr &= ~pin_mask;
            break;

        default:
            return ABL_STATUS_ERROR;
    }

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_write(abl_gpio_pin_t* pin, bool state) {
    if (!pin) {
        return ABL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return ABL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);

    if (state) {
        *regs->port |= pin_mask;
    } else {
        *regs->port &= ~pin_mask;
    }

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_toggle(abl_gpio_pin_t* pin) {
    if (!pin) {
        return ABL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return ABL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);
    *regs->pin |= pin_mask;

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_read(abl_gpio_pin_t* pin, bool* state) {
    if (!pin || !state) {
        return ABL_STATUS_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return ABL_STATUS_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);
    *state = ((*regs->pin & pin_mask) != 0);

    return ABL_STATUS_OK;
}
