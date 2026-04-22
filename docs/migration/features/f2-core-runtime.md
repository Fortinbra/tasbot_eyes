# F2: Core Runtime and Pico Event Loop

**Status:** Planned  
**Scope:** Phase 2 of 4  
**Depends On:** F1

## Goal

Move animation control flow from the legacy host assumptions into a Pico-safe runtime model without touching the original root sources.

## Scope

- copy or adapt portable logic into `pico_build\src\portable\`
- remove direct dependence on POSIX threads, host sleep calls, and desktop-only control flow
- build a firmware shell in `pico_build\src\firmware\` that owns timers, tick cadence, and board startup
- prove the Pico shell can render deterministic frames on the real array before full asset work begins

## Deliverables

- portable animation, color, palette, stack, and mapping code staged under `pico_build\src\portable\`
- Pico-facing runtime entry point in `pico_build\src\firmware\main.c`
- explicit board contract in `pico_build\src\firmware\board.h` for protocol, GPIO pin, pixel count, and smoke-test cadence
- timer-driven or polling-driven tick API for frame advancement
- clear seam between portable logic and hardware output hooks
- a hardware smoke test that can render a known frame sequence or test pattern on the active 8x28 area of the physical 8x32 matrix

## Runtime Seam Map

### Smallest practical copy batch

Start with code that is already deterministic and does not require host services:

1. **Copy first now**
   - `color.c/.h` as-is once the Pico target needs HSV fade or gamma math
   - TASBot layout data and mapping helpers from `tasbot.c`
   - blink timing math from `tasbot.c` after it is rewritten around explicit elapsed milliseconds instead of `usleep()`
2. **Do not copy yet**
   - `tasbot.c` wholesale, because it is still fused to GIF loading, stack/file discovery, render buffer globals, and host sleep calls
   - `stack.c/.h`, because the current implementation pulls in `arguments.h` just for verbose logging
   - `palette.c/.h`, because `ws2811_led_t` keeps the API tied to the Raspberry Pi LED driver
3. **Leave behind entirely for Pico runtime**
   - `main.c`, `network.c/.h`, `filesystem.c/.h`, and `led.c/.h`

### Pico blockers by header or API

| Legacy seam | Why it blocks Pico portability | Required replacement |
|---|---|---|
| `led.h` -> `<ws2811/ws2811.h>` and `ws2811_led_t` | Raspberry Pi DMA driver type leaks into unrelated modules | local `rgb888` color typedef plus `hw_led_present_rgb888()` |
| `led.h`, `network.h`, `tasbot.h` -> `<pthread.h>` | thread ownership is scattered through rendering, hue fade, and UDP control | single firmware-owned loop with explicit phase/tick state |
| `main.c`, `tasbot.c`, `led.c` -> `usleep()` | host sleep calls hide cadence inside portable logic | elapsed-ms tick/update functions owned by firmware |
| `filesystem.h/.c` -> `opendir()`, `readdir()`, `access()` | runtime asset discovery cannot exist on-device | offline-generated assets in `pico_build\assets\generated\` |
| `gif.h/.c` -> `<gif_lib.h>` and `DGif*` | runtime GIF decode is out of scope for firmware | preconverted frame blobs in F3 |
| `network.h/.c` -> `<sys/socket.h>`, `<netinet/in.h>` | UDP transport is not part of the deterministic smoke-test path | defer to later serial/Wi-Fi control decision |

### Exact boundary to create next

The next stable boundary is:

`portable runtime/test pattern` -> `logical 28x8 active frame` -> `portable column-serpentine matrix mapper` -> `256-pixel RGB888 transport buffer with a dark 32-pixel tail` -> `firmware hw_led driver`

That contract keeps panel geometry explicit and makes the hardware layer own only board bring-up plus pixel transport. For the current hardware slice, `board.h` is the single place that declares both the active display contract (8 rows, 28 columns, 224 active LEDs) and the physical harness contract (8 rows, 32 columns, 256 chained LEDs, with the last four columns intentionally dark), while the transport stays a Pico PIO-driven WS2812B path that consumes the mapped 256-pixel RGB888 buffer. The smoke test should keep proving four deterministic phases: left eye, right eye, nose field, and alignment markers.

## Acceptance Criteria

1. Portable runtime code compiles without desktop-only headers.
2. The Pico shell owns the main loop and timing cadence.
3. Root application sources remain unchanged.
4. The firmware can render a deterministic frame source on the active 8x28 region of the physical 8x32 array, even if it is only a short smoke-test pattern at this phase.
5. The event-loop model is documented well enough for generated-asset playback integration in F3.

## Constraints

- No filesystem assumptions.
- No runtime GIF decoding on device.
- No hidden platform work inside copied portable files.
- Do not block on full Raspberry Pi behavioral parity before the first hardware playback proof.

## Risks

| Risk | Mitigation |
|---|---|
| Timing drift changes blink feel | Use explicit elapsed-time tick APIs and validate against known sequences |
| Portable code still hides host dependencies | Audit includes and isolate hardware hooks early |
| Runtime and hardware work get mixed together | Keep `src\portable\` and `src\firmware\` responsibilities separate |

## Exit State

F2 is done when the Pico shell can drive the board and show a deterministic pattern or short generated frame sequence on the actual LED array. This is the seam-validation phase, not the final animation-parity phase.

## Handoff

F3 starts after F2 proves the runtime contract can feed frame data into the LED layer on hardware, so the asset work can focus on real `colorful.gif` playback instead of first-contact board debugging.
