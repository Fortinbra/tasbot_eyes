#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "runtime_types.h"

typedef struct hw_led_metrics {
    uint32_t frames_presented;
    uint32_t last_checksum;
    size_t last_lit_pixels;
} hw_led_metrics_t;

bool hw_led_init(void);
bool hw_led_present_rgb888(const tasbot_color_t* leds, size_t led_count, hw_led_metrics_t* metrics);
