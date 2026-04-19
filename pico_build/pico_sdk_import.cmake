if(NOT PICO_SDK_PATH AND DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
endif()

if(NOT PICO_SDK_PATH)
    get_filename_component(
        _tasbot_default_pico_sdk
        "${CMAKE_CURRENT_LIST_DIR}/../../pico-sdk"
        REALPATH
        BASE_DIR "${CMAKE_CURRENT_LIST_DIR}"
    )

    if(EXISTS "${_tasbot_default_pico_sdk}/pico_sdk_init.cmake")
        set(PICO_SDK_PATH "${_tasbot_default_pico_sdk}")
    endif()
endif()

set(PICO_SDK_PATH "${PICO_SDK_PATH}" CACHE PATH "Path to the Raspberry Pi Pico SDK")

if(NOT PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Set the cache entry or environment variable before configuring pico_build\\.")
endif()

get_filename_component(PICO_SDK_PATH "${PICO_SDK_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

if(NOT EXISTS "${PICO_SDK_PATH}/pico_sdk_init.cmake")
    message(FATAL_ERROR "PICO_SDK_PATH='${PICO_SDK_PATH}' does not contain pico_sdk_init.cmake.")
endif()

include("${PICO_SDK_PATH}/pico_sdk_init.cmake")
