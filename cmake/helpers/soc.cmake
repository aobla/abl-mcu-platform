# cmake/helpers/soc.cmake
#
# Индекс SoC: mcu.part → каталог SoC-дефиниции (единственный источник, R3).
# Сама дефиниция (device / core / fpu / файлы bring-up) лежит в
# soc/<family>/<variant>/soc.cmake и подключается только для выбранного part.
#
# Добавить новый МК:
#   1) каталог soc/<family>/<variant>/ с soc.cmake и файлами bring-up;
#   2) одна строка в ABL_SOC_INDEX ниже.
#
# Резолвер можно вызывать до project() (нужно для выбора тулчейна) и после.

set(ABL_SOC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../soc")

# part=каталог относительно soc/
set(ABL_SOC_INDEX
    "STM32F103C6T6=stm32/f1/STM32F103x6"
    "STM32F103C8T6=stm32/f1/STM32F103x6"
    "STM32H743VIT6=stm32/h7/STM32H743xx"
    "STM32H743ZIT6=stm32/h7/STM32H743xx"
)

# abl_soc_resolve(<part> <out_prefix>)
# Устанавливает в вызывающей области видимости:
#   <prefix>_DIR <prefix>_PART <prefix>_FAMILY <prefix>_DEVICE <prefix>_HAL_PREFIX
#   <prefix>_CORE <prefix>_FPU <prefix>_FLOAT_ABI
#   <prefix>_STARTUP <prefix>_LINKER <prefix>_SYSTEM <prefix>_HAL_CONF_DIR
function(abl_soc_resolve part out_prefix)
    if(NOT part)
        message(FATAL_ERROR "abl_soc_resolve: mcu.part не задан")
    endif()

    set(_variant "")
    foreach(_entry ${ABL_SOC_INDEX})
        if(_entry MATCHES "^${part}=(.*)$")
            set(_variant "${CMAKE_MATCH_1}")
            break()
        endif()
    endforeach()

    if(NOT _variant)
        message(FATAL_ERROR "abl_soc_resolve: неизвестный mcu.part '${part}'. "
                            "Добавьте SoC-дефиницию и строку в ABL_SOC_INDEX "
                            "(cmake/helpers/soc.cmake).")
    endif()

    set(_soc_dir "${ABL_SOC_ROOT}/${_variant}")
    if(NOT EXISTS "${_soc_dir}/soc.cmake")
        message(FATAL_ERROR "abl_soc_resolve: нет дефиниции ${_soc_dir}/soc.cmake")
    endif()

    # Дефиниция подключается во внутренней области видимости функции
    include("${_soc_dir}/soc.cmake")

    set(${out_prefix}_DIR          "${_soc_dir}"                      PARENT_SCOPE)
    set(${out_prefix}_PART         "${part}"                          PARENT_SCOPE)
    set(${out_prefix}_FAMILY       "${ABL_SOC_FAMILY}"                PARENT_SCOPE)
    set(${out_prefix}_DEVICE       "${ABL_SOC_DEVICE}"                PARENT_SCOPE)
    set(${out_prefix}_HAL_PREFIX   "${ABL_SOC_HAL_PREFIX}"            PARENT_SCOPE)
    set(${out_prefix}_CORE         "${ABL_SOC_CORE}"                  PARENT_SCOPE)
    set(${out_prefix}_FPU          "${ABL_SOC_FPU}"                   PARENT_SCOPE)
    set(${out_prefix}_FLOAT_ABI    "${ABL_SOC_FLOAT_ABI}"             PARENT_SCOPE)
    set(${out_prefix}_STARTUP      "${_soc_dir}/${ABL_SOC_STARTUP}"   PARENT_SCOPE)
    set(${out_prefix}_LINKER       "${_soc_dir}/${ABL_SOC_LINKER}"    PARENT_SCOPE)
    set(${out_prefix}_SYSTEM       "${_soc_dir}/${ABL_SOC_SYSTEM}"    PARENT_SCOPE)
    set(${out_prefix}_HAL_CONF_DIR "${_soc_dir}"                      PARENT_SCOPE)
endfunction()
