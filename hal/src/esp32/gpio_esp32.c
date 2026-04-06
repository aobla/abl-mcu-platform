#include "hal/gpio.h"
#include <driver/gpio.h>  // Включаем драйвер GPIO из ESP-IDF

// Внутренняя функция для преобразования номера пина
static gpio_num_t get_pin_from_handle(void* port) {
    // В ESP32 порт - это просто номер пина, так как у него нет понятия портов как у AVR/STM32
    return (gpio_num_t)(intptr_t)port;
}

hal_status_t hal_gpio_init(hal_gpio_pin_t* pin, hal_gpio_mode_t mode, hal_gpio_pull_t pull) {
    if (!pin) {
        return HAL_ERROR;
    }
    
    gpio_num_t gpio_num = get_pin_from_handle(pin->port);

    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << gpio_num);
    io_conf.mode = GPIO_MODE_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    switch (mode) {
        case HAL_GPIO_MODE_INPUT:
            io_conf.mode = GPIO_MODE_INPUT;
            break;
        case HAL_GPIO_MODE_OUTPUT:
            io_conf.mode = GPIO_MODE_OUTPUT;
            break;
        case HAL_GPIO_MODE_ALT_FUNCTION:
            // В ESP32 альтернативные функции настраиваются через rtc_gpio или специальные функции
            io_conf.mode = GPIO_MODE_INPUT_OUTPUT;  // Ближайшая аналогия
            break;
        case HAL_GPIO_MODE_ANALOG:
            // В ESP32 аналоговые пины требуют специальной настройки
            // Для простоты будем использовать как обычный вход
            io_conf.mode = GPIO_MODE_INPUT;
            break;
        default:
            return HAL_ERROR;
    }
    
    switch (pull) {
        case HAL_GPIO_PULL_UP:
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case HAL_GPIO_PULL_DOWN:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        case HAL_GPIO_PULL_NONE:
        default:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
    }
    
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

hal_status_t hal_gpio_write(hal_gpio_pin_t* pin, bool state) {
    if (!pin) {
        return HAL_ERROR;
    }
    
    gpio_num_t gpio_num = get_pin_from_handle(pin->port);
    
    esp_err_t err = gpio_set_level(gpio_num, state ? 1 : 0);
    if (err != ESP_OK) {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

hal_status_t hal_gpio_toggle(hal_gpio_pin_t* pin) {
    if (!pin) {
        return HAL_ERROR;
    }
    
    bool current_state;
    hal_status_t status = hal_gpio_read(pin, &current_state);
    if (status != HAL_OK) {
        return status;
    }
    
    return hal_gpio_write(pin, !current_state);
}

hal_status_t hal_gpio_read(hal_gpio_pin_t* pin, bool* state) {
    if (!pin || !state) {
        return HAL_ERROR;
    }
    
    gpio_num_t gpio_num = get_pin_from_handle(pin->port);
    
    int level = gpio_get_level(gpio_num);
    *state = (level == 1);
    
    return HAL_OK;
}