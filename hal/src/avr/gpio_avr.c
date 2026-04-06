#include "hal/gpio.h"
#include <avr/io.h>
#include <util/delay.h>

/*
 * В AVR порты — это не указатели, а SFR-макросы (PORTB, DDRC и т.д.).
 * В hal_gpio_pin_t::port мы храним индекс порта (0 = PORTB, 1 = PORTC, ...),
 * а не адрес. Функции ниже преобразуют индекс в реальные регистры.
 *
 * Для ATmega328P:
 *   PORTB → DDRB=0x04, PORTB=0x05, PINB=0x03
 *   PORTC → DDRC=0x07, PORTC=0x08, PINC=0x06
 *   PORTD → DDRD=0x0A, PORTD=0x0B, PIND=0x09
 *
 * Регистры НЕ расположены последовательно, поэтому используем
 * таблицу адресов из avr/io.h.
 */

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pin;
} avr_port_regs;

/*
 * Индексы портов задаются в hardware_pins.h как:
 *   #define LED_PORT 0   // PORTB
 *   #define LED_PORT 1   // PORTC
 *   #define LED_PORT 2   // PORTD
 */
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
        return HAL_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_ERROR;
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
            /* AVR не имеет универсальной альтернативной функции —
               настраиваем как вход, конкретная периферия настраивается отдельно */
            *regs->ddr &= ~pin_mask;
            break;

        default:
            return HAL_ERROR;
    }

    return HAL_OK;
}

hal_status_t hal_gpio_write(hal_gpio_pin_t* pin, bool state) {
    if (!pin) {
        return HAL_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);

    if (state) {
        *regs->port |= pin_mask;
    } else {
        *regs->port &= ~pin_mask;
    }

    return HAL_OK;
}

hal_status_t hal_gpio_toggle(hal_gpio_pin_t* pin) {
    if (!pin) {
        return HAL_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);
    *regs->pin |= pin_mask;  /* Запись в PINx переключает PORTx (ATmega особенность) */

    return HAL_OK;
}

hal_status_t hal_gpio_read(hal_gpio_pin_t* pin, bool* state) {
    if (!pin || !state) {
        return HAL_ERROR;
    }

    avr_port_regs* regs = get_port_regs(pin->port);
    if (!regs) {
        return HAL_ERROR;
    }

    uint8_t pin_mask = (1 << pin->pin);
    *state = ((*regs->pin & pin_mask) != 0);

    return HAL_OK;
}
