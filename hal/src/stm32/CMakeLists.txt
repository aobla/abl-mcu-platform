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

# Platform-specific settings
if(PLATFORM MATCHES "^stm32f103")
    set(HAL_PREFIX "stm32f1xx")
    target_compile_definitions(hal_stm32 PRIVATE STM32F103x6)
elseif(PLATFORM MATCHES "^stm32f4")
    set(HAL_PREFIX "stm32f4xx")
    target_compile_definitions(hal_stm32 PRIVATE STM32F407xx)
elseif(PLATFORM MATCHES "^stm32h743")
    set(HAL_PREFIX "stm32h7xx")
    target_compile_definitions(hal_stm32 PRIVATE STM32H743xx)
else()
    set(HAL_PREFIX "stm32f1xx")
    target_compile_definitions(hal_stm32 PRIVATE STM32F103x6)
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
