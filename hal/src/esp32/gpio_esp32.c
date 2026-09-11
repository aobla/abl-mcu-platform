/**
 * @brief ESP32 GPIO port (L1).
 *
 * Pin encoding (see abl_gpio.h): port = GPIO number, pin = unused.
 * `speed` has no equivalent on ESP32 and `af` is not used: peripherals are
 * routed to pins by the ESP-IDF driver configuration (GPIO matrix).
 *
 * TODO (Step 9): this port moves into the ESP-IDF wrapper.
 */

#include "abl_gpio.h"

#include "driver/gpio.h"
#include "esp_err.h"

#define ABL_ESP32_MAX_IRQ 8U

typedef struct {
    abl_gpio_irq_cb_t cb;
    void*             arg;
    uint32_t          gpio;
    bool              used;
} esp32_irq_slot_t;

static esp32_irq_slot_t s_irq[ABL_ESP32_MAX_IRQ];
static bool             s_isr_service_installed;

static gpio_num_t to_gpio_num(const abl_gpio_pin_t* pin)
{
    return (gpio_num_t)(intptr_t)pin->port;
}

/* gpio_isr_handler_add() hands the slot pointer back to us as the argument. */
static void esp32_irq_trampoline(void* ctx)
{
    esp32_irq_slot_t* slot = (esp32_irq_slot_t*)ctx;
    if (slot->cb != 0) {
        abl_gpio_pin_t pin = { (void*)(intptr_t)slot->gpio, 0 };
        slot->cb(&pin, slot->arg);
    }
}

/* ─── Configuration ─────────────────────────────────────────────────────── */

abl_status_t abl_gpio_configure(const abl_gpio_pin_t* pin, const abl_gpio_cfg_t* cfg)
{
    if (pin == 0 || cfg == 0) {
        return ABL_STATUS_ERROR;
    }

    const gpio_num_t num = to_gpio_num(pin);

    gpio_config_t io = {0};
    io.pin_bit_mask = (1ULL << (uint32_t)num);
    io.intr_type    = GPIO_INTR_DISABLE;
    io.pull_up_en   = (cfg->pull == ABL_GPIO_PULL_UP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    io.pull_down_en = (cfg->pull == ABL_GPIO_PULL_DOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;

    switch (cfg->mode) {
        case ABL_GPIO_MODE_INPUT:
        case ABL_GPIO_MODE_ANALOG:          /* ADC pins are plain inputs */
            io.mode = GPIO_MODE_INPUT;
            break;

        case ABL_GPIO_MODE_OUTPUT:
        case ABL_GPIO_MODE_ALT_FUNCTION:    /* routing is done by the peripheral driver */
            io.mode = GPIO_MODE_INPUT_OUTPUT;
            break;

        default:
            return ABL_STATUS_ERROR;
    }

    if (gpio_config(&io) != ESP_OK) {
        return ABL_STATUS_ERROR;
    }

    if (cfg->mode != ABL_GPIO_MODE_INPUT && cfg->state != ABL_GPIO_STATE_NONE) {
        if (gpio_set_level(num, (cfg->state == ABL_GPIO_STATE_HIGH) ? 1 : 0) != ESP_OK) {
            return ABL_STATUS_ERROR;
        }
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
    return (gpio_set_level(to_gpio_num(pin), state ? 1 : 0) == ESP_OK) ? ABL_STATUS_OK
                                                                      : ABL_STATUS_ERROR;
}

abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin)
{
    if (pin == 0) {
        return ABL_STATUS_ERROR;
    }

    const int level = gpio_get_level(to_gpio_num(pin));
    return (gpio_set_level(to_gpio_num(pin), level == 0 ? 1 : 0) == ESP_OK) ? ABL_STATUS_OK
                                                                           : ABL_STATUS_ERROR;
}

abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state)
{
    if (pin == 0 || state == 0) {
        return ABL_STATUS_ERROR;
    }
    *state = (gpio_get_level(to_gpio_num(pin)) != 0);
    return ABL_STATUS_OK;
}

/* ─── Interrupts ────────────────────────────────────────────────────────── */

abl_status_t abl_gpio_irq_attach(const abl_gpio_pin_t* pin,
                                 abl_gpio_irq_trigger_t trigger,
                                 abl_gpio_irq_cb_t cb, void* arg)
{
    if (pin == 0 || cb == 0) {
        return ABL_STATUS_ERROR;
    }

    const gpio_num_t num = to_gpio_num(pin);

    gpio_int_type_t intr;
    switch (trigger) {
        case ABL_GPIO_IRQ_RISING:  intr = GPIO_INTR_POSEDGE; break;
        case ABL_GPIO_IRQ_FALLING: intr = GPIO_INTR_NEGEDGE; break;
        case ABL_GPIO_IRQ_BOTH:    intr = GPIO_INTR_ANYEDGE; break;
        default:                   return ABL_STATUS_ERROR;
    }

    uint32_t slot_index = ABL_ESP32_MAX_IRQ;
    for (uint32_t i = 0; i < ABL_ESP32_MAX_IRQ; i++) {
        if (!s_irq[i].used) {
            slot_index = i;
            break;
        }
    }
    if (slot_index == ABL_ESP32_MAX_IRQ) {
        return ABL_STATUS_ERROR;
    }

    if (!s_isr_service_installed) {
        if (gpio_install_isr_service(0) != ESP_OK) {
            return ABL_STATUS_ERROR;
        }
        s_isr_service_installed = true;
    }

    s_irq[slot_index].cb   = cb;
    s_irq[slot_index].arg  = arg;
    s_irq[slot_index].gpio = (uint32_t)num;
    s_irq[slot_index].used = true;

    if (gpio_set_intr_type(num, intr) != ESP_OK ||
        gpio_isr_handler_add(num, esp32_irq_trampoline, &s_irq[slot_index]) != ESP_OK) {
        s_irq[slot_index].used = false;
        return ABL_STATUS_ERROR;
    }

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_irq_detach(const abl_gpio_pin_t* pin)
{
    if (pin == 0) {
        return ABL_STATUS_ERROR;
    }

    const gpio_num_t num = to_gpio_num(pin);

    for (uint32_t i = 0; i < ABL_ESP32_MAX_IRQ; i++) {
        if (s_irq[i].used && s_irq[i].gpio == (uint32_t)num) {
            (void)gpio_isr_handler_remove(num);
            (void)gpio_set_intr_type(num, GPIO_INTR_DISABLE);
            s_irq[i].used = false;
            s_irq[i].cb   = 0;
            s_irq[i].arg  = 0;
            return ABL_STATUS_OK;
        }
    }

    return ABL_STATUS_ERROR;
}
