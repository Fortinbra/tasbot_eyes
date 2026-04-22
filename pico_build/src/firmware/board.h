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
