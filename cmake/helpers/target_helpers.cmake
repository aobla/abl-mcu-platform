# cmake/helpers/target_helpers.cmake
# Вспомогательные функции для создания целей сборки

# Функция для добавления библиотеки HAL для конкретной платформы
function(add_hal_platform_library platform_name)
    set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/hal/src/${platform_name})
    if(EXISTS ${platform_dir})
        file(GLOB_RECURSE PLATFORM_SOURCES ${platform_dir}/*.c)
        add_library(hal_${platform_name} STATIC ${PLATFORM_SOURCES})
        target_include_directories(hal_${platform_name} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/hal/include
            ${platform_dir}
        )
    else()
        message(WARNING "Platform directory ${platform_dir} does not exist")
    endif()
endfunction()

# Функция для выбора активной платформы
function(set_active_platform platform_name)
    add_hal_platform_library(${platform_name})
    if(TARGET main)
        target_link_libraries(main hal_${platform_name})
    endif()
endfunction()

# Функция для генерации линкер-скрипта из шаблона
function(generate_linker_script template_path output_path)
    if(EXISTS ${template_path})
        configure_file(${template_path} ${output_path} @ONLY)
    else()
        message(WARNING "Linker script template ${template_path} does not exist")
    endif()
endfunction()

# Функция для добавления опций компиляции в зависимости от платформы
function(set_platform_compile_options target_name platform_name)
    if(${platform_name} STREQUAL "stm32")
        target_compile_definitions(${target_name} PRIVATE PLATFORM_STM32)
    elseif(${platform_name} STREQUAL "esp32")
        target_compile_definitions(${target_name} PRIVATE PLATFORM_ESP32)
    elseif(${platform_name} STREQUAL "avr")
        target_compile_definitions(${target_name} PRIVATE PLATFORM_AVR)
    endif()
endfunction()