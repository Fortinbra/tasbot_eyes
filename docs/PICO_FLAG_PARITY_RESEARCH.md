# Pico Feature Research: Legacy CLI Flag Parity

Date: 2026-05-12
Branch: work/2026-05-12

## Goal

Document what the legacy desktop flags mean and what is required to support equivalent behavior in `pico_build` firmware.

## Current Implementation Assumptions

- Feature values are static firmware constants in this phase.
- Validation is performed on physical hardware (on-board validation).
- Compile-time and runtime configuration mechanisms are future phases.

Target legacy flag set:

- `-U`
- `-w`
- `-W 1000`
- `-B 3-3-5`
- `-c`
- `-u`
- `-b 8`
- `-g`

## Legacy Behavior (Desktop C Runtime)

### Combined Example

Legacy invocation:

```bash
-U -w -W 1000 -B 3-3-5 -c -u -b 8 -g
```

Effective meaning in legacy runtime:

- `-U`: enable WLED UDP realtime control for center/nose LEDs.
- `-w`: enable rainbow mode (white pixels are hue-cycled).
- `-W 1000`: set rainbow hue fade step interval to 1000 ms.
- `-B 3-3-5`: blink pattern = up to 3 blinks, 3-5 seconds between blinks.
- `-c`: randomize color for monochrome animations.
- `-u`: skip startup animation.
- `-b 8`: set LED brightness to 8 (0-255 scale).
- `-g`: enable gamma correction pipeline.

### Per-Flag Semantics and Source of Truth

1. `-U` (realtime control)
- Parse and toggle: `arguments.c`
- Runtime effect:
  - Starts realtime UDP thread from `main.c`.
  - Masks center/nose pixels out of normal animation renderer (`tasbot.c`, `showFrame`) so network stream owns those pixels.
  - Uses UDP packet mode `2` stream in `network.c` (`receiveRealtimeControl`).
- Caveat: when enabled, verbose logging is forcibly disabled in `main.c` to reduce overhead.

2. `-w` + `-W <ms>` (rainbow mode)
- Parse and toggle: `arguments.c`
- Runtime effect:
  - Starts hue update thread in `tasbot.c` (`startHueThread`, `fadeHue`).
  - In `led.c`, any pixel equal to pure white (`0xFFFFFF`) is replaced with HSV(hue,1,1) and then translated to LED color.
- `-W` bounds: clamped to `10..1000` ms.

3. `-B x-y-z` (blink pattern)
- Parse and validation: `arguments.c`
  - Single-digit format expected (`digit-digit-digit`), max 9 per field.
  - If min > max, values are swapped.
- Runtime effect:
  - `maxBlinks` controls random blink count via `getBlinkAmount()`.
  - `minTimeBetweenBlinks`/`maxTimeBetweenBlinks` control random delay via `getBlinkDelay()`.
  - Values are converted seconds -> milliseconds once during startup (`main.c`, `initBlinking`).
- For `-B 3-3-5`:
  - Max blink repeats between animations: `3`
  - Delay range between blinks: `3000..5000 ms`

4. `-c` (random color for monochrome)
- Parse and toggle: `arguments.c`
- Runtime effect:
  - During animation playback, monochrome animations can be remapped to a random palette color.
  - Decision logic in `tasbot.c` (`playAnimation`), applied during frame output.

5. `-u` (skip startup animation)
- Parse and toggle: `arguments.c`
- Runtime effect:
  - Prevents startup GIF playback in first iteration (`main.c`, `tasbotsEyes`).

6. `-b <0..255>` (brightness)
- Parse and clamp: `arguments.c`
- Runtime effect:
  - `brightness` is passed into LED driver init (`led.c`) as channel brightness.

7. `-g` (gamma correction)
- Parse and toggle: `arguments.c`
- Runtime effect:
  - Enables gamma path in color translation (`led.c`, `translateColor`, called from render paths).
- Caveat: help text explicitly marks this as broken in legacy runtime.

## Pico Firmware: Current State vs Requested Flags

Current firmware baseline (`pico_build/src/firmware/main.c`) is fixed-function playlist playback with button-controlled brightness levels.

### Status Matrix

1. `-u` skip startup
- Status: Partially present, currently hardcoded.
- Evidence: firmware currently starts at playlist index `1` (startup skipped unconditionally).
- Gap: no runtime-configurable mode to include startup when desired.

2. `-b 8` brightness
- Status: Partially present with different control model.
- Evidence:
  - Global brightness percent is supported in `hw_led_pio.c`.
  - Board defaults include 8% as level 0 in `board.h`.
  - Runtime brightness cycles via hardware button, not argument/profile.
- Gap: no direct numeric input equivalent to legacy `-b`.

3. `-B 3-3-5` blink pattern
- Status: Not implemented as a scheduling concept.
- Evidence: firmware loops linear playlist; no stochastic blink scheduler equivalent to desktop `getBlinkDelay()/getBlinkAmount()`.
- Gap: requires explicit blink-state machine and timer/randomization policy.

