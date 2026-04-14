#include "abl_gpio.h"

/* HAL header — selected by compile-time define from toolchain */
#if defined(PLATFORM_STM32F1) || defined(PLATFORM_STM32F103)
#include <stm32f1xx_hal.h>
#elif defined(PLATFORM_STM32H7) || defined(PLATFORM_STM32H743)
#include <stm32h7xx_hal.h>
#else
#include <stm32f4xx_hal.h>
#endif

static GPIO_TypeDef* get_port_from_handle(void* port) {
    return (GPIO_TypeDef*)port;
}

abl_status_t abl_gpio_init(abl_gpio_pin_t* pin, abl_gpio_mode_t mode, abl_gpio_pull_t pull, abl_gpio_state_t state) {
    if (!pin || !pin->port) {
        return ABL_STATUS_ERROR;
    }

    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return ABL_STATUS_ERROR;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = (1U << pin->pin);

    if (gpio_port == GPIOA) {
        if (__HAL_RCC_GPIOA_IS_CLK_DISABLED())
            __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (gpio_port == GPIOB) {
        if (__HAL_RCC_GPIOB_IS_CLK_DISABLED())
            __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (gpio_port == GPIOC) {
        if (__HAL_RCC_GPIOC_IS_CLK_DISABLED())
            __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (gpio_port == GPIOD) {
        if (__HAL_RCC_GPIOD_IS_CLK_DISABLED())
            __HAL_RCC_GPIOD_CLK_ENABLE();
/** \todo add defines for for all series and ports */
#if defined(PLATFORM_STM32F4)
    } else if (gpio_port == GPIOE) {
        if (__HAL_RCC_GPIOE_IS_CLK_DISABLED())
            __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (gpio_port == GPIOF) {
        if (__HAL_RCC_GPIOF_IS_CLK_DISABLED())
            __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (gpio_port == GPIOG) {
        if (__HAL_RCC_GPIOG_IS_CLK_DISABLED())
            __HAL_RCC_GPIOG_CLK_ENABLE();
    } else if (gpio_port == GPIOH) {
        if (__HAL_RCC_GPIOH_IS_CLK_DISABLED())
            __HAL_RCC_GPIOH_CLK_ENABLE();
    } else if (gpio_port == GPIOI) {
        if (__HAL_RCC_GPIOI_IS_CLK_DISABLED())
            __HAL_RCC_GPIOI_CLK_ENABLE();
#endif
    } else {
        return ABL_STATUS_ERROR;
    }

    switch (mode) {
        case ABL_GPIO_MODE_INPUT:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            break;
        case ABL_GPIO_MODE_OUTPUT:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            break;
        case ABL_GPIO_MODE_ALT_FUNCTION:
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            break;
        case ABL_GPIO_MODE_ANALOG:
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            break;
        default:
            return ABL_STATUS_ERROR;
    }

    switch (pull) {
        case ABL_GPIO_PULL_UP:
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            break;
        case ABL_GPIO_PULL_DOWN:
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
            break;
        case ABL_GPIO_PULL_NONE:
        default:
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            break;
    }

    HAL_GPIO_Init(gpio_port, &GPIO_InitStruct);

    if (state != ABL_GPIO_PULL_NONE)
        HAL_GPIO_WritePin(gpio_port, (1U << pin->pin), state == ABL_GPIO_STATE_HIGH ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_write(const abl_gpio_pin_t* pin, bool state) {
    if (!pin || !pin->port) {
        return ABL_STATUS_ERROR;
    }

    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return ABL_STATUS_ERROR;
    }

    HAL_GPIO_WritePin(gpio_port, (1U << pin->pin), state ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin) {
    if (!pin || !pin->port) {
        return ABL_STATUS_ERROR;
    }

    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return ABL_STATUS_ERROR;
    }

    HAL_GPIO_TogglePin(gpio_port, (1U << pin->pin));

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state) {
    if (!pin || !pin->port || !state) {
        return ABL_STATUS_ERROR;
    }

    GPIO_TypeDef* gpio_port = get_port_from_handle(pin->port);
    if (!gpio_port) {
        return ABL_STATUS_ERROR;
    }

    *state = (HAL_GPIO_ReadPin(gpio_port, (1U << pin->pin)) == GPIO_PIN_SET);

    return ABL_STATUS_OK;
}
