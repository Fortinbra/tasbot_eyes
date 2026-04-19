#include "hw_led.h"

#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "board.h"
#include "ws2812.pio.h"

typedef struct hw_led_context {
    bool initialized;
    PIO pio;
    uint sm;
    uint offset;
    hw_led_metrics_t metrics;
} hw_led_context_t;

static hw_led_context_t g_hw_led;

static uint32_t hw_led_pack_ws2812(tasbot_color_t color)
{
    uint32_t red = (color >> 16) & 0xffu;
    uint32_t green = (color >> 8) & 0xffu;
    uint32_t blue = color & 0xffu;

    return (green << 24) | (red << 16) | (blue << 8);
}

static uint32_t hw_led_checksum_step(uint32_t hash, tasbot_color_t color)
{
    hash ^= color;
    hash *= 16777619u;
    return hash;
}

static void hw_led_wait_for_reset(PIO pio, uint sm)
{
    while (!pio_sm_is_tx_fifo_empty(pio, sm)) {
        tight_loop_contents();
    }

    sleep_us(TASBOT_EYES_WS2812_RESET_US + TASBOT_EYES_WS2812_OSR_DRAIN_US);
}

static void hw_led_push_frame(const tasbot_color_t* leds, size_t led_count)
{
    for (size_t i = 0; i < led_count; ++i) {
        pio_sm_put_blocking(g_hw_led.pio, g_hw_led.sm, hw_led_pack_ws2812(leds[i]));
    }

    hw_led_wait_for_reset(g_hw_led.pio, g_hw_led.sm);
}

bool hw_led_init(void)
{
    if (g_hw_led.initialized) {
        return true;
    }

    if (!pio_claim_free_sm_and_add_program_for_gpio_range(
            &ws2812_program,
            &g_hw_led.pio,
            &g_hw_led.sm,
            &g_hw_led.offset,
            TASBOT_EYES_LED_DATA_PIN,
            1,
            true)) {
        return false;
    }

    ws2812_program_init(
        g_hw_led.pio,
        g_hw_led.sm,
        g_hw_led.offset,
        TASBOT_EYES_LED_DATA_PIN,
        (float)TASBOT_EYES_WS2812_BIT_FREQUENCY,
        TASBOT_EYES_WS2812_IS_RGBW);

    memset(&g_hw_led.metrics, 0, sizeof(g_hw_led.metrics));
    g_hw_led.initialized = true;

    static const tasbot_color_t kClearFrame[TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT] = {0};
    hw_led_push_frame(kClearFrame, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT);

    return true;
}

bool hw_led_present_rgb888(const tasbot_color_t* leds, size_t led_count, hw_led_metrics_t* metrics)
{
    uint32_t checksum = 2166136261u;
    size_t lit_pixels = 0;

    if (!g_hw_led.initialized || leds == NULL || led_count != TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT) {
        return false;
    }

    for (size_t i = 0; i < led_count; ++i) {
        tasbot_color_t color = leds[i];

        if (color != 0u) {
            ++lit_pixels;
        }

        checksum = hw_led_checksum_step(checksum, color);
        pio_sm_put_blocking(g_hw_led.pio, g_hw_led.sm, hw_led_pack_ws2812(color));
    }

    hw_led_wait_for_reset(g_hw_led.pio, g_hw_led.sm);

    g_hw_led.metrics.frames_presented += 1u;
    g_hw_led.metrics.last_checksum = checksum;
    g_hw_led.metrics.last_lit_pixels = lit_pixels;

    if (metrics != NULL) {
        *metrics = g_hw_led.metrics;
    }

    return true;
}
