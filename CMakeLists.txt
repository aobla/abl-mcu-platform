cmake_minimum_required(VERSION 3.20)
project(abl-mcu-platform VERSION 1.0.0)

include(cmake/helpers/version_check.cmake)
include(cmake/helpers/component.cmake)

# Components (D4): soc (bring-up) -> hal (contracts + port) -> runtime -> drivers.
add_subdirectory(soc)
add_subdirectory(hal)
add_subdirectory(runtime)
add_subdirectory(drivers)

# ─── Host tests (D8) ─────────────────────────────────────────────────────────
# Standalone configure of the platform for PLATFORM=native:
#   cmake -S . -B build-tests -DPLATFORM=native -DABL_BUILD_TESTS=ON
#   cmake --build build-tests && ctest --test-dir build-tests
option(ABL_BUILD_TESTS "Build platform host tests (native only)" OFF)
if(ABL_BUILD_TESTS)
    if(NOT PLATFORM STREQUAL "native")
        message(FATAL_ERROR "ABL_BUILD_TESTS requires -DPLATFORM=native (got '${PLATFORM}')")
    endif()
    enable_testing()
    add_subdirectory(tests)
endif()

# ─── Firmware target ─────────────────────────────────────────────────────────
# Builds the firmware from the project sources + generated files and links the
# platform components. Include dirs, device defines and SDK paths are inherited
# transitively from the components' PUBLIC interface (D4, R7).
function(add_firmware_target)
    set(options "")
    set(oneValueArgs NAME PLATFORM STARTUP_FILE LINKER_SCRIPT SYSTEM_FILE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(FW "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Generated files are created by the project CMakeLists.txt before this call
    set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)

    set(FW_ALL_SOURCES
        ${FW_SOURCES}
        ${GEN_DIR}/generated_gpio_init.c
        ${GEN_DIR}/hardware_config.h
        ${GEN_DIR}/hardware_pins.h
    )

    # Bring-up files come from the SoC definition by default; a project may override
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

    if(_STARTUP_FILE AND EXISTS "${_STARTUP_FILE}")
        list(APPEND FW_ALL_SOURCES "${_STARTUP_FILE}")
        set_source_files_properties("${_STARTUP_FILE}" PROPERTIES LANGUAGE ASM)
    endif()

    if(_SYSTEM_FILE AND EXISTS "${_SYSTEM_FILE}")
        list(APPEND FW_ALL_SOURCES "${_SYSTEM_FILE}")
    endif()

    add_executable(${FW_NAME} ${FW_ALL_SOURCES})

    add_dependencies(${FW_NAME} generated_files)

    # Per-build generated headers
    target_include_directories(${FW_NAME} PRIVATE ${GEN_DIR})

    # Platform components: SoC bring-up + active HAL port + runtime backend (D6).
    # Drivers are linked by the project itself: target_link_libraries(... abl::<driver>)
    target_link_libraries(${FW_NAME} PRIVATE abl::soc abl::active abl::runtime)

    # Linker flags
    set(LINKER_FLAGS "-Wl,-gc-sections,--print-memory-usage")
    if(_LINKER_SCRIPT AND EXISTS "${_LINKER_SCRIPT}")
        set(LINKER_FLAGS "${LINKER_FLAGS} -T ${_LINKER_SCRIPT}")
    endif()
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-Map=${CMAKE_BINARY_DIR}/${FW_NAME}.map")
    set_target_properties(${FW_NAME} PROPERTIES LINK_FLAGS "${LINKER_FLAGS}")
endfunction()
