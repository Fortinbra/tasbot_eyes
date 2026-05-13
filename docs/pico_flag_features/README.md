# Pico Flag Feature Pack

This folder breaks legacy CLI parity into independently shippable firmware features.

Source research: `docs/PICO_FLAG_PARITY_RESEARCH.md`.

## Feature Docs

1. `F01_U_realtime_nose_udp.md` for `-U`
2. `F02_w_W_rainbow_white_remap.md` for `-w` + `-W`
3. `F03_B_blink_scheduler.md` for `-B`
4. `F04_c_random_monochrome_recolor.md` for `-c`
5. `F05_u_skip_startup.md` for `-u`
6. `F06_b_brightness_level.md` for `-b`
7. `F07_g_gamma_correction.md` for `-g`

## Current Focus

- `F01_U_realtime_nose_udp.md` is intentionally deferred for now.
- `F05_u_skip_startup.md` is currently not needed because startup playback is disabled.
- Active implementation/testing should proceed with the remaining features.

## Progress Status

- Done: F02 `-w/-W` rainbow remap
- Done: F03 `-B` blink scheduler
- Done: F04 `-c` random monochrome recolor
- Done: F06 `-b` brightness mapping/config
- Done: F07 `-g` gamma correction
- Not needed currently: F05 `-u` skip startup (startup playback disabled)
- Deferred: F01 `-U` realtime nose UDP

All currently in-scope non-deferred features are now implemented.

## Remaining Delivery Order

1. F05 `-u` skip startup (only if startup playback is re-enabled)
2. F01 `-U` realtime nose UDP (deferred)

## Test Strategy

Current assumption for this feature wave:

- feature values are static firmware constants
- runtime behavior is validated on physical hardware
- compile-time/runtime configuration expansion is a future phase

Each feature doc includes:

- firmware implementation scope
- explicit static constants for the current implementation
- future compile-time/runtime configuration notes
- unit/on-board/manual test cases
- pass/fail criteria
- out-of-scope notes

Enable only one feature at a time during board validation to keep behavior attribution clean.
