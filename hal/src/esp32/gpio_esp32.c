#include "abl_gpio.h"
#include <driver/gpio.h>

static gpio_num_t get_pin_from_handle(void* port) {
    return (gpio_num_t)(intptr_t)port;
}

abl_status_t abl_gpio_init(abl_gpio_pin_t* pin, abl_gpio_mode_t mode, abl_gpio_pull_t pull, abl_gpio_state_t state) {
    if (!pin) {
        return ABL_STATUS_ERROR;
    }

    gpio_num_t gpio_num = get_pin_from_handle(pin->port);

    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    io_conf.mode = GPIO_MODE_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    switch (mode) {
        case ABL_GPIO_MODE_INPUT:
            io_conf.mode = GPIO_MODE_INPUT;
            break;
        case ABL_GPIO_MODE_OUTPUT:
            io_conf.mode = GPIO_MODE_OUTPUT;
            break;
        case ABL_GPIO_MODE_ALT_FUNCTION:
            io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
            break;
        case ABL_GPIO_MODE_ANALOG:
            io_conf.mode = GPIO_MODE_INPUT;
            break;
        default:
            return ABL_STATUS_ERROR;
    }

    switch (pull) {
        case ABL_GPIO_PULL_UP:
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case ABL_GPIO_PULL_DOWN:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        case ABL_GPIO_PULL_NONE:
        default:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
    }

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        return ABL_STATUS_ERROR;
    }

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_write(const abl_gpio_pin_t* pin, bool state) {
    if (!pin) {
        return ABL_STATUS_ERROR;
    }

    gpio_num_t gpio_num = get_pin_from_handle(pin->port);

    esp_err_t err = gpio_set_level(gpio_num, state ? 1 : 0);
    if (err != ESP_OK) {
        return ABL_STATUS_ERROR;
    }

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin) {
    if (!pin) {
        return ABL_STATUS_ERROR;
    }

    bool current_state;
    abl_status_t status = abl_gpio_read(pin, &current_state);
    if (status != ABL_STATUS_OK) {
        return status;
    }

    return abl_gpio_write(pin, !current_state);
}

abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state) {
    if (!pin || !state) {
        return ABL_STATUS_ERROR;
    }

    gpio_num_t gpio_num = get_pin_from_handle(pin->port);

    int level = gpio_get_level(gpio_num);
    *state = (level == 1);

    return ABL_STATUS_OK;
}
