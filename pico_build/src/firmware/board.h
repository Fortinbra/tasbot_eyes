#pragma once

#include "runtime_types.h"

#define TASBOT_EYES_LED_PROTOCOL_WS2812B 1
#define TASBOT_EYES_LED_PROTOCOL TASBOT_EYES_LED_PROTOCOL_WS2812B
#define TASBOT_EYES_LED_PROTOCOL_NAME "WS2812B"
#define TASBOT_EYES_ACTIVE_LED_ROWS 8
#define TASBOT_EYES_ACTIVE_LED_COLUMNS 28
#define TASBOT_EYES_ACTIVE_LED_PIXEL_COUNT 224
#define TASBOT_EYES_PHYSICAL_LED_ROWS 8
#define TASBOT_EYES_PHYSICAL_LED_COLUMNS 32
/* Physical LED chain starts at the top-left LED and snakes by column. */
#define TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT 256
#define TASBOT_EYES_SMOKE_PHASE_INTERVAL_MS 750
#define TASBOT_EYES_SMOKE_PHASES_PER_CYCLE 4
#define TASBOT_EYES_SMOKE_PHASE_RATE_HZ_NUMERATOR 4
#define TASBOT_EYES_SMOKE_PHASE_RATE_HZ_DENOMINATOR 3
#define TASBOT_EYES_SMOKE_CYCLE_INTERVAL_MS (TASBOT_EYES_SMOKE_PHASE_INTERVAL_MS * TASBOT_EYES_SMOKE_PHASES_PER_CYCLE)

#define TASBOT_EYES_STRINGIFY_STEP(value) #value
#define TASBOT_EYES_STRINGIFY(value) TASBOT_EYES_STRINGIFY_STEP(value)

#define TASBOT_EYES_BOOT_BANNER "tasbot_eyes pico_build booting"
#define TASBOT_EYES_READY_BANNER "tasbot_eyes pico_build ready"
#define TASBOT_EYES_BOOT_DELAY_MS 2000u
#define TASBOT_EYES_READY_REPEAT_COUNT 4u
#define TASBOT_EYES_READY_REPEAT_INTERVAL_MS 250u

#if defined(PICO_DEFAULT_WS2812_PIN)
#define TASBOT_EYES_LED_DATA_PIN PICO_DEFAULT_WS2812_PIN
#define TASBOT_EYES_LED_DATA_PIN_SOURCE "PICO_DEFAULT_WS2812_PIN"
#elif defined(PLASMA2350_DATA_PIN)
#define TASBOT_EYES_LED_DATA_PIN PLASMA2350_DATA_PIN
#define TASBOT_EYES_LED_DATA_PIN_SOURCE "PLASMA2350_DATA_PIN"
#else
#define TASBOT_EYES_LED_DATA_PIN 15
#define TASBOT_EYES_LED_DATA_PIN_SOURCE "GPIO15 fallback"
#endif

#if defined(PLASMA2350_SW_A_PIN)
#define TASBOT_EYES_BRIGHTNESS_BUTTON_PIN PLASMA2350_SW_A_PIN
#define TASBOT_EYES_BRIGHTNESS_BUTTON_PIN_SOURCE "PLASMA2350_SW_A_PIN"
#else
#define TASBOT_EYES_BRIGHTNESS_BUTTON_PIN 12
#define TASBOT_EYES_BRIGHTNESS_BUTTON_PIN_SOURCE "GPIO12 fallback"
#endif

#define TASBOT_EYES_BRIGHTNESS_BUTTON_ACTIVE_LOW 1
#define TASBOT_EYES_BRIGHTNESS_BUTTON_DEBOUNCE_US 200000u

/* F06: legacy brightness parity (0..255 domain). */
#define TASBOT_EYES_BRIGHTNESS_BUTTON_ENABLE 1u
#define TASBOT_EYES_BRIGHTNESS_DEFAULT_0_255 20u
#define TASBOT_EYES_BRIGHTNESS_LEVEL_0_255 8u
#define TASBOT_EYES_BRIGHTNESS_LEVEL_1_255 20u
#define TASBOT_EYES_BRIGHTNESS_LEVEL_2_255 128u
#define TASBOT_EYES_BRIGHTNESS_LEVEL_3_255 255u

/* F02: rainbow white-pixel remap compile-time controls. */
#define TASBOT_EYES_RAINBOW_MODE 1u
#define TASBOT_EYES_RAINBOW_STEP_MIN_MS 10u
#define TASBOT_EYES_RAINBOW_STEP_MAX_MS 1000u
#define TASBOT_EYES_RAINBOW_STEP_MS 1000u
#define TASBOT_EYES_RAINBOW_STEP_MS_CLAMPED                                                   \
    ((TASBOT_EYES_RAINBOW_STEP_MS < TASBOT_EYES_RAINBOW_STEP_MIN_MS) ? TASBOT_EYES_RAINBOW_STEP_MIN_MS \
     : (TASBOT_EYES_RAINBOW_STEP_MS > TASBOT_EYES_RAINBOW_STEP_MAX_MS) ? TASBOT_EYES_RAINBOW_STEP_MAX_MS \
                                                                           : TASBOT_EYES_RAINBOW_STEP_MS)

