# F02: -w and -W Rainbow White-Pixel Remap

## Status

- Done.
- Implemented in firmware with white-pixel remap, hue stepping, and static step clamp.

## Legacy Intent

`-w` enables rainbow mode. `-W <ms>` sets hue step interval (clamped 10..1000 ms). In this mode, pure white pixels are remapped to HSV(hue, 1, 1) while non-white pixels remain unchanged.

## Firmware Scope

Add optional frame post-process stage:

- detect pure white pixels in frame output
- replace them with hue-driven RGB color
- advance hue based on static step interval for now

## Static Firmware Constants (Current)

Define these as constants (for example in `board.h` or a dedicated firmware feature header):

- `rainbow_mode` (bool)
- `rainbow_step_ms` (uint16, clamp 10..1000)

## Future Config Surface

- compile-time override macros
- optional runtime config channel

Runtime state:

- `rainbow_hue` (0..360 or 0..255 domain)
- `rainbow_last_step_us`

## Implementation Checklist

1. Add static constants and validation clamp for `rainbow_step_ms`.
2. Add hue clock update function using monotonic time.
3. Implement white-pixel detect (`R=255,G=255,B=255`) in post-process pass.
4. Convert hue to RGB and substitute only white pixels.
5. Ensure stage ordering with other transforms is documented and deterministic.

## Independent Test Plan

### Unit Tests

1. `rainbow_step_ms` clamps to [10, 1000].
2. White pixel is remapped; non-white pixel unchanged.
3. Hue progression wraps correctly at domain max.

### On-Board Validation (Firmware)

1. `rainbow_mode=false` with white-rich animation:
- verify output remains original colors

2. `rainbow_mode=true`, `rainbow_step_ms=1000`:
- verify white regions shift color once per second
- verify no color shift on non-white regions

3. `rainbow_mode=true`, `rainbow_step_ms=10`:
- verify fast smooth hue cycling without dropped frame instability

### Pass Criteria

- Remap affects only white pixels.
- Hue cadence matches configured step interval within expected timer jitter.
- No measurable frame loop stalls introduced by post-process.

## Out of Scope

- remapping near-white thresholds
- per-pixel saturation/value effects
