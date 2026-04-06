# hal/CMakeLists.txt
# Сборка HAL для выбранной платформы

# Определяем платформу из переменной PLATFORM
if(PLATFORM MATCHES "^stm32.*")
    set(HAL_PLATFORM "stm32")
elseif(PLATFORM STREQUAL "esp32")
    set(HAL_PLATFORM "esp32")
elseif(PLATFORM STREQUAL "avr")
    set(HAL_PLATFORM "avr")
else()
    message(FATAL_ERROR "Unsupported HAL platform: ${PLATFORM}. Supported: stm32*, esp32, avr")
endif()

add_subdirectory(src/${HAL_PLATFORM})

# Создаем псевдоним для активной платформы
add_library(hal::active ALIAS hal_${HAL_PLATFORM})