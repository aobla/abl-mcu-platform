# drivers/CMakeLists.txt
# Portable drivers (L3). Auto-discovery (D4): adding a driver = adding a directory
# with its own CMakeLists.txt that calls abl_component(); this file needs no edits.
#
# Every discovered driver is configured as a static library, but only the ones
# linked by the project end up in the firmware (unused code is dropped by
# -gc-sections). When the catalog grows, switch to config-driven selection.

file(GLOB _driver_dirs CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(_dir ${_driver_dirs})
    if(IS_DIRECTORY "${_dir}" AND EXISTS "${_dir}/CMakeLists.txt")
        add_subdirectory("${_dir}")
    endif()
endforeach()
