---
name: "panel-contract-migration"
description: "Fix firmware that still targets a legacy sparse LED layout after the real panel contract changed"
domain: "firmware-migration"
confidence: "high"
source: "earned"
---

## Context
Use this when a migrated firmware path still carries old pixel-count or mapper assumptions from a legacy layout, but the shipped hardware is a full rectangular matrix. Typical symptom: only part of the panel lights, and the visible image is scrambled because the transport buffer and physical mapper disagree with the real harness.

## Patterns
- Change the shared panel contract first: width, height, and total pixel count must match in runtime types, board contract headers, transport buffers, and present/clear loops.
- When the visible art uses only part of a larger wired panel, split the contract into an active frame size and a physical chain size instead of forcing one number to mean both.
- Replace legacy sparse lookup tables with direct serpentine math when the real panel is a complete matrix. For a top-left-first column-serpentine panel: even columns index downward, odd columns index upward.
- If existing assets are smaller than the new live panel, clear the full destination frame first and then blit the smaller asset into a defined anchor position instead of rejecting it or shrinking the transport buffer back down.
- For a dark inactive tail, keep sending the full physical chain length every frame and zero-fill the unused indices so stale colors cannot persist on the unwired or intentionally hidden section.
- Rebuild the real firmware artifact and verify both the physical-index coverage (`0..N-1`, unique) and the output artifact hash so the next flash target is unambiguous.

## Examples
- `pico_build\src\portable\runtime_types.h` widened from the legacy 154-pixel assumption to a 32x8 / 256-pixel contract.
- `pico_build\src\portable\tasbot_layout.c` switched from a sparse TASBot lookup table to direct column-serpentine mapping for the live panel.
- `pico_build\src\portable\embedded_animation.c` clears the 32x8 frame and copies the current 28x8 `colorful.gif` asset into it so hardware validation can proceed before source art changes.
- `pico_build\src\firmware\board.h` can carry both `active 8x28 / 224` and `physical 8x32 / 256` so a larger WS2812 chain stays electrically correct while the final four columns remain dark.

## Anti-Patterns
- Keeping the old pixel count in `board.h` while only changing the mapper formula.
- Reusing a single `PIXEL_COUNT` macro for both the visible frame and the physical WS2812 chain when those numbers intentionally differ.
- Preserving a legacy sparse occupancy table for hardware that is actually a full rectangular matrix.
- Refusing to render smaller assets on a larger panel when simple padding would preserve visible behavior and unblock hardware validation.
