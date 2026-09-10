cmake_minimum_required(VERSION 3.20)
project(abl-mcu-platform VERSION 1.0.0)

include(cmake/helpers/version_check.cmake)
include(cmake/helpers/target_helpers.cmake)

# Bring-up (target_init) + SoC-provided startup/linker/system/hal_conf (D1)
add_subdirectory(soc)

# HAL ports (STM32 использует SOC_HAL_CONF_DIR из soc)
add_subdirectory(hal)

# Функция для создания прошивки
function(add_firmware_target)
    set(options "")
    set(oneValueArgs NAME PLATFORM CONFIG STARTUP_FILE LINKER_SCRIPT SYSTEM_FILE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(FW "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Generated files are created by the project CMakeLists.txt before this call
    set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)

    # ─── Firmware target ─────────────────────────────────────────────────────
    set(FW_ALL_SOURCES
        ${FW_SOURCES}
        ${GEN_DIR}/generated_gpio_init.c
        ${GEN_DIR}/hardware_config.h
        ${GEN_DIR}/hardware_pins.h
    )

    # Bring-up files берём из SoC-дефиниции по умолчанию; проект может переопределить
    set(_STARTUP_FILE "${FW_STARTUP_FILE}")
    if(NOT _STARTUP_FILE)
        set(_STARTUP_FILE "${SOC_STARTUP_FILE}")
    endif()
    set(_LINKER_SCRIPT "${FW_LINKER_SCRIPT}")
    if(NOT _LINKER_SCRIPT)
        set(_LINKER_SCRIPT "${SOC_LINKER_SCRIPT}")
    endif()
    set(_SYSTEM_FILE "${FW_SYSTEM_FILE}")
    if(NOT _SYSTEM_FILE)
        set(_SYSTEM_FILE "${SOC_SYSTEM_FILE}")
    endif()

    # Add startup file if provided
    if(_STARTUP_FILE AND EXISTS "${_STARTUP_FILE}")
        list(APPEND FW_ALL_SOURCES "${_STARTUP_FILE}")
        set_source_files_properties("${_STARTUP_FILE}" PROPERTIES LANGUAGE ASM)
    endif()

    # Add system file if provided
    if(_SYSTEM_FILE AND EXISTS "${_SYSTEM_FILE}")
        list(APPEND FW_ALL_SOURCES "${_SYSTEM_FILE}")
        # System file needs device defines (e.g. STM32F103xB)
        if(FW_PLATFORM MATCHES "^stm32f103")
            set(SYS_DEVICE "STM32F103x6")
        elseif(FW_PLATFORM MATCHES "^stm32f4")
            set(SYS_DEVICE "STM32F407xx")
        elseif(FW_PLATFORM MATCHES "^stm32h7")
            set(SYS_DEVICE "STM32H743xx")
        else()
            set(SYS_DEVICE "STM32F407xx")
        endif()
        set_source_files_properties("${_SYSTEM_FILE}" PROPERTIES
            COMPILE_DEFINITIONS "${SYS_DEVICE};USE_HAL_DRIVER"
        )
    endif()

    add_executable(${FW_NAME} ${FW_ALL_SOURCES})

    add_dependencies(${FW_NAME} generated_files)

    target_include_directories(${FW_NAME} PRIVATE ${GEN_DIR})
    target_include_directories(${FW_NAME} PRIVATE
        ${platformcore_SOURCE_DIR}/hal/include
        ${platformcore_SOURCE_DIR}/soc/include
    )

    # SoC-provided HAL config (stm32f1xx_hal_conf.h и т.п.)
    if(DEFINED SOC_HAL_CONF_DIR AND SOC_HAL_CONF_DIR)
        target_include_directories(${FW_NAME} PRIVATE ${SOC_HAL_CONF_DIR})
    endif()

    # SDK include paths (from toolchain)
    if(DEFINED STM32_INCLUDE_DIRS)
        target_include_directories(${FW_NAME} PRIVATE ${STM32_INCLUDE_DIRS})
    endif()

    # Device define for STM32 HAL (required by stm32Xxxxx.h)
    if(FW_PLATFORM MATCHES "^stm32f103")
        target_compile_definitions(${FW_NAME} PRIVATE STM32F103x6)
    elseif(FW_PLATFORM MATCHES "^stm32f4")
        target_compile_definitions(${FW_NAME} PRIVATE STM32F407xx)
    elseif(FW_PLATFORM MATCHES "^stm32h743")
        target_compile_definitions(${FW_NAME} PRIVATE STM32H743xx)
    endif()

    # Добавляем библиотеку инициализации целевой платформы
    target_link_libraries(${FW_NAME} PRIVATE target_init)

    # Link HAL library for the platform
    if(FW_PLATFORM MATCHES "^stm32")
        target_link_libraries(${FW_NAME} PRIVATE hal_stm32)
    elseif(FW_PLATFORM STREQUAL "esp32")
        target_link_libraries(${FW_NAME} PRIVATE hal_esp32)
    elseif(FW_PLATFORM STREQUAL "avr")
        target_link_libraries(${FW_NAME} PRIVATE hal_avr)
    endif()

    # Линкер-флаги
    set(LINKER_FLAGS "-Wl,-gc-sections,--print-memory-usage") #-O3
    if(_LINKER_SCRIPT AND EXISTS "${_LINKER_SCRIPT}")
        set(LINKER_FLAGS "${LINKER_FLAGS} -T ${_LINKER_SCRIPT}")
    endif()
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-Map=${CMAKE_BINARY_DIR}/${FW_NAME}.map")
    set_target_properties(${FW_NAME} PROPERTIES LINK_FLAGS "${LINKER_FLAGS}")
endfunction()
