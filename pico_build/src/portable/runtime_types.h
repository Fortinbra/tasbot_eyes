#pragma once

#include <stdint.h>

#define TASBOT_LOGICAL_WIDTH 28u
#define TASBOT_LOGICAL_HEIGHT 8u
#define TASBOT_ACTIVE_LED_COUNT 224u
#define TASBOT_PHYSICAL_WIDTH 32u
#define TASBOT_PHYSICAL_HEIGHT 8u
#define TASBOT_PHYSICAL_LED_COUNT 256u
#define TASBOT_NOSE_RANGE_MIN 9u
#define TASBOT_NOSE_RANGE_MAX 18u
#define TASBOT_NOSE_FIELD_WIDTH 8u
#define TASBOT_NOSE_FIELD_HEIGHT 6u

typedef uint32_t tasbot_color_t;

typedef struct tasbot_frame {
    tasbot_color_t pixels[TASBOT_LOGICAL_HEIGHT][TASBOT_LOGICAL_WIDTH];
} tasbot_frame_t;
