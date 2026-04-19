#pragma once

#include <stdint.h>

#include "runtime_types.h"

typedef struct tasbot_smoke_pattern_status {
    uint32_t phase;
    uint32_t lit_pixels;
} tasbot_smoke_pattern_status_t;

void tasbot_smoke_pattern_render(tasbot_frame_t* frame, uint32_t phase, tasbot_smoke_pattern_status_t* status);
