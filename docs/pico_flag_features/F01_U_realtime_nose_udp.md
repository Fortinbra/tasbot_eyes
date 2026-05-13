# F01: -U Realtime Nose UDP Control

## Status

- Deferred until open design/protocol questions are resolved.
- Do not schedule implementation in the current feature wave.
- Revisit after completing the currently active non-network features.

## Legacy Intent

`-U` enables UDP realtime control for centered nose LEDs. Normal animation rendering should not overwrite the nose field while realtime data is active.

## Firmware Scope

Add optional realtime overlay channel for nose LEDs:

- receive UDP packets for nose pixel data
- map payload into nose logical positions
- composite realtime nose pixels over animation frame
- return control to animation after timeout

## Static Firmware Constants (Current)

When this feature is resumed, define these as constants (for example in `board.h` or a dedicated firmware feature header):

- `enable_realtime_nose` (bool)
- `realtime_udp_port` (uint16, default legacy-compatible)
- `realtime_stale_timeout_ms` (uint16)

## Future Config Surface

- compile-time override macros
- optional runtime config channel

New runtime pieces:

- UDP receiver task or polling loop
- nose overlay buffer with timestamp
- compositor stage before `hw_led_present_rgb888`

## Implementation Checklist

1. Add profile fields and boot-time log of active realtime settings.
2. Add nose overlay buffer sized to nose pixel count.
3. Implement UDP decode for expected packet mode.
4. Apply overlay only to nose region when data is fresh.
5. Expire overlay after timeout and fall back to animation ownership.
6. Add counters: packets received, decode errors, stale expirations.

## Independent Test Plan

### Unit Tests

1. Packet decode accepts valid payload and rejects malformed length.
2. Nose index mapping writes expected logical coordinates.
3. Stale timeout disables overlay after configured window.

### On-Board Validation (Firmware)

1. Boot with `enable_realtime_nose=false`:
- send packets
- verify no visible effect and packet path is inactive

2. Boot with `enable_realtime_nose=true`:
- send static nose color pattern
- verify nose region changes while non-nose pixels keep animation output

3. Stop packet stream:
- verify nose region returns to animation after timeout

### Pass Criteria

- Realtime data never corrupts non-nose pixels.
- Overlay behavior is deterministic across timeout boundary.
- No watchdog resets under sustained packet input.

## Out of Scope

- full legacy networking feature parity beyond nose overlay
- authentication/encryption on control stream
