# cmake/toolchains/avr-gcc.cmake
# Тулчейн для AVR с использованием GCC

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

# Bare-metal: the compiler check must not try to link a host executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ─── Toolchain resolution (priority order) ───────────────────────────────────
# 1. ABL_TOOLCHAIN_PATH env var
# 2. ~/.local/share/abl-mcu-toolchains/avr/current
# 3. System PATH (default)

if(DEFINED ENV{ABL_TOOLCHAIN_PATH})
    set(TOOLCHAIN_PREFIX "$ENV{ABL_TOOLCHAIN_PATH}/bin")
elseif(EXISTS "$ENV{HOME}/.local/share/abl-mcu-toolchains/avr/current/bin/avr-gcc")
    set(TOOLCHAIN_PREFIX "$ENV{HOME}/.local/share/abl-mcu-toolchains/avr/current/bin")
else()
    set(TOOLCHAIN_PREFIX "")
endif()

if(TOOLCHAIN_PREFIX)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}/avr-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}/avr-g++)
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}/avr-gcc)
    set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}/avr-objcopy)
    set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}/avr-size)
else()
    set(CMAKE_C_COMPILER avr-gcc)
    set(CMAKE_CXX_COMPILER avr-g++)
    set(CMAKE_ASM_COMPILER avr-gcc)
    set(CMAKE_OBJCOPY avr-objcopy)
    set(CMAKE_SIZE avr-size)
endif()

# Флаги компиляции для AVR
# Если не определен микроконтроллер, используем значение по умолчанию
if(NOT DEFINED AVR_MCU)
    set(AVR_MCU "atmega328p")  # можно изменить на нужный микроконтроллер
endif()

set(MCU_FLAG "-mmcu=${AVR_MCU}")

set(COMMON_FLAGS "${MCU_FLAG} -Wall -g2 -Os -fsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")

set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -std=gnu++17 -fno-rtti -fno-exceptions -fno-use-cxa-atexit")
set(CMAKE_ASM_FLAGS_INIT "${COMMON_FLAGS} -x assembler-with-cpp")
# No custom linker script: avr-gcc selects the device default from -mmcu.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-gc-sections,--print-memory-usage -Wl,-Map=${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.map")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)