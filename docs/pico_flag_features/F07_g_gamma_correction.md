# F07: -g Gamma Correction

## Status

- Done.
- Implemented with LUT-based gamma stage, curve selection constants, boot diagnostics, and bypass guard when disabled.

## Legacy Intent

`-g` toggles gamma correction in color translation path. Legacy help warns this feature is broken.

## Firmware Scope

Implement a reliable optional gamma correction stage for outgoing RGB values.

## Static Firmware Constants (Current)

Define these as constants (for example in `board.h` or a dedicated firmware feature header):

- `gamma_enable` (bool)
- `gamma_curve` (enum: `legacy22`, `srgb`, `custom_lut`)

## Future Config Surface

- compile-time override macros
- optional runtime config channel

Runtime assets:

- 256-entry LUT per channel (or shared LUT)

## Implementation Checklist

1. Add LUT-based gamma transform utility with deterministic outputs.
2. Add constant toggle and curve selection.
3. Apply gamma stage in post-process pipeline after recolor/rainbow decisions.
4. Add quick diagnostics print of selected gamma curve.
5. Add guardrails to bypass gamma when disabled.

## Independent Test Plan

### Unit Tests

1. LUT monotonicity: output never decreases for increasing input.
2. Endpoint checks: `0 -> 0`, `255 -> 255`.
3. Toggle behavior: when disabled, output equals input.

### On-Board Validation (Firmware)

1. `gamma_enable=false` reference capture:
- record baseline checksum/visual for fixed animation

2. `gamma_enable=true`:
- verify predictable change in mid-tones and matching checksum expectations

3. Long-run stability:
- verify no added crashes or watchdog events over extended playback

### Pass Criteria

- Gamma transform is mathematically stable and deterministic.
- Disabled mode is transparent passthrough.
- Enabled mode produces expected perceptual dimming in darker ranges.

## Out of Scope

- color calibration against specific camera sensor pipelines
