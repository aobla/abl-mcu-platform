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
    set(oneValueArgs NAME PLATFORM CONFIG PINMUX STARTUP_FILE LINKER_SCRIPT SYSTEM_FILE)
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

    # Add startup file if provided
    if(FW_STARTUP_FILE AND EXISTS "${FW_STARTUP_FILE}")
        list(APPEND FW_ALL_SOURCES "${FW_STARTUP_FILE}")
        set_source_files_properties("${FW_STARTUP_FILE}" PROPERTIES LANGUAGE ASM)
    endif()

    # Add system file if provided
    if(FW_SYSTEM_FILE AND EXISTS "${FW_SYSTEM_FILE}")
        list(APPEND FW_ALL_SOURCES "${FW_SYSTEM_FILE}")
        # System file needs device defines (e.g. STM32F103xB)
        if(FW_PLATFORM MATCHES "^stm32f1")
            set(SYS_DEVICE "STM32F103xB")
        elseif(FW_PLATFORM MATCHES "^stm32f4")
            set(SYS_DEVICE "STM32F407xx")
        elseif(FW_PLATFORM MATCHES "^stm32h7")
            set(SYS_DEVICE "STM32H743xx")
        else()
            set(SYS_DEVICE "STM32F407xx")
        endif()
        set_source_files_properties("${FW_SYSTEM_FILE}" PROPERTIES
            COMPILE_DEFINITIONS "${SYS_DEVICE};USE_HAL_DRIVER"
        )
    endif()

    add_executable(${FW_NAME} ${FW_ALL_SOURCES})

    add_dependencies(${FW_NAME} generated_files)

    target_include_directories(${FW_NAME} PRIVATE ${GEN_DIR})
    target_include_directories(${FW_NAME} PRIVATE
        ${platformcore_SOURCE_DIR}/hal/include
        ${platformcore_SOURCE_DIR}/target/common
    )

    # SDK include paths (from toolchain)
    if(DEFINED STM32_INCLUDE_DIRS)
        target_include_directories(${FW_NAME} PRIVATE ${STM32_INCLUDE_DIRS})
    endif()

    # Project HAL config (e.g. stm32f1xx_hal_conf.h)
    if(DEFINED HAL_CONFIG_DIR)
        target_include_directories(${FW_NAME} PRIVATE ${HAL_CONFIG_DIR})
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
    if(FW_LINKER_SCRIPT AND EXISTS "${FW_LINKER_SCRIPT}")
        set(LINKER_FLAGS "${LINKER_FLAGS} -T ${FW_LINKER_SCRIPT}")
    endif()
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-Map=${CMAKE_BINARY_DIR}/${FW_NAME}.map")
    set_target_properties(${FW_NAME} PROPERTIES LINK_FLAGS "${LINKER_FLAGS}")
endfunction()