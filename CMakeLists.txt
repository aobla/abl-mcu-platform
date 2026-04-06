cmake_minimum_required(VERSION 3.20)
project(abl-mcu-platform-core VERSION 1.0.0)

include(cmake/helpers/version_check.cmake)
include(cmake/helpers/target_helpers.cmake)

# Добавляем HAL и Target в сборку
add_subdirectory(hal)
add_subdirectory(target)

# Функция для создания прошивки
function(add_firmware_target)
    set(options "")
    set(oneValueArgs NAME PLATFORM CONFIG PINMUX)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(FW "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ─── Generated files ──────────────────────────────────────────────────────
    set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)
    file(MAKE_DIRECTORY ${GEN_DIR})

    # hardware_config.h
    add_custom_command(
        OUTPUT ${GEN_DIR}/hardware_config.h
        COMMAND python3 ${platformcore_SOURCE_DIR}/scripts/gen_config_v1.0.py ${FW_CONFIG}
        DEPENDS ${FW_CONFIG}
                ${platformcore_SOURCE_DIR}/scripts/gen_config_v1.0.py
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating hardware_config.h from ${FW_CONFIG}"
    )

    # hardware_pins.h + generated_gpio_init.c
    if(DEFINED FW_PINMUX)
        set(PINMUX_INPUT ${FW_PINMUX})
    else()
        set(PINMUX_INPUT ${FW_CONFIG})
    endif()

    add_custom_command(
        OUTPUT ${GEN_DIR}/hardware_pins.h ${GEN_DIR}/generated_gpio_init.c
        COMMAND python3 ${platformcore_SOURCE_DIR}/scripts/gen_pinmux_v1.0.py ${PINMUX_INPUT}
        DEPENDS ${PINMUX_INPUT}
                ${platformcore_SOURCE_DIR}/scripts/gen_pinmux_v1.0.py
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating pinmux from ${PINMUX_INPUT}"
    )

    # linker_script.ld
    set(LINKER_OUTPUT ${GEN_DIR}/linker_script.ld)
    if(EXISTS ${platformcore_SOURCE_DIR}/templates/linker_script.ld.jinja)
        add_custom_command(
            OUTPUT ${LINKER_OUTPUT}
            COMMAND python3 ${platformcore_SOURCE_DIR}/scripts/gen_linker_v1.0.py ${FW_CONFIG}
            DEPENDS ${FW_CONFIG}
                    ${platformcore_SOURCE_DIR}/scripts/gen_linker_v1.0.py
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating linker script from ${FW_CONFIG}"
        )
    endif()

    # ─── Firmware target ─────────────────────────────────────────────────────
    add_executable(${FW_NAME}
        ${FW_SOURCES}
        ${GEN_DIR}/generated_gpio_init.c
        ${GEN_DIR}/hardware_config.h
        ${GEN_DIR}/hardware_pins.h
    )

    target_include_directories(${FW_NAME} PRIVATE ${GEN_DIR})
    target_include_directories(${FW_NAME} PRIVATE
        ${platformcore_SOURCE_DIR}/hal/include
        ${platformcore_SOURCE_DIR}/target/common
    )

    # Добавляем библиотеку инициализации целевой платформы
    target_link_libraries(${FW_NAME} PRIVATE target_init)

    # Применяем тулчейн
    include(${platformcore_SOURCE_DIR}/cmake/toolchains/${FW_PLATFORM}.cmake)

    # Устанавливаем опции для тулчейна, если они определены в конфиге
    if(DEFINED STM32_MCU)
        set(STM32_MCU ${STM32_MCU} CACHE STRING "STM32 Microcontroller")
    endif()
    if(DEFINED STM32_FPU)
        set(STM32_FPU ${STM32_FPU} CACHE STRING "STM32 FPU Type")
    endif()
    if(DEFINED STM32_FLOAT_ABI)
        set(STM32_FLOAT_ABI ${STM32_FLOAT_ABI} CACHE STRING "STM32 Float ABI")
    endif()
    if(DEFINED AVR_MCU)
        set(AVR_MCU ${AVR_MCU} CACHE STRING "AVR Microcontroller")
    endif()
endfunction()