4. `-w` and `-W`
- Status: Not implemented.
- Evidence: no hue thread/state or white-pixel remap pipeline in current Pico renderer path.
- Gap: requires dynamic frame post-process pass and a hue clock.

5. `-c`
- Status: Not implemented.
- Evidence: no monochrome detection + palette remap path in portable/firmware playback flow.
- Gap: requires metadata or runtime detection and recolor pass before LED submit.

6. `-g`
- Status: Not implemented.
- Evidence: no gamma toggle/lookup in current `hw_led_pio.c` output path.
- Gap: add gamma LUT transform in output pipeline (or asset-preprocessed path).

7. `-U`
- Status: Not implemented.
- Evidence: no UDP realtime control listener in Pico firmware.
- Gap: requires lwIP/UDP receive path and nose-region compositing policy.

## Firmware Implementation Requirements (Proposed)

Because Pico firmware has no shell argv model, implement these values as static firmware constants for the current phase.

Future phases can add compile-time override macros and optional runtime control channels.

## 1) Configuration Surface

Add static constants (example fields):

- `bool enable_realtime_nose` (`-U`)
- `bool rainbow_mode` (`-w`)
- `uint16_t rainbow_step_ms` (`-W`)
- `uint8_t blink_max_count` (`-B x`)
- `uint16_t blink_min_delay_ms` (`-B y`)
- `uint16_t blink_max_delay_ms` (`-B z`)
- `bool randomize_monochrome` (`-c`)
- `bool skip_startup` (`-u`)
- `uint8_t brightness_percent` (mapped from legacy `-b`)
- `bool gamma_enable` (`-g`)

Constant source options:

- Compile-time macros in `board.h` (quickest).
- Build-time generated header from JSON/TOML config (best long-term).
- Optional serial command channel for live overrides (future).

## 2) Value Mapping and Validation

Legacy-to-Pico brightness mapping:

- Legacy range: `0..255`
- Pico runtime uses percent `0..100`
- Conversion:

$$
\text{percent} = \left\lfloor \frac{\text{legacy} \times 100 + 127}{255} \right\rfloor
$$

For `-b 8`, mapped percent is:

$$
\left\lfloor \frac{8 \times 100 + 127}{255} \right\rfloor = 3\%
$$

Note: existing board default level is 8%, which is brighter than strict legacy `-b 8` parity.

Validation rules to preserve from legacy behavior:

- `-W`: clamp to `10..1000` ms.
- `-B`: require `x,y,z` each in `0..9`; if `y > z`, swap.

## 3) Runtime Architecture Changes Needed

1. Add a playback state machine
- Modes: startup, base, blink burst, playlist animation.
- Implement random blink bursts using existing frame delay scheduler and RNG.

2. Add a frame post-process pipeline before `hw_led_present_rgb888`
- Stage A: optional monochrome recolor (`-c`).
- Stage B: optional rainbow white-pixel substitution (`-w`/`-W`).
- Stage C: optional gamma transform (`-g`).

3. Add optional realtime overlay channel (`-U`)
- UDP listener task receives nose-pixel payload.
- Compositor precedence policy:
  - Realtime nose pixels override animation nose region.
  - Remaining pixels come from animation pipeline.
- Timeout fallback to animation ownership after stale network interval.
- Current planning status: deferred until open protocol/design questions are resolved.

4. Add startup control
- Respect `skip_startup` constant instead of hardcoded playlist index jump.

5. Add deterministic diagnostics
- Boot log prints active constant values.
- Include explicit warnings for unsupported/disabled features (for staged rollout).

## 4) Suggested Rollout Order

1. Static constant plumbing + boot diagnostics + startup/brightness parity.
2. Blink scheduler parity (`-B`) with tests.
3. Rainbow mode + configurable fade step (`-w`, `-W`).
4. Gamma toggle (`-g`) with LUT and visual validation.
5. Monochrome random color (`-c`) once monochrome detection metadata strategy is chosen.
6. Realtime nose UDP path (`-U`) and compositor timeout behavior (deferred).

## Open Questions Before Implementation

1. Should parity target strict behavior compatibility, or improved behavior where legacy notes say broken (`-g`)?
2. For `-b`, do we want exact 0..255 input support (via generated config schema), or percent-only with mapped guidance?
3. For `-U`, should protocol match legacy WLED UDP mode exactly, or can we define a smaller Pico-specific packet contract?
4. For `-c`, should monochrome detection be runtime (scan frame) or generation-time metadata in asset headers?

## Quick Parity Constants for Requested Command

Requested legacy command:

```bash
-U -w -W 1000 -B 3-3-5 -c -u -b 8 -g
```

Equivalent Pico constant intent:

- `enable_realtime_nose = true`
- `rainbow_mode = true`
- `rainbow_step_ms = 1000`
- `blink_max_count = 3`
- `blink_min_delay_ms = 3000`
- `blink_max_delay_ms = 5000`
- `randomize_monochrome = true`
- `skip_startup = true`
- `brightness_percent = 3` (strict mapped parity from legacy `-b 8`)
- `gamma_enable = true`
