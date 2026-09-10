# hal/src/stm32/CMakeLists.txt
# Сборка HAL для STM32

file(GLOB_RECURSE STM32_HAL_SOURCES *.c)

add_library(hal_stm32 STATIC ${STM32_HAL_SOURCES})

target_include_directories(hal_stm32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# SoC-provided HAL config (stm32f1xx_hal_conf.h и т.п.)
if(DEFINED SOC_HAL_CONF_DIR AND SOC_HAL_CONF_DIR)
    target_include_directories(hal_stm32 PUBLIC ${SOC_HAL_CONF_DIR})
endif()

# STM32 HAL SDK пути устанавливаются из тулчейна
if(DEFINED STM32_INCLUDE_DIRS)
    target_include_directories(hal_stm32 PUBLIC ${STM32_INCLUDE_DIRS})
endif()

# Platform-specific settings берутся из SoC-дефиниции (R3)
if(NOT DEFINED SOC_HAL_PREFIX OR SOC_HAL_PREFIX STREQUAL "")
    message(FATAL_ERROR "hal_stm32: SOC_HAL_PREFIX не задан — нет SoC-дефиниции для mcu.part=${MCU_PART}")
endif()
set(HAL_PREFIX "${SOC_HAL_PREFIX}")
if(DEFINED SOC_DEVICE AND SOC_DEVICE)
    target_compile_definitions(hal_stm32 PRIVATE ${SOC_DEVICE})
endif()

# Добавляем исходники STM32 HAL драйвера из SDK
if(DEFINED STM32_SDK_ROOT AND IS_DIRECTORY "${STM32_SDK_ROOT}/Src")
    # Минимальный набор: core + GPIO + RCC + Cortex
    set(STM32_HAL_SDK_SOURCES
        ${STM32_SDK_ROOT}/Src/${HAL_PREFIX}_hal.c
        ${STM32_SDK_ROOT}/Src/${HAL_PREFIX}_hal_gpio.c
        ${STM32_SDK_ROOT}/Src/${HAL_PREFIX}_hal_rcc.c
        ${STM32_SDK_ROOT}/Src/${HAL_PREFIX}_hal_rcc_ex.c
        ${STM32_SDK_ROOT}/Src/${HAL_PREFIX}_hal_cortex.c
    )

    # Добавляем только существующие файлы
    foreach(src ${STM32_HAL_SDK_SOURCES})
        if(EXISTS "${src}")
            target_sources(hal_stm32 PRIVATE "${src}")
        endif()
    endforeach()

    # Компилируем с нужными дефайнами
    target_compile_definitions(hal_stm32 PRIVATE USE_HAL_DRIVER)
endif()
