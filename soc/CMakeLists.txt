# soc/CMakeLists.txt
# Подъём платформы (D1): target_init + SoC-provided startup/linker/system/hal_conf.

set(SOC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOC_INCLUDE_DIR ${SOC_DIR}/include)

# ─── SoC / семейство по PLATFORM ─────────────────────────────────────────────
if(PLATFORM MATCHES "^stm32f103")
    set(SOC_TARGET_INIT_SRC ${SOC_DIR}/stm32/stm32_target_init.c)
    set(SOC_HAL_CONF_DIR   ${SOC_DIR}/stm32/f1)
    set(SOC_DEVICE         STM32F103x6)
    set(SOC_STARTUP_FILE   ${SOC_HAL_CONF_DIR}/startup_stm32f103x6.s)
    set(SOC_LINKER_SCRIPT  ${SOC_HAL_CONF_DIR}/STM32F103X6_FLASH.ld)
    set(SOC_SYSTEM_FILE    ${SOC_HAL_CONF_DIR}/system_stm32f1xx.c)
elseif(PLATFORM MATCHES "^stm32f4")
    set(SOC_TARGET_INIT_SRC ${SOC_DIR}/stm32/stm32_target_init.c)
    set(SOC_HAL_CONF_DIR   ${SOC_DIR}/stm32/f4)
    set(SOC_DEVICE         STM32F407xx)
    set(SOC_STARTUP_FILE   ${SOC_HAL_CONF_DIR}/startup_stm32f407xx.s)
    set(SOC_LINKER_SCRIPT  ${SOC_HAL_CONF_DIR}/STM32F407xx_FLASH.ld)
    set(SOC_SYSTEM_FILE    ${SOC_HAL_CONF_DIR}/system_stm32f4xx.c)
elseif(PLATFORM MATCHES "^stm32h743")
    set(SOC_TARGET_INIT_SRC ${SOC_DIR}/stm32/stm32_target_init.c)
    set(SOC_HAL_CONF_DIR   ${SOC_DIR}/stm32/h7)
    set(SOC_DEVICE         STM32H743xx)
    set(SOC_STARTUP_FILE   ${SOC_HAL_CONF_DIR}/startup_stm32h743xx.s)
    set(SOC_LINKER_SCRIPT  ${SOC_HAL_CONF_DIR}/STM32H743XI_FLASH.ld)
    set(SOC_SYSTEM_FILE    ${SOC_HAL_CONF_DIR}/system_stm32h7xx.c)
elseif(PLATFORM STREQUAL "avr")
    # TODO (Шаг 8): avr_target_init.c всё ещё ссылается на app.h — чинить на Шаге 8.
    set(SOC_TARGET_INIT_SRC ${SOC_DIR}/avr/avr_target_init.c)
    set(SOC_HAL_CONF_DIR   "")
    set(SOC_DEVICE         "")
    set(SOC_STARTUP_FILE   "")
    set(SOC_LINKER_SCRIPT  "")
    set(SOC_SYSTEM_FILE    "")
elseif(PLATFORM STREQUAL "esp32")
    # TODO (Шаг 9): esp32-порт переводится на обёртку ESP-IDF.
    set(SOC_TARGET_INIT_SRC ${SOC_DIR}/esp32/esp32_target_init.c)
    set(SOC_HAL_CONF_DIR   "")
    set(SOC_DEVICE         "")
    set(SOC_STARTUP_FILE   "")
    set(SOC_LINKER_SCRIPT  "")
    set(SOC_SYSTEM_FILE    "")
else()
    message(FATAL_ERROR "soc: unsupported platform: ${PLATFORM}")
endif()

# Экспортируем в вышестоящие области видимости (используются add_firmware_target)
set(SOC_STARTUP_FILE  "${SOC_STARTUP_FILE}"  CACHE FILEPATH "SoC startup file" FORCE)
set(SOC_LINKER_SCRIPT "${SOC_LINKER_SCRIPT}" CACHE FILEPATH "SoC linker script" FORCE)
set(SOC_SYSTEM_FILE   "${SOC_SYSTEM_FILE}"   CACHE FILEPATH "SoC system file" FORCE)
set(SOC_DEVICE        "${SOC_DEVICE}"        CACHE STRING  "SoC device define" FORCE)
set(SOC_HAL_CONF_DIR  "${SOC_HAL_CONF_DIR}"  CACHE PATH    "SoC HAL config dir" FORCE)

# ─── target_init ─────────────────────────────────────────────────────────────
add_library(target_init STATIC ${SOC_TARGET_INIT_SRC})
target_include_directories(target_init PUBLIC
    ${SOC_INCLUDE_DIR}
    ${SOC_DIR}/../hal/include
)
if(PLATFORM MATCHES "^stm32")
    target_include_directories(target_init PUBLIC ${SOC_HAL_CONF_DIR})
    if(DEFINED STM32_INCLUDE_DIRS)
        target_include_directories(target_init PUBLIC ${STM32_INCLUDE_DIRS})
    endif()
    target_compile_definitions(target_init PRIVATE
        PLATFORM_${PLATFORM_UPPER} USE_HAL_DRIVER ${SOC_DEVICE}
    )
elseif(PLATFORM STREQUAL "avr")
    target_compile_definitions(target_init PRIVATE PLATFORM_AVR)
elseif(PLATFORM STREQUAL "esp32")
    target_compile_definitions(target_init PRIVATE PLATFORM_ESP32)
endif()
