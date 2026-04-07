#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief ABL HAL status codes
 *
 * HAL_STATUS_ prefix avoids conflicts with vendor HAL libraries
 * (STM32 has HAL_OK, ESP-IDF has ESP_OK, etc.)
 */
typedef enum {
    HAL_STATUS_OK       = 0,
    HAL_STATUS_ERROR    = 1,
    HAL_STATUS_BUSY     = 2,
    HAL_STATUS_TIMEOUT  = 3
} hal_status_t;

#endif /* HAL_COMMON_H */
