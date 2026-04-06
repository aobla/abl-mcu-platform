# target/CMakeLists.txt
# Core предоставляет только include пути для target_init.h
# Реализация target_init() находится в проекте пользователя

add_library(platform_target INTERFACE)
target_include_directories(platform_target INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)
