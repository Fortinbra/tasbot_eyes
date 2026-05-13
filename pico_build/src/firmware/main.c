#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"

#include "animation_registry.h"
#include "board.h"
#include "embedded_animation.h"
#include "hw_led.h"
#include "tasbot_layout.h"

#define TASBOT_COLOR_WHITE 0x00ffffffu

typedef enum blink_scheduler_state {
    BLINK_STATE_ANIMATION = 0,
    BLINK_STATE_BASE,
    BLINK_STATE_WAIT,
    BLINK_STATE_BURST
} blink_scheduler_state_t;

static tasbot_color_t rainbow_hue_to_rgb888(uint8_t hue)
{
    uint8_t region = (uint8_t)(hue / 43u);
    uint8_t remainder = (uint8_t)((hue - (region * 43u)) * 6u);
    uint8_t q = (uint8_t)(255u - (((uint16_t)255u * remainder) >> 8));
    uint8_t t = (uint8_t)(((uint16_t)255u * remainder) >> 8);
    uint8_t red = 0u;
    uint8_t green = 0u;
    uint8_t blue = 0u;

    switch (region) {
        case 0u:
            red = 255u;
            green = t;
            blue = 0u;
            break;
        case 1u:
            red = q;
            green = 255u;
            blue = 0u;
            break;
        case 2u:
            red = 0u;
            green = 255u;
            blue = t;
            break;
        case 3u:
            red = 0u;
            green = q;
            blue = 255u;
            break;
        case 4u:
            red = t;
            green = 0u;
            blue = 255u;
            break;
        default:
            red = 255u;
            green = 0u;
            blue = q;
            break;
    }

    return ((tasbot_color_t)red << 16) | ((tasbot_color_t)green << 8) | (tasbot_color_t)blue;
}

static void rainbow_update_hue(uint8_t* rainbow_hue, uint64_t* rainbow_last_step_us, uint64_t now_us)
{
    const uint64_t step_us = (uint64_t)TASBOT_EYES_RAINBOW_STEP_MS_CLAMPED * 1000u;
    uint64_t elapsed_us;
    uint64_t elapsed_steps;

    if (step_us == 0u || now_us <= *rainbow_last_step_us) {
        return;
    }

    elapsed_us = now_us - *rainbow_last_step_us;
    elapsed_steps = elapsed_us / step_us;

    if (elapsed_steps == 0u) {
        return;
    }

    *rainbow_hue = (uint8_t)(*rainbow_hue + (uint8_t)elapsed_steps);
    *rainbow_last_step_us += elapsed_steps * step_us;
}

static void rainbow_remap_white_pixels(tasbot_color_t* leds, size_t led_count, tasbot_color_t rainbow_rgb)
{
    for (size_t i = 0; i < led_count; ++i) {
        if ((leds[i] & TASBOT_COLOR_WHITE) == TASBOT_COLOR_WHITE) {
            leds[i] = rainbow_rgb;
        }
    }
}

static uint8_t brightness_legacy_to_percent(uint8_t legacy)
{
    return (uint8_t)((((uint16_t)legacy * 100u) + 127u) / 255u);
}

static uint8_t brightness_default_level_index(const uint8_t* levels, size_t level_count)
{
    for (size_t i = 0; i < level_count; ++i) {
        if (levels[i] == (uint8_t)TASBOT_EYES_BRIGHTNESS_DEFAULT_0_255) {
            return (uint8_t)i;
        }
    }

    return 0u;
}

static uint16_t playlist_next_animation_index(uint16_t current_index, uint16_t playlist_count)
{
    const uint16_t kPlaylistFirstAnimationIndex = 3u;

    if (playlist_count <= kPlaylistFirstAnimationIndex) {
        return 1u;
    }

    current_index = (uint16_t)(current_index + 1u);
    if (current_index >= playlist_count) {
        current_index = kPlaylistFirstAnimationIndex;
    }

    return current_index;
}

static uint32_t blink_prng_next(uint32_t* state)
{
    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}

static uint16_t blink_random_delay_ms(uint32_t* state)
{
    const uint16_t low = (uint16_t)TASBOT_EYES_BLINK_DELAY_LOW_MS;
    const uint16_t high = (uint16_t)TASBOT_EYES_BLINK_DELAY_HIGH_MS;

    if (high <= low) {
        return low;
    }

    return (uint16_t)(low + (blink_prng_next(state) % ((uint32_t)(high - low) + 1u)));
}

