---
name: "ws2812-pio-transport"
description: "Implement an isolated Pico SDK WS2812B transport behind an RGB888 firmware seam"
domain: "firmware"
confidence: "high"
source: "earned"
---

## Context

Use this when a Pico firmware slice has already established a clean logical-frame-to-RGB888 seam and now needs a real WS2812B hardware path without dragging legacy Raspberry Pi driver code into the new tree.

## Patterns

### Keep the seam stable

- Leave the portable layer producing plain RGB888 transport pixels.
- Repack to WS2812 GRB ordering inside the firmware driver only.
- Keep metrics, checksum, and lit-pixel accounting in the same driver pass so transport proof stays cheap.

### Use standard Pico PIO structure

- Keep a readable `ws2812.pio` source file in `src\firmware\`.
- Initialize the state machine with `pio_claim_free_sm_and_add_program_for_gpio_range()` and the standard WS2812 timing constants.
- Wait for TX FIFO drain, then hold a reset window before returning so back-to-back frames do not violate the latch gap.

### Prefer explicit board pin resolution

- Resolve the WS2812 data pin from board-provided macros first.
- Add one hard fallback pin only when hardware direction is already known and you need the build to stay usable across boards.
- For Plasma 2350-class boards, GPIO 15 / `PLASMA2350_DATA_PIN` is the important seam to preserve.

### Make the build honest on Windows

- If local SDK `pioasm` source builds are flaky because the host C++ environment is incomplete, generate `ws2812.pio.h` once with the prebuilt `pioasm.exe` and track it beside the `.pio` file.
- Remove the live `pico_generate_pio_header()` dependency from CMake in that case, but keep the source `.pio` checked in so the transport stays reviewable.

## Examples

- `pico_build\src\firmware\hw_led_pio.c`
- `pico_build\src\firmware\ws2812.pio`
- `pico_build\src\firmware\ws2812.pio.h`
- `pico_build\src\firmware\board.h`

## Anti-Patterns

- Leaving a metrics-only stub in place once hardware protocol is known
- Letting GRB byte-order knowledge leak back into portable mapping code
- Forcing the whole firmware build to depend on rebuilding SDK host tools when a checked-in generated header would keep the build reproducible
