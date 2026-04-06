# hal/src/stm32/CMakeLists.txt
# Сборка HAL для STM32

file(GLOB_RECURSE STM32_HAL_SOURCES *.c)

add_library(hal_stm32 STATIC ${STM32_HAL_SOURCES})

target_include_directories(hal_stm32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# STM32 HAL драйвер подключается через внешние SDK (CMSIS/STM32Cube)
# Пользователь должен добавить include пути через STM32_INCLUDE_DIRS
# или через переменную окружения/кеша
if(DEFINED STM32_INCLUDE_DIRS)
    target_include_directories(hal_stm32 PUBLIC ${STM32_INCLUDE_DIRS})
endif()

# Компилятор найдёт заголовки STM32 HAL если они установлены в системе
# или доступны через TOOLCHAIN_PATH