static uint8_t blink_random_burst_count(uint32_t* state)
{
    if (TASBOT_EYES_BLINK_MAX_COUNT == 0u) {
        return 0u;
    }

    return (uint8_t)(1u + (blink_prng_next(state) % TASBOT_EYES_BLINK_MAX_COUNT));
}

static bool animation_is_monochrome(const tasbot_embedded_animation_t* animation)
{
    size_t total_pixels;
    tasbot_color_t base_color = 0u;
    bool saw_non_black = false;

    if (animation == NULL || animation->frame_pixels == NULL) {
        return false;
    }

    total_pixels = (size_t)animation->frame_count *
                   (size_t)animation->width *
                   (size_t)animation->height;

    for (size_t i = 0; i < total_pixels; ++i) {
        tasbot_color_t color = animation->frame_pixels[i] & 0x00ffffffu;

        if (color == 0u) {
            continue;
        }

        if (!saw_non_black) {
            base_color = color;
            saw_non_black = true;
            continue;
        }

        if (color != base_color) {
            return false;
        }
    }

    return true;
}

static tasbot_color_t recolor_pick_palette_color(uint32_t* prng_state)
{
    static const tasbot_color_t kRecolorPalette[TASBOT_EYES_RECOLOR_PALETTE_COUNT] = {
        TASBOT_EYES_RECOLOR_PALETTE_COLOR_0,
        TASBOT_EYES_RECOLOR_PALETTE_COLOR_1,
        TASBOT_EYES_RECOLOR_PALETTE_COLOR_2,
        TASBOT_EYES_RECOLOR_PALETTE_COLOR_3,
        TASBOT_EYES_RECOLOR_PALETTE_COLOR_4,
        TASBOT_EYES_RECOLOR_PALETTE_COLOR_5,
        TASBOT_EYES_RECOLOR_PALETTE_COLOR_6
    };

    return kRecolorPalette[blink_prng_next(prng_state) % TASBOT_EYES_RECOLOR_PALETTE_COUNT];
}

static void recolor_non_black_pixels(tasbot_color_t* leds, size_t led_count, tasbot_color_t recolor)
{
    for (size_t i = 0; i < led_count; ++i) {
        if ((leds[i] & 0x00ffffffu) != 0u) {
            leds[i] = recolor;
        }
    }
}

static const uint8_t kGammaLutLegacy22[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11,
    11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20,
    21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34,
    35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50, 51, 52,
    54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68, 69, 70, 72, 73, 74, 75, 77,
    78, 79, 81, 82, 83, 85, 86, 87, 89, 90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105,
    107, 109, 110, 112, 114, 115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135,
    137, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169,
    171, 173, 175, 177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208,
    210, 213, 215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252,
    255
};

static const uint8_t* gamma_active_lut(void)
{
    if (TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_LEGACY22 ||
        TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_SRGB ||
        TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_CUSTOM_LUT) {
        return kGammaLutLegacy22;
    }

    return NULL;
}

static const char* gamma_curve_name(void)
{
    if (TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_LEGACY22) {
        return "legacy22";
    }

    if (TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_SRGB) {
        return "srgb";
    }

    if (TASBOT_EYES_GAMMA_CURVE == TASBOT_EYES_GAMMA_CURVE_CUSTOM_LUT) {
        return "custom_lut";
    }

    return "unknown";
}

static void gamma_apply_to_leds(tasbot_color_t* leds, size_t led_count)
{
    const uint8_t* lut = gamma_active_lut();

    if (lut == NULL) {
        return;
    }

    for (size_t i = 0; i < led_count; ++i) {
        uint8_t red = (uint8_t)((leds[i] >> 16) & 0xffu);
        uint8_t green = (uint8_t)((leds[i] >> 8) & 0xffu);
        uint8_t blue = (uint8_t)(leds[i] & 0xffu);

        leds[i] = ((tasbot_color_t)lut[red] << 16) |
                  ((tasbot_color_t)lut[green] << 8) |
                  (tasbot_color_t)lut[blue];
    }
}

