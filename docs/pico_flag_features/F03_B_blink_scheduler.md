# F03: -B Blink Scheduler Pattern

## Status

- Done.
- Implemented with animation/base/wait/blink scheduler states, PRNG burst selection, and delay-range normalization.

## Legacy Intent

`-B x-y-z` means:

- `x`: max blink count between animations
- `y`: min seconds between blinks
- `z`: max seconds between blinks

Legacy behavior chooses random blink amount in `[1..x]` (or zero if `x=0`) and random delay in `[y..z]` seconds, converted to milliseconds.

## Firmware Scope

Introduce blink scheduling state machine independent from linear playlist loop.

## Static Firmware Constants (Current)

Define these as constants (for example in `board.h` or a dedicated firmware feature header):

- `blink_max_count` (uint8, 0..9)
- `blink_min_delay_ms` (uint16)
- `blink_max_delay_ms` (uint16)

## Future Config Surface

- compile-time override macros
- optional runtime config channel

Optional legacy parser helper:

- parse string pattern `x-y-z` and apply validation/swap rules.

## Implementation Checklist

1. Add static constants and validation (`0..9`, swap min/max when inverted).
2. Add PRNG-backed helper for blink count and delay selection.
3. Add scheduler states: animation, base, wait, blink burst.
4. Ensure `blink_max_count=0` disables blink burst cleanly.
5. Emit debug counters for selected blink counts and delays.

## Independent Test Plan

### Unit Tests

1. Pattern parser accepts `3-3-5` and yields 3,3000,5000 ms.
2. Inverted range (for example `3-5-3`) is corrected to 3000..5000 ms.
3. Blink count generator returns only expected bounds.

### On-Board Validation (Firmware)

1. Config `3-3000-5000 ms`:
- observe multiple cycles
- verify each burst has 1..3 blinks
- verify delay between blinks is within 3000..5000 ms

2. Config `0-3000-5000 ms`:
- verify no blink burst plays

3. Long-run test:
- verify no scheduler lockups or watchdog resets

### Pass Criteria

- Burst count and delay always stay in configured ranges.
- Scheduler transitions remain deterministic and recoverable.
- Blink feature can be enabled/disabled without side effects on main playback.

## Out of Scope

- multi-blink animation library selection policy changes
- dynamic reconfiguration while a burst is running
