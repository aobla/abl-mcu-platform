#include "hal/gpio.h"
#include <stm32f4xx_hal.h>  // Включаем HAL STM32

// Внутренняя функция для преобразования номера пина в GPIO_TypeDef*
static GPIO_TypeDef* get_port_from_handle(void* port) {
    return (GPIO_TypeDef*)port;
}

hal_status_t hal_gpio_init(hal_gpio_pin_t* pin, hal_gpio_mode_t mode, hal_gpio_pull_t pull) {
    if (!pin || !pin->port) {
        return HAL_ERROR;
    }
    
    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return HAL_ERROR;
    }
    
    // Настройка структуры инициализации GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = (1U << pin->pin);
    
    switch (mode) {
        case HAL_GPIO_MODE_INPUT:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            break;
        case HAL_GPIO_MODE_OUTPUT:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            break;
        case HAL_GPIO_MODE_ALT_FUNCTION:
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            break;
        case HAL_GPIO_MODE_ANALOG:
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            break;
        default:
            return HAL_ERROR;
    }
    
    switch (pull) {
        case HAL_GPIO_PULL_UP:
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            break;
        case HAL_GPIO_PULL_DOWN:
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
            break;
        case HAL_GPIO_PULL_NONE:
        default:
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            break;
    }
    
    // Инициализируем пин
    HAL_GPIO_Init(gpio_port, &GPIO_InitStruct);
    
    return HAL_OK;
}

hal_status_t hal_gpio_write(hal_gpio_pin_t* pin, bool state) {
    if (!pin || !pin->port) {
        return HAL_ERROR;
    }
    
    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return HAL_ERROR;
    }
    
    HAL_GPIO_WritePin(gpio_port, (1U << pin->pin), state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    return HAL_OK;
}

hal_status_t hal_gpio_toggle(hal_gpio_pin_t* pin) {
    if (!pin || !pin->port) {
        return HAL_ERROR;
    }
    
    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return HAL_ERROR;
    }
    
    HAL_GPIO_TogglePin(gpio_port, (1U << pin->pin));
    
    return HAL_OK;
}

hal_status_t hal_gpio_read(hal_gpio_pin_t* pin, bool* state) {
    if (!pin || !pin->port || !state) {
        return HAL_ERROR;
    }
    
    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return HAL_ERROR;
    }
    
    *state = (HAL_GPIO_ReadPin(gpio_port, (1U << pin->pin)) == GPIO_PIN_SET);
    
    return HAL_OK;
}