int main(void)
{
    /* Keep large buffers in static storage — together they consume ~1.9 KB,
       which exceeds the default 2 KB Pico stack and causes a hard fault. */
    static tasbot_frame_t logical_frame;
    static tasbot_color_t physical_leds[TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT];
    const uint16_t kBaseAnimationIndex = 1u;
    const uint16_t kBlinkAnimationIndex = 2u;
    const uint16_t kPlaylistFirstAnimationIndex = 3u;
    hw_led_metrics_t metrics;
    const tasbot_embedded_animation_t* anim = NULL;
    const tasbot_embedded_animation_t* recolor_anim = NULL;
    uint16_t* active_frame_index = NULL;
    blink_scheduler_state_t blink_state = BLINK_STATE_ANIMATION;
    uint32_t ready_emit = 0u;
    uint16_t playlist_animation_index = 0u;
    uint16_t playlist_frame_index = 0u;
    uint16_t base_frame_index = 0u;
    uint16_t blink_frame_index = 0u;
    uint16_t delay_ms;
    uint16_t blink_wait_delay_ms = 0u;
    bool button_was_pressed = false;
    uint64_t button_debounce_until_us = 0u;
    uint64_t blink_wait_until_us = 0u;
    uint8_t brightness_level_index = 0u;
    uint8_t blink_burst_remaining = 0u;
    uint8_t rainbow_hue = 0u;
    uint64_t rainbow_last_step_us = 0u;
    uint32_t blink_prng_state = 0u;
    uint32_t blink_burst_selection_count = 0u;
    uint32_t blink_wait_selection_count = 0u;
    uint32_t recolor_applied_count = 0u;
    uint32_t recolor_skipped_count = 0u;
    bool recolor_active_for_animation = false;
    tasbot_color_t recolor_selected = 0u;
    static const uint8_t kBrightnessLevels[] = {
        TASBOT_EYES_BRIGHTNESS_LEVEL_0_255,
        TASBOT_EYES_BRIGHTNESS_LEVEL_1_255,
        TASBOT_EYES_BRIGHTNESS_LEVEL_2_255
    };

    stdio_init_all();
    puts(TASBOT_EYES_BOOT_BANNER);
    puts(TASBOT_EYES_BOARD_CONTRACT_BANNER);
    sleep_ms(TASBOT_EYES_BOOT_DELAY_MS);

    for (ready_emit = 0u; ready_emit < TASBOT_EYES_READY_REPEAT_COUNT; ++ready_emit) {
        puts(TASBOT_EYES_READY_BANNER);
        sleep_ms(TASBOT_EYES_READY_REPEAT_INTERVAL_MS);
    }

    puts("runtime seam: logical 28x8 active frame -> column-serpentine mapper -> "
         TASBOT_EYES_STRINGIFY(TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT) " LED physical transport");
    printf("[playlist] %u animations loaded\n", (unsigned)g_tasbot_animation_playlist_count);
    printf("[rainbow] mode=%u step_ms=%u\n",
           (unsigned)TASBOT_EYES_RAINBOW_MODE,
           (unsigned)TASBOT_EYES_RAINBOW_STEP_MS_CLAMPED);
        printf("[blink] max_count=%u delay_range_ms=%u..%u\n",
            (unsigned)TASBOT_EYES_BLINK_MAX_COUNT,
            (unsigned)TASBOT_EYES_BLINK_DELAY_LOW_MS,
            (unsigned)TASBOT_EYES_BLINK_DELAY_HIGH_MS);
        printf("[recolor] enabled=%u palette_size=%u\n",
            (unsigned)TASBOT_EYES_RANDOMIZE_MONOCHROME,
            (unsigned)TASBOT_EYES_RECOLOR_PALETTE_COUNT);
        printf("[gamma] enabled=%u curve=%s\n",
            (unsigned)TASBOT_EYES_GAMMA_ENABLE,
            gamma_curve_name());

    /* Enable the watchdog NOW — before hw_led_init() — so that a deadlock in
       the PIO clear-frame push (which runs inside hw_led_init) triggers an
       automatic reset.  The boot delay above is ~3 s; 8 s gives ample margin
       before the first watchdog_update() in the animation loop below. */
    watchdog_enable(8000, true);

    if (!hw_led_init()) {
        puts("hw_led_init failed");
        return 1;
    }

    gpio_init(TASBOT_EYES_BRIGHTNESS_BUTTON_PIN);
    gpio_set_dir(TASBOT_EYES_BRIGHTNESS_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(TASBOT_EYES_BRIGHTNESS_BUTTON_PIN);

    brightness_level_index = brightness_default_level_index(kBrightnessLevels, sizeof(kBrightnessLevels) / sizeof(kBrightnessLevels[0]));

    {
        uint8_t brightness_legacy = kBrightnessLevels[brightness_level_index];
        uint8_t brightness_percent = brightness_legacy_to_percent(brightness_legacy);

        hw_led_set_brightness_percent(brightness_percent);
        printf("[brightness] legacy=%u/255 mapped=%u%% (button=%u GPIO%u, source %s)\n",
               (unsigned)brightness_legacy,
               (unsigned)hw_led_get_brightness_percent(),
               (unsigned)TASBOT_EYES_BRIGHTNESS_BUTTON_ENABLE,
               (unsigned)TASBOT_EYES_BRIGHTNESS_BUTTON_PIN,
               TASBOT_EYES_BRIGHTNESS_BUTTON_PIN_SOURCE);
    }

    if (g_tasbot_animation_playlist_count < kPlaylistFirstAnimationIndex) {
        puts("g_tasbot_animation_playlist_count must include startup/base/blink assets");
        return 1;
    }

    if (g_tasbot_animation_playlist_count > kPlaylistFirstAnimationIndex) {
        playlist_animation_index = kPlaylistFirstAnimationIndex;
    } else {
        /* If only startup/base/blink are available, idle on base. */
        playlist_animation_index = kBaseAnimationIndex;
    }

    blink_prng_state = (uint32_t)(time_us_64() ^ 0xA5A55A5Au);
    if (blink_prng_state == 0u) {
        blink_prng_state = 1u;
    }

    rainbow_last_step_us = time_us_64();
    printf("[playlist] starting with: %s\n", g_tasbot_animation_playlist[playlist_animation_index]->name);

    while (true) {
        if (blink_state == BLINK_STATE_BASE) {
            blink_wait_delay_ms = blink_random_delay_ms(&blink_prng_state);
            blink_wait_until_us = time_us_64() + ((uint64_t)blink_wait_delay_ms * 1000u);
            blink_wait_selection_count += 1u;
            base_frame_index = 0u;
            blink_state = BLINK_STATE_WAIT;
#if TASBOT_EYES_RUNTIME_VERBOSE_LOGS
            printf("[blink] wait #%lu delay_ms=%u remaining=%u\n",
                   (unsigned long)blink_wait_selection_count,
                   (unsigned)blink_wait_delay_ms,
                   (unsigned)blink_burst_remaining);
#endif
        }

        switch (blink_state) {
            case BLINK_STATE_ANIMATION:
                anim = g_tasbot_animation_playlist[playlist_animation_index];
                active_frame_index = &playlist_frame_index;
                break;
            case BLINK_STATE_WAIT:
                anim = g_tasbot_animation_playlist[kBaseAnimationIndex];
                active_frame_index = &base_frame_index;
                break;
            case BLINK_STATE_BURST:
                anim = g_tasbot_animation_playlist[kBlinkAnimationIndex];
                active_frame_index = &blink_frame_index;
                break;
            default:
                puts("invalid blink scheduler state");
                return 1;
        }

        if (anim->frame_count == 0u) {
            puts("animation frame_count must be greater than zero");
            return 1;
        }

        if (anim != recolor_anim) {
            recolor_anim = anim;
            recolor_active_for_animation = false;

            if (TASBOT_EYES_RANDOMIZE_MONOCHROME != 0u && animation_is_monochrome(anim)) {
                recolor_selected = recolor_pick_palette_color(&blink_prng_state);
                recolor_active_for_animation = true;
                recolor_applied_count += 1u;
#if TASBOT_EYES_RUNTIME_VERBOSE_LOGS
                printf("[recolor] applied=%lu skipped=%lu animation=%s color=#%06lx\n",
                       (unsigned long)recolor_applied_count,
                       (unsigned long)recolor_skipped_count,
                       anim->name,
                       (unsigned long)(recolor_selected & 0x00ffffffu));
#endif
            } else {
                recolor_skipped_count += 1u;
#if TASBOT_EYES_RUNTIME_VERBOSE_LOGS
                printf("[recolor] applied=%lu skipped=%lu animation=%s (passthrough)\n",
                       (unsigned long)recolor_applied_count,
                       (unsigned long)recolor_skipped_count,
                       anim->name);
#endif
            }
        }

        delay_ms = tasbot_embedded_animation_frame_delay_ms(anim, *active_frame_index);

        if (!tasbot_embedded_animation_load_frame(anim, *active_frame_index, &logical_frame)) {
            puts("tasbot_embedded_animation_load_frame failed");
            return 1;
        }

        /* Deterministic transform order: decode -> layout -> recolor -> rainbow -> gamma -> LED present. */
        tasbot_layout_blit_frame(&logical_frame, physical_leds, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT);

        if (recolor_active_for_animation) {
            recolor_non_black_pixels(physical_leds, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT, recolor_selected);
        }

        if (TASBOT_EYES_RAINBOW_MODE != 0u) {
            uint64_t now_us = time_us_64();
            tasbot_color_t rainbow_rgb;

            rainbow_update_hue(&rainbow_hue, &rainbow_last_step_us, now_us);
            rainbow_rgb = rainbow_hue_to_rgb888(rainbow_hue);
            rainbow_remap_white_pixels(physical_leds, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT, rainbow_rgb);
        }

        if (TASBOT_EYES_GAMMA_ENABLE != 0u) {
            gamma_apply_to_leds(physical_leds, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT);
        }

        {
            bool button_pressed = gpio_get(TASBOT_EYES_BRIGHTNESS_BUTTON_PIN) == 0u;
            uint64_t now_us = time_us_64();

            if (TASBOT_EYES_BRIGHTNESS_BUTTON_ENABLE != 0u &&
                button_pressed && !button_was_pressed &&
                now_us >= button_debounce_until_us) {
                uint8_t brightness_legacy;
                uint8_t brightness_percent;

                brightness_level_index = (uint8_t)((brightness_level_index + 1u) % (sizeof(kBrightnessLevels) / sizeof(kBrightnessLevels[0])));
                brightness_legacy = kBrightnessLevels[brightness_level_index];
                brightness_percent = brightness_legacy_to_percent(brightness_legacy);
                hw_led_set_brightness_percent(brightness_percent);
                button_debounce_until_us = now_us + TASBOT_EYES_BRIGHTNESS_BUTTON_DEBOUNCE_US;
                printf("[brightness] legacy=%u/255 mapped=%u%%\n",
                       (unsigned)brightness_legacy,
                       (unsigned)hw_led_get_brightness_percent());
            }

            button_was_pressed = button_pressed;
        }

        if (!hw_led_present_rgb888(physical_leds, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT, &metrics)) {
            puts("hw_led_present_rgb888 failed");
            return 1;
        }

        *active_frame_index = (uint16_t)(*active_frame_index + 1u);
        if (*active_frame_index >= anim->frame_count) {
            *active_frame_index = 0u;

            if (blink_state == BLINK_STATE_ANIMATION) {
                if (TASBOT_EYES_BLINK_MAX_COUNT == 0u || g_tasbot_animation_playlist_count <= kPlaylistFirstAnimationIndex) {
                    playlist_animation_index = playlist_next_animation_index(playlist_animation_index, g_tasbot_animation_playlist_count);
#if TASBOT_EYES_RUNTIME_VERBOSE_LOGS
                    printf("[playlist] switching to: %s\n", g_tasbot_animation_playlist[playlist_animation_index]->name);
#endif
                } else {
                    blink_burst_remaining = blink_random_burst_count(&blink_prng_state);
                    blink_burst_selection_count += 1u;
                    blink_state = BLINK_STATE_BASE;
#if TASBOT_EYES_RUNTIME_VERBOSE_LOGS
                    printf("[blink] burst #%lu count=%u\n",
                           (unsigned long)blink_burst_selection_count,
                           (unsigned)blink_burst_remaining);
#endif
                }
            } else if (blink_state == BLINK_STATE_BURST) {
                if (blink_burst_remaining > 0u) {
                    blink_burst_remaining = (uint8_t)(blink_burst_remaining - 1u);
                }

                if (blink_burst_remaining > 0u) {
                    blink_state = BLINK_STATE_BASE;
                } else {
                    blink_state = BLINK_STATE_ANIMATION;
                    playlist_animation_index = playlist_next_animation_index(playlist_animation_index, g_tasbot_animation_playlist_count);
#if TASBOT_EYES_RUNTIME_VERBOSE_LOGS
                    printf("[playlist] switching to: %s\n", g_tasbot_animation_playlist[playlist_animation_index]->name);
#endif
                }
            }
        }

        if (blink_state == BLINK_STATE_WAIT && time_us_64() >= blink_wait_until_us) {
            blink_state = BLINK_STATE_BURST;
            blink_frame_index = 0u;
        }

        watchdog_update();
        sleep_ms(delay_ms);
    }
}

