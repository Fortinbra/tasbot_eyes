#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "runtime_types.h"

void tasbot_frame_clear(tasbot_frame_t* frame);
bool tasbot_frame_set_pixel(tasbot_frame_t* frame, uint8_t x, uint8_t y, tasbot_color_t color);
int tasbot_layout_index(uint8_t x, uint8_t y);
bool tasbot_nose_field_to_logical(uint8_t index, uint8_t* x, uint8_t* y);
void tasbot_layout_blit_frame(const tasbot_frame_t* frame, tasbot_color_t* leds, size_t led_count);
