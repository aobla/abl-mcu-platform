# cmake/toolchains/esp32-clang.cmake
# Тулчейн для ESP32 с использованием Clang

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR xtensa)

# ─── Toolchain resolution (priority order) ───────────────────────────────────
# 1. ABL_TOOLCHAIN_PATH env var
# 2. ~/.local/share/abl-mcu-toolchains/esp-idf/<version>/tools/xtensa-esp32-elf
# 3. $IDF_PATH env var (ESP-IDF standard)
# 4. System PATH (default)

if(DEFINED ENV{ABL_TOOLCHAIN_PATH})
    set(TOOLCHAIN_PREFIX "$ENV{ABL_TOOLCHAIN_PATH}/bin")
elseif(DEFINED ENV{IDF_PATH} AND EXISTS "$ENV{IDF_PATH}/tools/xtensa-esp32-elf/bin/xtensa-esp32-elf-gcc")
    set(TOOLCHAIN_PREFIX "$ENV{IDF_PATH}/tools/xtensa-esp32-elf/bin")
else()
    # Try to find ESP-IDF in managed toolchain directory
    file(GLOB ESP_IDF_DIRS "$ENV{HOME}/.local/share/abl-mcu-toolchains/esp-idf/*")
    set(TOOLCHAIN_PREFIX "")
    foreach(dir ${ESP_IDF_DIRS})
        if(IS_DIRECTORY "${dir}/tools/xtensa-esp32-elf/bin")
            set(TOOLCHAIN_PREFIX "${dir}/tools/xtensa-esp32-elf/bin")
            break()
        endif()
    endforeach()
endif()

if(TOOLCHAIN_PREFIX)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}/xtensa-esp32-elf-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}/xtensa-esp32-elf-g++)
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}/xtensa-esp32-elf-gcc)
    set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}/xtensa-esp32-elf-objcopy)
    set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}/xtensa-esp32-elf-size)
else()
    set(CMAKE_C_COMPILER xtensa-esp32-elf-gcc)
    set(CMAKE_CXX_COMPILER xtensa-esp32-elf-g++)
    set(CMAKE_ASM_COMPILER xtensa-esp32-elf-gcc)
    set(CMAKE_OBJCOPY xtensa-esp32-elf-objcopy)
    set(CMAKE_SIZE xtensa-esp32-elf-size)
endif()

# Флаги компиляции для ESP32
set(COMMON_FLAGS "-mlongcalls -Wno-frame-address -ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -std=gnu++17 -fno-rtti -fno-exceptions -fno-use-cxa-atexit")
set(CMAKE_ASM_FLAGS_INIT "${COMMON_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-gc-sections,--print-memory-usage -T ${CMAKE_BINARY_DIR}/generated/linker_script.ld -Wl,-Map=${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.map")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)