# cmake/helpers/component.cmake
#
# abl_component() — the single way to define a platform module (D4, R7).
#
# Creates a static library `abl_<name>` plus alias `abl::<name>` with an explicit
# public surface: only the listed INCLUDES/DEFINES are visible to dependents,
# everything else stays private to the component.
#
# Usage:
#   abl_component(<name>
#       [SOURCES          <src>...]      # omit for an INTERFACE (header-only) component
#       [INCLUDES         <dir>...]      # PUBLIC include dirs (dependents see them)
#       [PRIVATE_INCLUDES <dir>...]      # PRIVATE include dirs
#       [DEPS             <target>...]   # other components, e.g. abl::hal
#       [DEFINES          <def>...]      # PUBLIC compile definitions
#       [PRIVATE_DEFINES  <def>...]      # PRIVATE compile definitions
#   )
#
# Rules (ARCHITECTURE.md §12):
#   - a component owns its public headers (by convention under <dir>/include/);
#   - dependencies are declared explicitly, cycles are forbidden;
#   - vendor SDK headers/sources are allowed only inside hal ports and soc.
#
# Adding a module = new directory + one abl_component() call. No other CMake
# file has to be touched.

function(abl_component name)
    cmake_parse_arguments(
        C
        ""
        ""
        "SOURCES;INCLUDES;PRIVATE_INCLUDES;DEPS;DEFINES;PRIVATE_DEFINES"
        ${ARGN}
    )

    # INTERFACE libraries have no PRIVATE scope: use INTERFACE there, PUBLIC otherwise.
    if(C_SOURCES)
        add_library(abl_${name} STATIC ${C_SOURCES})
        set(_abl_scope PUBLIC)
    else()
        add_library(abl_${name} INTERFACE)
        set(_abl_scope INTERFACE)
    endif()

    add_library(abl::${name} ALIAS abl_${name})

    if(C_INCLUDES)
        target_include_directories(abl_${name} ${_abl_scope} ${C_INCLUDES})
    endif()

    if(C_DEFINES)
        target_compile_definitions(abl_${name} ${_abl_scope} ${C_DEFINES})
    endif()

    # Source-less components are INTERFACE libraries: no PRIVATE properties.
    if(C_SOURCES)
        if(C_PRIVATE_INCLUDES)
            target_include_directories(abl_${name} PRIVATE ${C_PRIVATE_INCLUDES})
        endif()
        if(C_PRIVATE_DEFINES)
            target_compile_definitions(abl_${name} PRIVATE ${C_PRIVATE_DEFINES})
        endif()
    endif()

    if(C_DEPS)
        target_link_libraries(abl_${name} ${_abl_scope} ${C_DEPS})
    endif()
endfunction()