/* F03: blink scheduler parity controls. */
#define TASBOT_EYES_BLINK_MAX_COUNT 3u
#define TASBOT_EYES_BLINK_MIN_DELAY_MS 3000u
#define TASBOT_EYES_BLINK_MAX_DELAY_MS 5000u
#define TASBOT_EYES_BLINK_DELAY_LOW_MS                                                        \
    ((TASBOT_EYES_BLINK_MIN_DELAY_MS <= TASBOT_EYES_BLINK_MAX_DELAY_MS) ? TASBOT_EYES_BLINK_MIN_DELAY_MS \
                                                                           : TASBOT_EYES_BLINK_MAX_DELAY_MS)
#define TASBOT_EYES_BLINK_DELAY_HIGH_MS                                                       \
    ((TASBOT_EYES_BLINK_MIN_DELAY_MS <= TASBOT_EYES_BLINK_MAX_DELAY_MS) ? TASBOT_EYES_BLINK_MAX_DELAY_MS \
                                                                           : TASBOT_EYES_BLINK_MIN_DELAY_MS)

/* F04: random monochrome recolor controls. */
#define TASBOT_EYES_RANDOMIZE_MONOCHROME 1u
#define TASBOT_EYES_RECOLOR_PALETTE_COLOR_0 0x00ff4040u
#define TASBOT_EYES_RECOLOR_PALETTE_COLOR_1 0x00ff9f1cu
#define TASBOT_EYES_RECOLOR_PALETTE_COLOR_2 0x00ffd400u
#define TASBOT_EYES_RECOLOR_PALETTE_COLOR_3 0x002ecc71u
#define TASBOT_EYES_RECOLOR_PALETTE_COLOR_4 0x0012c2e9u
#define TASBOT_EYES_RECOLOR_PALETTE_COLOR_5 0x008a5cffu
#define TASBOT_EYES_RECOLOR_PALETTE_COLOR_6 0x00ff4ec4u
#define TASBOT_EYES_RECOLOR_PALETTE_COUNT 7u

/* F07: gamma correction controls. */
#define TASBOT_EYES_GAMMA_ENABLE 1u
#define TASBOT_EYES_GAMMA_CURVE_LEGACY22 1u
#define TASBOT_EYES_GAMMA_CURVE_SRGB 2u
#define TASBOT_EYES_GAMMA_CURVE_CUSTOM_LUT 3u
#define TASBOT_EYES_GAMMA_CURVE TASBOT_EYES_GAMMA_CURVE_SRGB

/* Cleanup: optional high-frequency runtime diagnostics. */
#define TASBOT_EYES_RUNTIME_VERBOSE_LOGS 0u

#if !defined(TASBOT_EYES_LED_PROTOCOL)
#error "TASBOT_EYES_LED_PROTOCOL must be declared in board.h."
#endif

#if TASBOT_EYES_LED_PROTOCOL != TASBOT_EYES_LED_PROTOCOL_WS2812B
#error "This runtime slice only supports WS2812B over PIO."
#endif

_Static_assert(TASBOT_EYES_ACTIVE_LED_PIXEL_COUNT == TASBOT_ACTIVE_LED_COUNT,
               "board.h active pixel contract must match the portable layout's active LED count.");
_Static_assert(TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT == TASBOT_PHYSICAL_LED_COUNT,
               "board.h physical pixel contract must match the transport chain length.");
_Static_assert(TASBOT_EYES_ACTIVE_LED_COLUMNS == TASBOT_LOGICAL_WIDTH,
               "board.h active column contract must match the portable frame width.");
_Static_assert(TASBOT_EYES_ACTIVE_LED_ROWS == TASBOT_LOGICAL_HEIGHT,
               "board.h active row contract must match the portable frame height.");
_Static_assert(TASBOT_EYES_PHYSICAL_LED_ROWS == TASBOT_PHYSICAL_HEIGHT,
               "board.h physical row contract must match the wired matrix height.");
_Static_assert(TASBOT_EYES_PHYSICAL_LED_COLUMNS == TASBOT_PHYSICAL_WIDTH,
               "board.h physical column contract must match the wired matrix width.");
_Static_assert((TASBOT_EYES_SMOKE_PHASE_RATE_HZ_NUMERATOR * TASBOT_EYES_SMOKE_PHASE_INTERVAL_MS) ==
                   (1000 * TASBOT_EYES_SMOKE_PHASE_RATE_HZ_DENOMINATOR),
               "board.h smoke cadence and phase rate must stay in sync.");

