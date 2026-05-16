#include "smoke_patterns.h"

#include "tasbot_layout.h"

#define COLOR_RED 0x00FF0000u
#define COLOR_GREEN 0x0000FF00u
#define COLOR_BLUE 0x000000FFu
#define COLOR_WHITE 0x00FFFFFFu

static uint32_t render_left_eye(tasbot_frame_t* frame)
{
    uint8_t y;
    uint32_t lit = 0;

    for (y = 2; y <= 5; ++y) {
        lit += tasbot_frame_set_pixel(frame, 0, y, COLOR_RED) ? 1u : 0u;
        lit += tasbot_frame_set_pixel(frame, 7, y, COLOR_RED) ? 1u : 0u;
    }

    for (uint8_t x = 1; x <= 6; ++x) {
        lit += tasbot_frame_set_pixel(frame, x, 2, COLOR_RED) ? 1u : 0u;
        lit += tasbot_frame_set_pixel(frame, x, 5, COLOR_RED) ? 1u : 0u;
    }

    return lit;
}

static uint32_t render_right_eye(tasbot_frame_t* frame)
{
    uint8_t y;
    uint32_t lit = 0;

    for (y = 2; y <= 5; ++y) {
        lit += tasbot_frame_set_pixel(frame, 20, y, COLOR_GREEN) ? 1u : 0u;
        lit += tasbot_frame_set_pixel(frame, 27, y, COLOR_GREEN) ? 1u : 0u;
    }

    for (uint8_t x = 21; x <= 26; ++x) {
        lit += tasbot_frame_set_pixel(frame, x, 2, COLOR_GREEN) ? 1u : 0u;
        lit += tasbot_frame_set_pixel(frame, x, 5, COLOR_GREEN) ? 1u : 0u;
    }

    return lit;
}

static uint32_t render_nose_field(tasbot_frame_t* frame)
{
    uint8_t logical_x;
    uint8_t logical_y;
    uint8_t index;
    uint32_t lit = 0;

    for (index = 0; index < (TASBOT_NOSE_FIELD_WIDTH * TASBOT_NOSE_FIELD_HEIGHT); ++index) {
        if (!tasbot_nose_field_to_logical(index, &logical_x, &logical_y)) {
            continue;
        }

        lit += tasbot_frame_set_pixel(frame, logical_x, logical_y, COLOR_BLUE) ? 1u : 0u;

        if ((logical_x == 10u || logical_x == 17u) && (logical_y == 1u || logical_y == 7u)) {
            lit += tasbot_frame_set_pixel(frame, (uint8_t)(logical_x == 10u ? logical_x - 1u : logical_x + 1u), logical_y, COLOR_BLUE) ? 1u : 0u;
        }
    }

    return lit;
}

static uint32_t render_alignment_points(tasbot_frame_t* frame)
{
    static const uint8_t points[][2] = {
        {2u, 0u},
        {5u, 7u},
        {22u, 0u},
        {25u, 7u},
        {13u, 1u},
        {14u, 6u},
    };
    uint32_t lit = 0;

    for (size_t i = 0; i < (sizeof(points) / sizeof(points[0])); ++i) {
        lit += tasbot_frame_set_pixel(frame, points[i][0], points[i][1], COLOR_WHITE) ? 1u : 0u;
    }

    return lit;
}

void tasbot_smoke_pattern_render(tasbot_frame_t* frame, uint32_t phase, tasbot_smoke_pattern_status_t* status)
{
    uint32_t lit = 0;

    tasbot_frame_clear(frame);

    switch (phase % 4u) {
        case 0u:
            lit = render_left_eye(frame);
            break;
        case 1u:
            lit = render_right_eye(frame);
            break;
        case 2u:
            lit = render_nose_field(frame);
            break;
        default:
            lit = render_alignment_points(frame);
            break;
    }

    if (status != NULL) {
        status->phase = phase % 4u;
        status->lit_pixels = lit;
    }
}
