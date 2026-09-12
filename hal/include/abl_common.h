#ifndef ABL_COMMON_H
#define ABL_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief ABL HAL status codes
 *
 * ABL_STATUS_ prefix avoids conflicts with vendor HAL libraries
 * (STM32 has ABL_OK, ESP-IDF has ESP_OK, etc.)
 */
typedef enum {
    ABL_STATUS_OK       = 0,
    ABL_STATUS_ERROR    = 1,
    ABL_STATUS_BUSY     = 2,
    ABL_STATUS_TIMEOUT  = 3
} abl_status_t;

#endif /* ABL_COMMON_H */
