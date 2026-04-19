#include <stdio.h>

#include "pico/stdlib.h"

#include "animation_registry.h"
#include "board.h"
#include "embedded_animation.h"
#include "hw_led.h"
#include "tasbot_layout.h"

int main(void)
{
    tasbot_frame_t logical_frame;
    tasbot_color_t physical_leds[TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT];
    hw_led_metrics_t metrics;
    const tasbot_embedded_animation_t* anim;
    uint32_t ready_emit = 0u;
    uint16_t animation_index = 0u;
    uint16_t frame_index = 0u;
    uint16_t delay_ms;

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

    if (!hw_led_init()) {
        puts("hw_led_init failed");
        return 1;
    }

    if (g_tasbot_animation_playlist_count == 0u) {
        puts("g_tasbot_animation_playlist_count must be greater than zero");
        return 1;
    }

    while (true) {
        anim = g_tasbot_animation_playlist[animation_index];

        if (anim->frame_count == 0u) {
            puts("animation frame_count must be greater than zero");
            return 1;
        }

        delay_ms = tasbot_embedded_animation_frame_delay_ms(anim, frame_index);

        if (!tasbot_embedded_animation_load_frame(anim, frame_index, &logical_frame)) {
            puts("tasbot_embedded_animation_load_frame failed");
            return 1;
        }

        tasbot_layout_blit_frame(&logical_frame, physical_leds, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT);

        if (!hw_led_present_rgb888(physical_leds, TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT, &metrics)) {
            puts("hw_led_present_rgb888 failed");
            return 1;
        }

        printf("[asset] frame=%u/%u delay=%u ms lit=%lu checksum=%08lx\n",
               (unsigned)(frame_index + 1u),
               (unsigned)anim->frame_count,
               (unsigned)delay_ms,
               (unsigned long)metrics.last_lit_pixels,
               (unsigned long)metrics.last_checksum);

        frame_index = (uint16_t)(frame_index + 1u);
        if (frame_index >= anim->frame_count) {
            frame_index = 0u;
            animation_index = (uint16_t)((animation_index + 1u) % g_tasbot_animation_playlist_count);
            printf("[playlist] switching to: %s\n", g_tasbot_animation_playlist[animation_index]->name);
        }

        sleep_ms(delay_ms);
    }
}

