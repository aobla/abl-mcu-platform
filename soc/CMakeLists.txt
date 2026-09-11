# soc/CMakeLists.txt
# Platform bring-up (D1): the SoC component owns startup/linker/system/hal_conf
# and the common bring-up code. All SoC facts come from cmake/helpers/soc.cmake (R3).

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/helpers/soc.cmake)

set(SOC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ─── SoC definition ──────────────────────────────────────────────────────────
if(PLATFORM MATCHES "^stm32")
    if(NOT DEFINED MCU_PART OR MCU_PART STREQUAL "")
        message(FATAL_ERROR "soc: MCU_PART is not set (mcu.part in the board config)")
    endif()
    abl_soc_resolve("${MCU_PART}" ABL_SOC)
    # Common bring-up + SoC-specific clock (R3)
    set(SOC_SOURCES
        ${SOC_DIR}/stm32/stm32_target_init.c
        ${ABL_SOC_DIR}/clock.c
    )
    set(SOC_HAL_CONF_DIR  "${ABL_SOC_HAL_CONF_DIR}")
    set(SOC_DEVICE        "${ABL_SOC_DEVICE}")
    set(SOC_HAL_PREFIX    "${ABL_SOC_HAL_PREFIX}")
    set(SOC_STARTUP_FILE  "${ABL_SOC_STARTUP}")
    set(SOC_LINKER_SCRIPT "${ABL_SOC_LINKER}")
    set(SOC_SYSTEM_FILE   "${ABL_SOC_SYSTEM}")
elseif(PLATFORM STREQUAL "avr")
    if(NOT DEFINED F_CPU_HZ OR F_CPU_HZ STREQUAL "")
        message(FATAL_ERROR "soc: F_CPU_HZ is not set (oscillator.freq_hz in the board config)")
    endif()
    set(SOC_SOURCES ${SOC_DIR}/avr/avr_target_init.c)
    set(SOC_HAL_CONF_DIR "")
    set(SOC_DEVICE "")
    set(SOC_HAL_PREFIX "")
    set(SOC_STARTUP_FILE "")
    set(SOC_LINKER_SCRIPT "")
    set(SOC_SYSTEM_FILE "")
elseif(PLATFORM STREQUAL "esp32")
    # TODO (Step 9): the ESP32 port moves to the ESP-IDF wrapper.
    set(SOC_SOURCES ${SOC_DIR}/esp32/esp32_target_init.c)
    set(SOC_HAL_CONF_DIR "")
    set(SOC_DEVICE "")
    set(SOC_HAL_PREFIX "")
    set(SOC_STARTUP_FILE "")
    set(SOC_LINKER_SCRIPT "")
    set(SOC_SYSTEM_FILE "")
else()
    message(FATAL_ERROR "soc: unsupported platform: ${PLATFORM}")
endif()

# Exported as cache variables because add_firmware_target() runs in the
# project's directory scope. Device define / hal_conf / SDK paths are NOT
# exported here: they travel through the soc component's PUBLIC interface.
set(SOC_STARTUP_FILE  "${SOC_STARTUP_FILE}"  CACHE FILEPATH "SoC startup file" FORCE)
set(SOC_LINKER_SCRIPT "${SOC_LINKER_SCRIPT}" CACHE FILEPATH "SoC linker script" FORCE)
set(SOC_SYSTEM_FILE   "${SOC_SYSTEM_FILE}"   CACHE FILEPATH "SoC system file" FORCE)
set(SOC_HAL_PREFIX    "${SOC_HAL_PREFIX}"    CACHE STRING  "SoC HAL source prefix" FORCE)

# ─── Component: soc ──────────────────────────────────────────────────────────
# The PUBLIC interface carries everything the HAL port and the application need:
# contract headers, the vendor header shim (soc_hal.h), hal_conf, SDK includes,
# the device define and USE_HAL_DRIVER.
if(PLATFORM MATCHES "^stm32")
    abl_component(soc
        SOURCES  ${SOC_SOURCES}
        INCLUDES ${SOC_DIR}/include
                 ${SOC_HAL_CONF_DIR}
                 ${STM32_INCLUDE_DIRS}
        DEFINES  ${SOC_DEVICE}
                 USE_HAL_DRIVER
    )
elseif(PLATFORM STREQUAL "avr")
    # F_CPU is exported PUBLIC: the HAL port and the application need it for
    # _delay_* and for any code timing based on the CPU clock.
    abl_component(soc
        SOURCES  ${SOC_SOURCES}
        INCLUDES ${SOC_DIR}/include
        DEFINES  F_CPU=${F_CPU_HZ}
    )
elseif(PLATFORM STREQUAL "esp32")
    abl_component(soc
        SOURCES         ${SOC_SOURCES}
        INCLUDES        ${SOC_DIR}/include
        PRIVATE_DEFINES PLATFORM_ESP32
    )
endif()
