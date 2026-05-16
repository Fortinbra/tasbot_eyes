#pragma once

#include "embedded_animation.h"
#include <stdint.h>

extern const tasbot_embedded_animation_t g_tasbot_startup_animation;
extern const tasbot_embedded_animation_t g_tasbot_base_animation;
extern const tasbot_embedded_animation_t g_tasbot_blink_animation;
extern const tasbot_embedded_animation_t* const* g_tasbot_animation_playlist;
extern const uint16_t g_tasbot_animation_playlist_count;
