# Force RP2350/Plasma defaults before pico-vscode.cmake selects a toolchain.
set(PICO_BOARD pimoroni_plasma2350 CACHE STRING "Pico SDK board type" FORCE)
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Pico SDK platform" FORCE)
set(PICO_COMPILER pico_arm_cortex_m33_gcc CACHE STRING "Pico SDK compiler" FORCE)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.16)

include(pico_sdk_import.cmake)

if(NOT EXISTS "${PICO_SDK_PATH}/src/boards/include/boards/pimoroni_plasma2350.h")
    message(FATAL_ERROR "Required board definition 'pimoroni_plasma2350' was not found in PICO_SDK_PATH='${PICO_SDK_PATH}'.")
endif()

project(tasbot_eyes_pico C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

option(TASBOT_ENABLE_ASSET_GENERATION "Generate animation assets during build" ON)
option(TASBOT_LED_INDEX_USE_TASBOT_8X28 "Use TASBotIndex-based 8x28 LED indexing map" OFF)

pico_sdk_init()

set(TASBOT_GENERATED_ASSET_DIR ${CMAKE_CURRENT_LIST_DIR}/assets/generated)
set(TASBOT_ANIMATION_REGISTRY_HEADER ${TASBOT_GENERATED_ASSET_DIR}/animation_registry.generated.h)

if(TASBOT_ENABLE_ASSET_GENERATION)
    find_program(POWERSHELL_EXECUTABLE NAMES powershell.exe pwsh REQUIRED)

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
endif()

add_executable(tasbot_eyes_pico
    src/firmware/animation_registry.c
    src/firmware/main.c
    src/firmware/hw_led_pio.c
    src/portable/embedded_animation.c
    src/portable/tasbot_layout.c
    src/portable/smoke_patterns.c
)

if(TASBOT_ENABLE_ASSET_GENERATION)
    add_dependencies(tasbot_eyes_pico generate_all_assets)
else()
    message(STATUS "Asset generation disabled; using checked-in generated assets")
endif()

target_include_directories(tasbot_eyes_pico PRIVATE
    ${TASBOT_GENERATED_ASSET_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src/firmware
    ${CMAKE_CURRENT_LIST_DIR}/src/portable
)

target_link_libraries(tasbot_eyes_pico
    pico_stdlib
    hardware_clocks
    hardware_pio
    hardware_watchdog
)

if(TASBOT_LED_INDEX_USE_TASBOT_8X28)
    target_compile_definitions(tasbot_eyes_pico PRIVATE TASBOT_LED_INDEX_USE_TASBOT_8X28=1)
endif()

# Use UART (GPIO 0/1 at 115200 baud) instead of USB CDC.
# USB CDC requires TinyUSB interrupt handlers that share the 2 KB main stack;
# combined with the animation loop call depth this can overflow the stack and
# produce a hard fault that survives hardware reset.  UART has no such overhead.
pico_enable_stdio_usb(tasbot_eyes_pico 0)
pico_enable_stdio_uart(tasbot_eyes_pico 1)

pico_add_extra_outputs(tasbot_eyes_pico)

message(STATUS "Configured tasbot_eyes_pico for board '${PICO_BOARD}'")
if(TASBOT_LED_INDEX_USE_TASBOT_8X28)
    message(STATUS "LED indexing mode: TASBotIndex 8x28")
else()
    message(STATUS "LED indexing mode: serpentine 8x32")
endif()
