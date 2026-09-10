# soc/CMakeLists.txt
# Подъём платформы (D1/D5): target_init + SoC-provided startup/linker/system/hal_conf.
# Все факты о SoC берутся из единственного источника — cmake/helpers/soc.cmake (R3).

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/helpers/soc.cmake)

set(SOC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ─── SoC-дефиниция ───────────────────────────────────────────────────────────
if(PLATFORM MATCHES "^stm32")
    if(NOT DEFINED MCU_PART OR MCU_PART STREQUAL "")
        message(FATAL_ERROR "soc: MCU_PART не задан (mcu.part в board-конфиге)")
    endif()
    abl_soc_resolve("${MCU_PART}" ABL_SOC)
    # Общий bring-up + конкретика SoC (тактирование) — из SoC-дефиниции (R3)
    set(SOC_TARGET_INIT_SRC
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
    # TODO (Шаг 8): avr_target_init.c всё ещё ссылается на app.h — чинить на Шаге 8.
    set(SOC_TARGET_INIT_SRC ${SOC_DIR}/avr/avr_target_init.c)
    set(SOC_HAL_CONF_DIR "")
    set(SOC_DEVICE "")
    set(SOC_HAL_PREFIX "")
    set(SOC_STARTUP_FILE "")
    set(SOC_LINKER_SCRIPT "")
    set(SOC_SYSTEM_FILE "")
elseif(PLATFORM STREQUAL "esp32")
    # TODO (Шаг 9): esp32-порт переводится на обёртку ESP-IDF.
    set(SOC_TARGET_INIT_SRC ${SOC_DIR}/esp32/esp32_target_init.c)
    set(SOC_HAL_CONF_DIR "")
    set(SOC_DEVICE "")
    set(SOC_HAL_PREFIX "")
    set(SOC_STARTUP_FILE "")
    set(SOC_LINKER_SCRIPT "")
    set(SOC_SYSTEM_FILE "")
else()
    message(FATAL_ERROR "soc: unsupported platform: ${PLATFORM}")
endif()

# Экспорт в вышестоящие области видимости (используют add_firmware_target и hal)
set(SOC_STARTUP_FILE  "${SOC_STARTUP_FILE}"  CACHE FILEPATH "SoC startup file" FORCE)
set(SOC_LINKER_SCRIPT "${SOC_LINKER_SCRIPT}" CACHE FILEPATH "SoC linker script" FORCE)
set(SOC_SYSTEM_FILE   "${SOC_SYSTEM_FILE}"   CACHE FILEPATH "SoC system file" FORCE)
set(SOC_DEVICE        "${SOC_DEVICE}"        CACHE STRING  "SoC device define" FORCE)
set(SOC_HAL_PREFIX    "${SOC_HAL_PREFIX}"    CACHE STRING  "SoC HAL file prefix" FORCE)
set(SOC_HAL_CONF_DIR  "${SOC_HAL_CONF_DIR}"  CACHE PATH    "SoC HAL config dir" FORCE)

# ─── target_init ─────────────────────────────────────────────────────────────
add_library(target_init STATIC ${SOC_TARGET_INIT_SRC})
target_include_directories(target_init PUBLIC
    ${SOC_DIR}/include
    ${SOC_DIR}/../hal/include
)
if(PLATFORM MATCHES "^stm32")
    target_include_directories(target_init PUBLIC ${SOC_HAL_CONF_DIR})
    if(DEFINED STM32_INCLUDE_DIRS)
        target_include_directories(target_init PUBLIC ${STM32_INCLUDE_DIRS})
    endif()
    # PLATFORM_* здесь не нужен: общий bring-up не знает о платформе (R2)
    target_compile_definitions(target_init PRIVATE
        USE_HAL_DRIVER ${SOC_DEVICE}
    )
elseif(PLATFORM STREQUAL "avr")
    target_compile_definitions(target_init PRIVATE PLATFORM_AVR)
elseif(PLATFORM STREQUAL "esp32")
    target_compile_definitions(target_init PRIVATE PLATFORM_ESP32)
endif()
