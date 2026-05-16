#include "embedded_animation.h"

#include "tasbot_layout.h"

size_t tasbot_embedded_animation_frame_pixel_count(const tasbot_embedded_animation_t* animation)
{
    if (animation == NULL) {
        return 0u;
    }

    return (size_t)animation->width * (size_t)animation->height;
}

bool tasbot_embedded_animation_load_frame(
    const tasbot_embedded_animation_t* animation,
    uint16_t frame_index,
    tasbot_frame_t* frame)
{
    size_t frame_pixel_count;
    const tasbot_color_t* source_pixels;
    uint8_t x;
    uint8_t y;

    if (animation == NULL || frame == NULL || animation->frame_pixels == NULL) {
        return false;
    }

    if (animation->width > TASBOT_LOGICAL_WIDTH ||
        animation->height > TASBOT_LOGICAL_HEIGHT ||
        frame_index >= animation->frame_count) {
        return false;
    }

    frame_pixel_count = tasbot_embedded_animation_frame_pixel_count(animation);
    source_pixels = animation->frame_pixels + ((size_t)frame_index * frame_pixel_count);

    tasbot_frame_clear(frame);

    for (y = 0; y < animation->height; ++y) {
        for (x = 0; x < animation->width; ++x) {
            frame->pixels[y][x] = source_pixels[((size_t)y * animation->width) + x];
        }
    }

    return true;
}

uint16_t tasbot_embedded_animation_frame_delay_ms(
    const tasbot_embedded_animation_t* animation,
    uint16_t frame_index)
{
    if (animation == NULL || animation->frame_delays_ms == NULL || frame_index >= animation->frame_count) {
        return 0u;
    }

    return animation->frame_delays_ms[frame_index];
}
