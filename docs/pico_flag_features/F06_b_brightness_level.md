# F06: -b Brightness Level Mapping

## Status

- Done.
- Implemented with legacy 0..255 mapping (default 8), percent conversion, and button cycling retained.

## Legacy Intent

`-b <0..255>` sets maximum brightness for LED output. Legacy path clamps the value to 0..255.

## Firmware Scope

Support explicit configured brightness target for parity, while preserving existing hardware button level cycling as optional behavior.

## Static Firmware Constants (Current)

Define these as constants (for example in `board.h` or a dedicated firmware feature header):

- `brightness_legacy_0_255` (uint16, clamped to 0..255) or direct `brightness_percent`
- `brightness_button_enable` (bool)

## Future Config Surface

- compile-time override macros
- optional runtime config channel

Mapping:

$$
percent = \left\lfloor\frac{legacy \times 100 + 127}{255}\right\rfloor
$$

For legacy `-b 8`, parity target is `3%`.

## Implementation Checklist

1. Add static constant input and clamp logic.
2. Convert legacy scale to percent used by `hw_led_set_brightness_percent`.
3. Apply configured brightness at boot before first frame present.
4. Gate button cycling via config flag.
5. Log effective brightness percent after mapping.

## Independent Test Plan

### Unit Tests

1. Clamp tests: input `-10`, `0`, `8`, `255`, `300` map to expected effective value.
2. Mapping test confirms `8 -> 3%`.
3. Button-disabled config prevents runtime brightness changes.

### On-Board Validation (Firmware)

1. Set brightness profile to low (legacy 8):
- verify visibly dim output and logged 3%

2. Set brightness profile to max (legacy 255):
- verify logged 100%

3. With button enabled:
- verify button still cycles levels from configured starting point

### Pass Criteria

- Effective brightness matches mapping/clamp rules.
- Startup brightness is stable before first visible animation frame.
- Optional button behavior does not overwrite disabled mode.

## Out of Scope

- gamma-perceived brightness correction coupling
