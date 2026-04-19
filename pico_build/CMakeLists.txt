cmake_minimum_required(VERSION 3.16)

include(pico_sdk_import.cmake)

if(NOT DEFINED PICO_BOARD)
    if(EXISTS "${PICO_SDK_PATH}/src/boards/include/boards/pimoroni_plasma2350.h")
        set(PICO_BOARD pimoroni_plasma2350 CACHE STRING "Pico SDK board type")
    else()
        set(PICO_BOARD pico2 CACHE STRING "Pico SDK board type")
    endif()
endif()

project(tasbot_eyes_pico C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

find_program(POWERSHELL_EXECUTABLE NAMES powershell.exe pwsh REQUIRED)

set(TASBOT_GENERATED_ASSET_DIR ${CMAKE_CURRENT_LIST_DIR}/assets/generated)
set(TASBOT_ANIMATION_REGISTRY_HEADER ${TASBOT_GENERATED_ASSET_DIR}/animation_registry.generated.h)

add_custom_target(generate_all_assets
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TASBOT_GENERATED_ASSET_DIR}
    COMMAND ${POWERSHELL_EXECUTABLE}
        -NoProfile
        -ExecutionPolicy Bypass
        -File ${CMAKE_CURRENT_LIST_DIR}/tools/generate-animation-registry.ps1
        -ProjectRoot ${CMAKE_CURRENT_LIST_DIR}
        -OutputDir ${TASBOT_GENERATED_ASSET_DIR}
    BYPRODUCTS
        ${TASBOT_ANIMATION_REGISTRY_HEADER}
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/tools/generate-animation-registry.ps1
        ${CMAKE_CURRENT_LIST_DIR}/tools/generate-gif-asset.ps1
    COMMENT "Generating all embedded animation assets and registry"
    VERBATIM
)

add_executable(tasbot_eyes_pico
    src/firmware/animation_registry.c
    src/firmware/main.c
    src/firmware/hw_led_pio.c
    src/portable/embedded_animation.c
    src/portable/tasbot_layout.c
    src/portable/smoke_patterns.c
)

add_dependencies(tasbot_eyes_pico generate_all_assets)

target_include_directories(tasbot_eyes_pico PRIVATE
    ${TASBOT_GENERATED_ASSET_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src/firmware
    ${CMAKE_CURRENT_LIST_DIR}/src/portable
)

target_link_libraries(tasbot_eyes_pico
    pico_stdlib
    hardware_clocks
    hardware_pio
)

pico_enable_stdio_usb(tasbot_eyes_pico 1)
pico_enable_stdio_uart(tasbot_eyes_pico 0)

pico_add_extra_outputs(tasbot_eyes_pico)

message(STATUS "Configured tasbot_eyes_pico for board '${PICO_BOARD}'")
