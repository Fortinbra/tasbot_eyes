#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "runtime_types.h"

typedef struct tasbot_embedded_animation {
    const char* name;
    const char* source_path;
    const char* source_pool;
    const char* source_rule;
    const char* source_sha256;
    uint16_t width;
    uint16_t height;
    uint16_t frame_count;
    const uint16_t* frame_delays_ms;
    const tasbot_color_t* frame_pixels;
} tasbot_embedded_animation_t;

size_t tasbot_embedded_animation_frame_pixel_count(const tasbot_embedded_animation_t* animation);
bool tasbot_embedded_animation_load_frame(
    const tasbot_embedded_animation_t* animation,
    uint16_t frame_index,
    tasbot_frame_t* frame);
uint16_t tasbot_embedded_animation_frame_delay_ms(
    const tasbot_embedded_animation_t* animation,
    uint16_t frame_index);
