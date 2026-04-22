# F3: Offline Assets and Playback Integration

**Status:** In progress  
**Scope:** Phase 3 of 4  
**Depends On:** F2

## Goal

Define and implement the offline asset pipeline, compare the legacy and submodule animation sets, and feed the chosen results into Pico playback.

This is the MVP demo phase. Its job is to get `colorful.gif` playing on the physical array as quickly and honestly as possible.

## Asset Sources

| Source | Path | Role |
|---|---|---|
| Legacy local GIF dump | `gifs\` | Historical reference set; ignored in git |
| Tracked animation submodule | `external\TASBot-eye-animations\gifs\` | Maintained comparison source |
| Editable source art | `external\TASBot-eye-animations\aseprite\` | Upstream working files for future refinement |

## Planned Pipeline

1. Inventory both GIF sets.
2. Compare overlapping animation names such as `base.gif`, `blink.gif`, and `colorful.gif`.
3. Decide which source file becomes canonical per animation.
4. Convert the chosen files offline into generated assets under `pico_build\assets\generated\`.
5. Link those generated assets into the UF2 for playback.

## Current Slice

The first implementation slice is now centered on `colorful.gif`:

1. `pico_build\tools\generate-gif-asset.ps1` resolves the canonical source by preferring the
   submodule copy and falling back to root `gifs\` only when needed.
2. The converter emits `colorful_asset.generated.h` plus `colorful_asset.metadata.txt` under
   `pico_build\assets\generated\`.
3. The Pico firmware links that generated contract and loops the embedded `colorful.gif` frames
   from flash-backed data over the WS2812B PIO transport.
4. The current `colorful.gif` art is 28x8 source content, which now matches the active display
   contract exactly. Firmware still transmits the full physical 8x32 chain, but leaves the final
   four columns dark so the live transport buffer carries 224 active LEDs plus a 32-pixel inactive tail.

### Source Selection Policy

1. Prefer `external\TASBot-eye-animations\gifs\` when the submodule is available and contains the requested animation.
2. Fall back to root `gifs\` when the submodule is unavailable or the requested animation does not exist there.
3. Emit a build-time error when neither source contains the requested animation.

For the current `colorful.gif` slice, the canonical source is
`external\TASBot-eye-animations\gifs\others\colorful.gif` when the submodule is available.

## Deliverables

- asset inventory and comparison notes for legacy vs. submodule sources
- offline converter for GIF to embedded frame data
- generated headers or blobs in `pico_build\assets\generated\`
- LED driver hookup capable of rendering the generated frame format
- `colorful.gif` playback as the first end-to-end validation target

## Acceptance Criteria

1. The pipeline does not depend on runtime filesystem access.
2. The docs describe how both animation source pools are compared before generation.
3. `colorful.gif` has a documented canonical source, generated output format, and reproducible build path.
4. Playback uses generated assets compiled into the firmware image.
5. A flashed UF2 loops `colorful.gif` on the real panel with stable colors, correct
   top-left-first column-serpentine ordering across the active 8x28 window, intentionally dark
   unused columns, and no obvious frame corruption for a meaningful
   demo run.
6. Submodule absence is handled intentionally: the build either falls back to `gifs\` for `colorful.gif` or fails with a clear error instead of silently selecting an unknown source.
7. Broader assets such as startup, base, and blink are planned next, but they do not block the `colorful.gif` MVP gate.

## Risks

| Risk | Mitigation |
|---|---|
| Two GIF sets diverge silently | Make comparison an explicit pipeline step, not tribal knowledge |
| Generated assets bloat flash usage | Measure size during conversion and prune or compress early |
| LED playback work starts before format is stable | Lock the generated asset contract before driver integration |

## Exit State

F3 is done when `colorful.gif` is embedded into the UF2 and visibly playing on the target hardware. That is the first honest end-to-end demo milestone for the Pico migration.

## Handoff

F4 validates the broader asset set, longer timing runs, and polish work after the `colorful.gif` demo is already in hand.
