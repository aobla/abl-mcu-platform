# hal/src/stm32/CMakeLists.txt
# Сборка HAL для STM32

file(GLOB_RECURSE STM32_HAL_SOURCES *.c)

add_library(hal_stm32 STATIC ${STM32_HAL_SOURCES})

target_include_directories(hal_stm32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# STM32 HAL SDK пути устанавливаются из тулчейна
if(DEFINED STM32_INCLUDE_DIRS)
    target_include_directories(hal_stm32 PUBLIC ${STM32_INCLUDE_DIRS})
endif()

# Добавляем исходники STM32 HAL драйвера из SDK
if(DEFINED STM32_SDK_ROOT AND IS_DIRECTORY "${STM32_SDK_ROOT}/Src")
    # Минимальный набор: core + GPIO + RCC + Cortex
    set(STM32_HAL_SDK_SOURCES
        ${STM32_SDK_ROOT}/Src/stm32f1xx_hal.c
        ${STM32_SDK_ROOT}/Src/stm32f1xx_hal_gpio.c
        ${STM32_SDK_ROOT}/Src/stm32f1xx_hal_rcc.c
        ${STM32_SDK_ROOT}/Src/stm32f1xx_hal_rcc_ex.c
        ${STM32_SDK_ROOT}/Src/stm32f1xx_hal_cortex.c
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
