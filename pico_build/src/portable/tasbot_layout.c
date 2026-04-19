#include "tasbot_layout.h"

void tasbot_frame_clear(tasbot_frame_t* frame)
{
    uint8_t x;
    uint8_t y;

    for (y = 0; y < TASBOT_LOGICAL_HEIGHT; ++y) {
        for (x = 0; x < TASBOT_LOGICAL_WIDTH; ++x) {
            frame->pixels[y][x] = 0;
        }
    }
}

bool tasbot_frame_set_pixel(tasbot_frame_t* frame, uint8_t x, uint8_t y, tasbot_color_t color)
{
    if (x >= TASBOT_LOGICAL_WIDTH || y >= TASBOT_LOGICAL_HEIGHT) {
        return false;
    }

    frame->pixels[y][x] = color;
    return true;
}

int tasbot_layout_index(uint8_t x, uint8_t y)
{
    if (x >= TASBOT_LOGICAL_WIDTH || y >= TASBOT_LOGICAL_HEIGHT) {
        return -1;
    }

    if ((x & 1u) == 0u) {
        return (int)(((unsigned int)x * TASBOT_LOGICAL_HEIGHT) + y);
    }

    return (int)(((unsigned int)x * TASBOT_LOGICAL_HEIGHT) + (TASBOT_LOGICAL_HEIGHT - 1u - y));
}

bool tasbot_nose_field_to_logical(uint8_t index, uint8_t* x, uint8_t* y)
{
    uint8_t logical_x;
    uint8_t logical_y;

    if (index >= (TASBOT_NOSE_FIELD_WIDTH * TASBOT_NOSE_FIELD_HEIGHT)) {
        return false;
    }

    logical_y = index / TASBOT_NOSE_FIELD_WIDTH;
    switch (logical_y) {
        case 2:
        case 3:
            logical_y += 1u;
            break;
        case 4:
        case 5:
            logical_y += 2u;
            break;
        default:
            break;
    }

    logical_x = (uint8_t)(TASBOT_NOSE_RANGE_MIN + 1u + (index % TASBOT_NOSE_FIELD_WIDTH));

    *x = logical_x;
    *y = logical_y;
    return true;
}

void tasbot_layout_blit_frame(const tasbot_frame_t* frame, tasbot_color_t* leds, size_t led_count)
{
    uint8_t x;
    uint8_t y;

    for (size_t i = 0; i < led_count; ++i) {
        leds[i] = 0;
    }

    for (y = 0; y < TASBOT_LOGICAL_HEIGHT; ++y) {
        for (x = 0; x < TASBOT_LOGICAL_WIDTH; ++x) {
            int index = tasbot_layout_index(x, y);

            if (index >= 0 && (size_t)index < led_count) {
                leds[index] = frame->pixels[y][x];
            }
        }
    }
}
