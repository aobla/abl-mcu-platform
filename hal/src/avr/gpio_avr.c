/**
 * @brief AVR GPIO port (L1).
 *
 * Pin encoding (see abl_gpio.h): port = index in the table below, where
 * 0 = PORTB, 1 = PORTC, ...; pin = bit number. Index 0 is a valid port here.
 *
 * speed/otype/alternate function have no AVR8 equivalent and are ignored.
 * Per-pin interrupts are not supported (AVR8 provides port-level pin-change
 * interrupts), so the IRQ entry points report ABL_STATUS_ERROR.
 */

#include "abl_gpio.h"

#include <avr/io.h>

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pin;
} avr_port_regs;

static const avr_port_regs s_ports[] = {
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

#define ABL_AVR_PORT_COUNT (sizeof(s_ports) / sizeof(s_ports[0]))

/* Note: port index 0 (PORTB) is valid, so only the range is checked. */
static const avr_port_regs* port_regs(const abl_gpio_pin_t* pin)
{
    const uintptr_t index = (uintptr_t)pin->port;
    if (index >= ABL_AVR_PORT_COUNT) {
        return 0;
    }
    return &s_ports[index];
}

/* ─── Configuration ─────────────────────────────────────────────────────── */

abl_status_t abl_gpio_configure(const abl_gpio_pin_t* pin, const abl_gpio_cfg_t* cfg)
{
    if (pin == 0 || cfg == 0) {
        return ABL_STATUS_ERROR;
    }

    const avr_port_regs* regs = port_regs(pin);
    if (regs == 0 || pin->pin > 7U) {
        return ABL_STATUS_ERROR;
    }

    const uint8_t mask = (uint8_t)(1U << pin->pin);

    switch (cfg->mode) {
        case ABL_GPIO_MODE_OUTPUT:
            *regs->ddr |= mask;
            if (cfg->state == ABL_GPIO_STATE_HIGH) {
                *regs->port |= mask;
            } else if (cfg->state == ABL_GPIO_STATE_LOW) {
                *regs->port &= (uint8_t)~mask;
            }
            break;

        case ABL_GPIO_MODE_INPUT:
            *regs->ddr &= (uint8_t)~mask;
            /* On AVR8 the pull-up is enabled by writing 1 to the PORT bit. */
            if (cfg->pull == ABL_GPIO_PULL_UP) {
                *regs->port |= mask;
            } else {
                *regs->port &= (uint8_t)~mask;
            }
            break;

        case ABL_GPIO_MODE_ALT_FUNCTION:
        case ABL_GPIO_MODE_ANALOG:
            /* No GPIO mux on AVR8: keep the pin as a plain input. */
            *regs->ddr &= (uint8_t)~mask;
            break;

        default:
            return ABL_STATUS_ERROR;
    }

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_init(const abl_gpio_pin_t* pin, abl_gpio_mode_t mode,
                           abl_gpio_pull_t pull, abl_gpio_state_t state)
{
    abl_gpio_cfg_t cfg = {0};
    cfg.mode  = mode;
    cfg.pull  = pull;
    cfg.state = state;
    return abl_gpio_configure(pin, &cfg);
}

/* ─── Basic I/O ─────────────────────────────────────────────────────────── */

abl_status_t abl_gpio_write(const abl_gpio_pin_t* pin, bool state)
{
    if (pin == 0) {
        return ABL_STATUS_ERROR;
    }

    const avr_port_regs* regs = port_regs(pin);
    if (regs == 0 || pin->pin > 7U) {
        return ABL_STATUS_ERROR;
    }

    const uint8_t mask = (uint8_t)(1U << pin->pin);
    if (state) {
        *regs->port |= mask;
    } else {
        *regs->port &= (uint8_t)~mask;
    }
    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin)
{
    if (pin == 0) {
        return ABL_STATUS_ERROR;
    }

    const avr_port_regs* regs = port_regs(pin);
    if (regs == 0 || pin->pin > 7U) {
        return ABL_STATUS_ERROR;
    }

    /* Writing 1 to PINx toggles the corresponding PORTx bit. */
    *regs->pin = (uint8_t)(1U << pin->pin);
    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state)
{
    if (pin == 0 || state == 0) {
        return ABL_STATUS_ERROR;
    }

    const avr_port_regs* regs = port_regs(pin);
    if (regs == 0 || pin->pin > 7U) {
        return ABL_STATUS_ERROR;
    }

    *state = ((*regs->pin & (uint8_t)(1U << pin->pin)) != 0U);
    return ABL_STATUS_OK;
}

/* ─── Interrupts ────────────────────────────────────────────────────────── */

abl_status_t abl_gpio_irq_attach(const abl_gpio_pin_t* pin,
                                 abl_gpio_irq_trigger_t trigger,
                                 abl_gpio_irq_cb_t cb, void* arg)
{
    (void)pin; (void)trigger; (void)cb; (void)arg;
    /* TODO (Step 8+): port-level pin-change interrupts (PCINT) if needed. */
    return ABL_STATUS_ERROR;
}

abl_status_t abl_gpio_irq_detach(const abl_gpio_pin_t* pin)
{
    (void)pin;
    return ABL_STATUS_ERROR;
}