#define TASBOT_EYES_BOARD_CONTRACT_BANNER                                                    \
    "board contract: " TASBOT_EYES_LED_PROTOCOL_NAME " on GPIO" TASBOT_EYES_STRINGIFY(      \
        TASBOT_EYES_LED_DATA_PIN) ", active " TASBOT_EYES_STRINGIFY(TASBOT_EYES_ACTIVE_LED_ROWS) "x" \
        TASBOT_EYES_STRINGIFY(TASBOT_EYES_ACTIVE_LED_COLUMNS) " ("                           \
        TASBOT_EYES_STRINGIFY(TASBOT_EYES_ACTIVE_LED_PIXEL_COUNT) " pixels) within physical " \
        TASBOT_EYES_STRINGIFY(TASBOT_EYES_PHYSICAL_LED_ROWS) "x"                             \
        TASBOT_EYES_STRINGIFY(TASBOT_EYES_PHYSICAL_LED_COLUMNS) " ("                         \
        TASBOT_EYES_STRINGIFY(TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT) " pixels)"

#define TASBOT_EYES_WS2812_BIT_FREQUENCY 800000u
#define TASBOT_EYES_WS2812_IS_RGBW 0
#define TASBOT_EYES_WS2812_RESET_US 300u
#define TASBOT_EYES_WS2812_OSR_DRAIN_US 40u
#define TASBOT_EYES_GLOBAL_BRIGHTNESS_PERCENT (((TASBOT_EYES_BRIGHTNESS_DEFAULT_0_255 * 100u) + 127u) / 255u)

_Static_assert(TASBOT_EYES_GLOBAL_BRIGHTNESS_PERCENT <= 100u,
               "TASBOT_EYES_GLOBAL_BRIGHTNESS_PERCENT must be in the range 0..100.");
_Static_assert(TASBOT_EYES_BRIGHTNESS_BUTTON_ENABLE <= 1u,
               "TASBOT_EYES_BRIGHTNESS_BUTTON_ENABLE must be 0 or 1.");
_Static_assert(TASBOT_EYES_BRIGHTNESS_DEFAULT_0_255 <= 255u,
               "TASBOT_EYES_BRIGHTNESS_DEFAULT_0_255 must be in the range 0..255.");
_Static_assert(TASBOT_EYES_BRIGHTNESS_LEVEL_0_255 <= 255u,
               "TASBOT_EYES_BRIGHTNESS_LEVEL_0_255 must be in the range 0..255.");
_Static_assert(TASBOT_EYES_BRIGHTNESS_LEVEL_1_255 <= 255u,
               "TASBOT_EYES_BRIGHTNESS_LEVEL_1_255 must be in the range 0..255.");
_Static_assert(TASBOT_EYES_BRIGHTNESS_LEVEL_2_255 <= 255u,
               "TASBOT_EYES_BRIGHTNESS_LEVEL_2_255 must be in the range 0..255.");
_Static_assert(TASBOT_EYES_BRIGHTNESS_LEVEL_3_255 <= 255u,
               "TASBOT_EYES_BRIGHTNESS_LEVEL_3_255 must be in the range 0..255.");
_Static_assert(TASBOT_EYES_RAINBOW_MODE <= 1u,
               "TASBOT_EYES_RAINBOW_MODE must be 0 or 1.");
_Static_assert(TASBOT_EYES_RAINBOW_STEP_MS_CLAMPED >= TASBOT_EYES_RAINBOW_STEP_MIN_MS &&
                   TASBOT_EYES_RAINBOW_STEP_MS_CLAMPED <= TASBOT_EYES_RAINBOW_STEP_MAX_MS,
               "TASBOT_EYES_RAINBOW_STEP_MS_CLAMPED must be in the range 10..1000.");
_Static_assert(TASBOT_EYES_BLINK_MAX_COUNT <= 9u,
               "TASBOT_EYES_BLINK_MAX_COUNT must be in the range 0..9.");
_Static_assert(TASBOT_EYES_BLINK_DELAY_LOW_MS <= TASBOT_EYES_BLINK_DELAY_HIGH_MS,
               "TASBOT_EYES_BLINK_DELAY_LOW_MS must be <= TASBOT_EYES_BLINK_DELAY_HIGH_MS.");
_Static_assert(TASBOT_EYES_BLINK_DELAY_HIGH_MS <= 65535u,
               "TASBOT_EYES_BLINK delays must fit into uint16 milliseconds.");
_Static_assert(TASBOT_EYES_RANDOMIZE_MONOCHROME <= 1u,
               "TASBOT_EYES_RANDOMIZE_MONOCHROME must be 0 or 1.");
_Static_assert(TASBOT_EYES_RECOLOR_PALETTE_COUNT > 0u,
               "TASBOT_EYES_RECOLOR_PALETTE_COUNT must be greater than zero.");
_Static_assert(TASBOT_EYES_GAMMA_ENABLE <= 1u,
               "TASBOT_EYES_GAMMA_ENABLE must be 0 or 1.");
_Static_assert(TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_LEGACY22 ||
                   TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_SRGB ||
                   TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_CUSTOM_LUT,
               "TASBOT_EYES_GAMMA_CURVE must be LEGACY22, SRGB, or CUSTOM_LUT.");
_Static_assert(TASBOT_EYES_RUNTIME_VERBOSE_LOGS <= 1u,
               "TASBOT_EYES_RUNTIME_VERBOSE_LOGS must be 0 or 1.");
