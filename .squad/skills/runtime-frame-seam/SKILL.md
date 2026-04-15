---
name: "runtime-frame-seam"
description: "Split portable frame generation from hardware pixel transport during embedded runtime migration"
domain: "firmware"
confidence: "high"
source: "earned"
---

## Context

Use this when a legacy renderer mixes product-specific layout logic with platform-specific LED, timing, or I/O code and the fastest safe migration path is to prove a deterministic hardware smoke test first.

## Patterns

- Define a portable logical frame type first, using local fixed-width color types instead of driver-owned typedefs.
- Keep product geometry and logical-to-physical mapping in portable code if it describes the device layout rather than the transport peripheral.
- Make the firmware boundary a narrow sink such as `hw_led_present_rgb888(const pixels, count)`.
- Prove the seam with a short deterministic phase sequence before porting assets, networking, or filesystem behavior.

## Examples

- `pico_build\src\portable\runtime_types.h`
- `pico_build\src\portable\tasbot_layout.c`
- `pico_build\src\portable\smoke_patterns.c`
- `pico_build\src\firmware\hw_led_stub.c`

## Anti-Patterns

- Copying the whole legacy runtime before stripping `pthread`, `usleep`, sockets, and filesystem calls.
- Letting `ws2811_led_t` or other platform driver types leak into new portable headers.
- Hiding timing decisions inside portable animation code instead of keeping them in the firmware loop.
