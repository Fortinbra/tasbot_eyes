---
name: "bootsel-flash-verification"
description: "Flash a Pico/RP2350 in BOOTSEL mode on Windows and prove the right UF2 actually booted"
domain: "firmware-operations"
confidence: "high"
source: "earned"
---

## Context

Use this when a Pico-family board is already in BOOTSEL mode as a removable drive and the task is to flash a known UF2 without guessing whether the copy really completed.

## Patterns

- Verify the exact UF2 path exists before touching the board.
- Hash the UF2 with SHA-256 and compare it to the expected value so stale build directories do not get flashed by accident.
- Record the BOOTSEL drive label if available, then copy the UF2 directly to the mounted drive root.
- Treat BOOTSEL exit as an observable state change: after a successful copy, the mass-storage drive should disappear.
- Confirm the runtime side came back by watching Windows serial devices for a USB Serial Device (COMx) enumeration.
- Attempt a short serial read after enumeration, but report missing banner bytes as a host-capture limitation if the firmware only prints boot text once before the host opens the port.

## Examples

- C:\ws\tasbot_eyes\pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2 hashed to D55128FDE5396663238FB4978BC3AA6D82110D1DC5FB1FB768BD1D1C2827B108, copied to BOOTSEL D:\ (RP2350), then the drive disappeared and the board came back as USB Serial Device (COM10).

## Anti-Patterns

- Flashing a UF2 because its directory name looks current without hashing the file.
- Assuming the copy succeeded just because Copy-Item returned without verifying the BOOTSEL drive disappeared.
- Claiming a fresh boot banner was seen when the COM port was opened only after re-enumeration and no bytes were actually captured.
