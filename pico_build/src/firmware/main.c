#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "animation_registry.h"
#include "board.h"
#include "embedded_animation.h"
#include "hw_led.h"
#include "tasbot_layout.h"

int main(void)
{
    /* Keep large buffers in static storage — together they consume ~1.9 KB,
       which exceeds the default 2 KB Pico stack and causes a hard fault. */
    static tasbot_frame_t logical_frame;
    static tasbot_color_t physical_leds[TASBOT_EYES_PHYSICAL_LED_PIXEL_COUNT];
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

    /* Enable the watchdog NOW — before hw_led_init() — so that a deadlock in
       the PIO clear-frame push (which runs inside hw_led_init) triggers an
       automatic reset.  The boot delay above is ~3 s; 8 s gives ample margin
       before the first watchdog_update() in the animation loop below. */
    watchdog_enable(8000, true);

    if (!hw_led_init()) {
        puts("hw_led_init failed");
        return 1;
    }

    if (g_tasbot_animation_playlist_count < 2u) {
        puts("g_tasbot_animation_playlist_count must be at least 2 (boot + cycle)");
        return 1;
    }

    /* Skip startup animation — jump straight into the cycling playlist. */
    animation_index = 1u;
    frame_index = 0u;
    printf("[playlist] starting with: %s\n", g_tasbot_animation_playlist[animation_index]->name);

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

        frame_index = (uint16_t)(frame_index + 1u);
        if (frame_index >= anim->frame_count) {
            frame_index = 0u;
            /* Cycle through animations 1..N-1, wrapping back to 1 (not 0). */
            animation_index = (uint16_t)(((animation_index - 1u + 1u) % (g_tasbot_animation_playlist_count - 1u)) + 1u);
            printf("[playlist] switching to: %s\n", g_tasbot_animation_playlist[animation_index]->name);
        }

        watchdog_update();
        sleep_ms(delay_ms);
    }
}

