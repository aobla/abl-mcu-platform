/**
 * @brief Native (host) GPIO port (L1) — simulation (D8).
 *
 * Pins are identified by (port, pin) pairs, exactly like on real hardware, but
 * everything is kept in a small in-memory registry:
 *   - abl_gpio_write()/toggle() update the simulated output level;
 *   - abl_gpio_read() returns the simulated input level (written by the test
 *     through abl_native_gpio_inject(), or the last written output level).
 *
 * Setting ABL_NATIVE_GPIO_TRACE=1 in the environment prints every transition,
 * which makes the native blink demo observable.
 */

#include "abl_gpio.h"
#include "abl_native_sim.h"

#include <stdio.h>
#include <stdlib.h>

#define ABL_NATIVE_MAX_PINS 32U

typedef struct {
    void*    port;
    uint16_t pin;
    bool     used;
    bool     output;
    bool     level;
} native_pin_slot_t;

static native_pin_slot_t s_pins[ABL_NATIVE_MAX_PINS];

static bool trace_enabled(void)
{
    static int state = -1;
    if (state < 0) {
        const char* env = getenv("ABL_NATIVE_GPIO_TRACE");
        state = (env != 0 && env[0] != '\0' && env[0] != '0') ? 1 : 0;
    }
    return state == 1;
}

static native_pin_slot_t* slot_find(const abl_gpio_pin_t* pin, bool create)
{
    if (pin == 0) {
        return 0;
    }

    for (uint32_t i = 0; i < ABL_NATIVE_MAX_PINS; i++) {
        if (s_pins[i].used && s_pins[i].port == pin->port && s_pins[i].pin == pin->pin) {
            return &s_pins[i];
        }
    }

    if (!create) {
        return 0;
    }

    for (uint32_t i = 0; i < ABL_NATIVE_MAX_PINS; i++) {
        if (!s_pins[i].used) {
            s_pins[i].used  = true;
            s_pins[i].port  = pin->port;
            s_pins[i].pin   = pin->pin;
            s_pins[i].level = false;
            return &s_pins[i];
        }
    }

    return 0;
}

static void slot_set_level(native_pin_slot_t* slot, bool level)
{
    if (slot->level != level && trace_enabled()) {
        printf("[native-gpio] port=%p pin=%u -> %u\n", slot->port, (unsigned)slot->pin,
               (unsigned)level);
        fflush(stdout);
    }
    slot->level = level;
}

/* ─── Configuration ─────────────────────────────────────────────────────── */

abl_status_t abl_gpio_configure(const abl_gpio_pin_t* pin, const abl_gpio_cfg_t* cfg)
{
    if (pin == 0 || cfg == 0) {
        return ABL_STATUS_ERROR;
    }

    native_pin_slot_t* slot = slot_find(pin, true);
    if (slot == 0) {
        return ABL_STATUS_ERROR;   /* registry full */
    }

    switch (cfg->mode) {
        case ABL_GPIO_MODE_OUTPUT:
        case ABL_GPIO_MODE_ALT_FUNCTION:
            slot->output = true;
            if (cfg->state != ABL_GPIO_STATE_NONE) {
                slot_set_level(slot, cfg->state == ABL_GPIO_STATE_HIGH);
            }
            break;

        case ABL_GPIO_MODE_INPUT:
        case ABL_GPIO_MODE_ANALOG:
            slot->output = false;
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
    native_pin_slot_t* slot = slot_find(pin, false);
    if (slot == 0) {
        return ABL_STATUS_ERROR;   /* write to an unconfigured pin */
    }
    slot_set_level(slot, state);
    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin)
{
    native_pin_slot_t* slot = slot_find(pin, false);
    if (slot == 0) {
        return ABL_STATUS_ERROR;
    }
    slot_set_level(slot, !slot->level);
    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state)
{
    if (state == 0) {
        return ABL_STATUS_ERROR;
    }

    native_pin_slot_t* slot = slot_find(pin, false);
    if (slot == 0) {
        return ABL_STATUS_ERROR;
    }

    *state = slot->level;
    return ABL_STATUS_OK;
}

/* ─── Interrupts ────────────────────────────────────────────────────────── */

abl_status_t abl_gpio_irq_attach(const abl_gpio_pin_t* pin,
                                 abl_gpio_irq_trigger_t trigger,
                                 abl_gpio_irq_cb_t cb, void* arg)
{
    (void)pin; (void)trigger; (void)cb; (void)arg;
    /* The host has no interrupts; tests drive the code synchronously instead. */
    return ABL_STATUS_ERROR;
}

abl_status_t abl_gpio_irq_detach(const abl_gpio_pin_t* pin)
{
    (void)pin;
    return ABL_STATUS_ERROR;
}

/* ─── Simulation hooks (host tests only) ────────────────────────────────── */

abl_status_t abl_native_gpio_inject(const abl_gpio_pin_t* pin, bool level)
{
    native_pin_slot_t* slot = slot_find(pin, true);
    if (slot == 0) {
        return ABL_STATUS_ERROR;
    }
    slot->output = false;   /* simulated input */
    slot_set_level(slot, level);
    return ABL_STATUS_OK;
}

abl_status_t abl_native_gpio_get(const abl_gpio_pin_t* pin, bool* level)
{
    if (level == 0) {
        return ABL_STATUS_ERROR;
    }
    const native_pin_slot_t* slot = slot_find(pin, false);
    if (slot == 0) {
        return ABL_STATUS_ERROR;
    }
    *level = slot->level;
    return ABL_STATUS_OK;
}

unsigned abl_native_gpio_count(void)
{
    unsigned count = 0;
    for (uint32_t i = 0; i < ABL_NATIVE_MAX_PINS; i++) {
        if (s_pins[i].used) {
            count++;
        }
    }
    return count;
}
