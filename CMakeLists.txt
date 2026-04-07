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

    # Generated files are created by the project CMakeLists.txt before this call
    set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)

    # ─── Firmware target ─────────────────────────────────────────────────────
    add_executable(${FW_NAME}
        ${FW_SOURCES}
        ${GEN_DIR}/generated_gpio_init.c
        ${GEN_DIR}/hardware_config.h
        ${GEN_DIR}/hardware_pins.h
    )

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

    # Разрешаем платформу в тулчейн-файл (для справки)
    # Тулчейн уже включён в проекте ДО project(), параметры MCU заданы там

    # Линкер-флаги (после генерации скрипта)
    set_target_properties(${FW_NAME} PROPERTIES
        LINK_FLAGS "-Wl,-gc-sections,--print-memory-usage -T ${GEN_DIR}/linker_script.ld -Wl,-Map=${CMAKE_BINARY_DIR}/${FW_NAME}.map"
    )
endfunction()