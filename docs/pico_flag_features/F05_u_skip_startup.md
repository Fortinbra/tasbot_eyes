# F05: -u Skip Startup Animation

## Status

- Not needed in the current firmware state.
- Startup animation playback is currently disabled, so there is no startup phase to skip.
- Keep this note as deferred parity work only if startup playback is reintroduced later.

## Legacy Intent

`-u` skips startup animation on the first loop iteration.

## Firmware Scope

Make startup behavior driven by a static firmware constant instead of always hardcoded to skip.

## Static Firmware Constants (Current)

Define this as a constant (for example in `board.h` or a dedicated firmware feature header):

- `skip_startup` (bool)

## Future Config Surface

- compile-time override macros
- optional runtime config channel

## Implementation Checklist

1. Replace hardcoded startup skip in firmware main loop with constant-driven branch.
2. Keep existing behavior as default if desired (`skip_startup=true`).
3. Log active startup mode at boot.

## Independent Test Plan

### Unit Tests

1. Startup-mode helper resolves `skip_startup=true/false` correctly.

### On-Board Validation (Firmware)

1. `skip_startup=true`:
- verify startup asset is not displayed
- verify runtime begins directly in cycle playlist

2. `skip_startup=false`:
- verify startup asset plays once before cycling playlist

3. Reboot consistency:
- verify behavior is consistent across power cycles

### Pass Criteria

- Startup mode exactly follows profile on every boot.
- No off-by-one playlist index errors.

## Out of Scope

- startup preemption by external realtime control
