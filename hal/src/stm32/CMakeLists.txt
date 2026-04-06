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