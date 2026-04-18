---
name: "bootsel-window-flash"
description: "Recover a missed RP2350 BOOTSEL window, flash the newest validated UF2, and record what re-enumerated."
domain: "testing"
confidence: "high"
source: "earned"
tools:
  - name: "powershell"
    description: "Checks Windows volumes, copies UF2 files, and samples serial ports."
    when: "Use for BOOTSEL drive discovery, UF2 copy, and quick post-flash observation."
  - name: "picotool"
    description: "Queries or reboots RP-series boards over USB."
    when: "Use when the board is reachable over USB serial but no BOOTSEL drive is mounted."
---

## Context
Use this when a Pico/RP2350 board is supposed to be in BOOTSEL mode, but the mass-storage drive has already vanished or the host only still sees the runtime USB connection. The goal is to use the available hardware window without rebuilding or touching source.

## Patterns
- Start by listing current Windows logical disks and serial ports so you know whether the board is already in BOOTSEL or has re-enumerated into runtime.
- If no BOOTSEL drive is mounted but `picotool info -a` sees an RP-series device on USB serial, recover the BOOTSEL window with `picotool reboot -f -u`.
- After BOOTSEL returns, copy the exact UF2 you chose and record its full path plus SHA-256 so the next hardware check is unambiguous.
- Treat the BOOTSEL drive disappearing after copy as expected reboot behavior; confirm the board is no longer in BOOTSEL and note any runtime USB serial device that remains.
- If you attach to the COM port after reboot and get silence, record that as a late-capture limitation rather than inventing missing boot output.

## Examples
- `picotool info -a` reports `RP2350 device ... appears to have a USB serial connection, not in BOOTSEL mode`; run `picotool reboot -f -u`, wait for `D:\` labeled `RP2350`, then copy `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2`.
- Post-flash verification on Windows: `D:\` disappears, `picotool info -a` no longer sees a BOOTSEL device, and the board is back on a USB serial port.

## Anti-Patterns
- Assuming the user-reported drive letter is still mounted without checking current volumes.
- Rebuilding firmware during an operational BOOTSEL task when a validated UF2 already exists.
- Reporting a serial-ready pass from a port opened only after the reboot window already elapsed.
