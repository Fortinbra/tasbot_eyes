#include "tasbot_layout.h"

#if defined(TASBOT_LED_INDEX_USE_TASBOT_8X28)
/* Legacy TASBot physical mapping table from tasbot.c.
   Values < 0 indicate no physical LED at that logical location. */
static const int16_t kTasbotIndex8x28[TASBOT_LOGICAL_HEIGHT][TASBOT_LOGICAL_WIDTH] = {
    {-1, -1, 0, 1, 2, 3, -1, -1, -1, -1, 101, 100, 99, 98, 97, 96, 95, 94, -1, -1, -1, -1, 105, 104, 103, 102, -1, -1},
    {-1, 4, 5, 6, 7, 8, 9, -1, -1, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, -1, -1, 111, 110, 109, 108, 107, 106, -1},
    {10, 11, 12, 13, 14, 15, 16, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 119, 118, 117, 116, 115, 114, 113, 112},
    {18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, 83, 82, 81, 80, 79, 78, -1, -1, -1, 127, 126, 125, 124, 123, 122, 121, 120},
    {26, 27, 28, 29, 30, 31, 32, 33, -1, -1, 70, 71, 72, 73, 74, 75, 76, 77, -1, -1, 135, 134, 133, 132, 131, 130, 129, 128},
    {34, 35, 36, 37, 38, 39, 40, 41, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 143, 142, 141, 140, 139, 138, 137, 136},
    {-1, 42, 43, 44, 45, 46, 47, -1, -1, -1, 68, 67, 66, 65, 64, 63, 62, 61, -1, -1, -1, 149, 148, 147, 146, 145, 144, -1},
    {-1, -1, 48, 49, 50, 51, -1, -1, -1, 69, 52, 53, 54, 55, 56, 57, 58, 59, 60, -1, -1, -1, 153, 152, 151, 150, -1, -1}
};
#endif

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

#if defined(TASBOT_LED_INDEX_USE_TASBOT_8X28)
    return (int)kTasbotIndex8x28[y][x];
#else
    const uint8_t centered_x = (uint8_t)(x + ((TASBOT_PHYSICAL_WIDTH - TASBOT_LOGICAL_WIDTH) / 2u));

    if ((centered_x & 1u) == 0u) {
        return (int)(((unsigned int)centered_x * TASBOT_PHYSICAL_HEIGHT) + y);
    }

    return (int)(((unsigned int)centered_x * TASBOT_PHYSICAL_HEIGHT) + (TASBOT_PHYSICAL_HEIGHT - 1u - y));
#endif
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
