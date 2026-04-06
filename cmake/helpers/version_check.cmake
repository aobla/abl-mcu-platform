# cmake/helpers/version_check.cmake
# Функция для проверки версии platform-core

function(check_core_version min_version)
    if(DEFINED PROJECT_VERSION)
        if(PROJECT_VERSION VERSION_LESS min_version)
            message(FATAL_ERROR "abl-mcu-platform-core requires minimum version ${min_version}, but current version is ${PROJECT_VERSION}")
        endif()
    else()
        message(WARNING "abl-mcu-platform-core version not detected")
    endif()
endfunction()