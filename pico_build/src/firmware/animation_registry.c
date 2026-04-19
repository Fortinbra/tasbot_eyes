#include "animation_registry.h"

#include "animation_registry.generated.h"

const tasbot_embedded_animation_t* const* g_tasbot_animation_playlist = kTasbotAnimationPlaylist;
const uint16_t g_tasbot_animation_playlist_count =
    (uint16_t)(sizeof(kTasbotAnimationPlaylist) / sizeof(kTasbotAnimationPlaylist[0]));
