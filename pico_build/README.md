# pico_build

This directory is the isolated Pico SDK foundation slice for the migration. It intentionally keeps firmware glue, proof tooling, and future generated assets out of the legacy root build.

## Build

```text
cmake -S pico_build -B pico_build\build\proof -G Ninja -DPICO_SDK_PATH=C:\ws\pico-sdk -DPICO_BOARD=pimoroni_plasma2350
cmake --build pico_build\build\proof
```

Expected first-pass artifacts after a successful build:

- `tasbot_eyes_pico.elf`
- `tasbot_eyes_pico.bin`
- `tasbot_eyes_pico.hex`
- `tasbot_eyes_pico.uf2`
- `tasbot_eyes_pico.dis`

The build now also regenerates `pico_build\assets\generated\colorful_asset.generated.h` and
`pico_build\assets\generated\colorful_asset.metadata.txt` from `colorful.gif` before compiling
the firmware. Source selection follows the documented rule: prefer
`external\TASBot-eye-animations\gifs\`, fall back to root `gifs\`, and fail loudly if neither
contains the requested asset.

## Proof workflow

Use `tools\collect-proof.ps1` to reproduce the scaffold evidence package:

```text
powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1
powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1 -BuildDir C:\ws\tasbot_eyes\pico_build\build\ws2812-proof
```

The default script path is now `pico_build\build\ws2812-proof`, so the published proof hashes and the documented command path stay aligned for the path-sensitive `.elf`, `.dis`, and `.elf.map` artifacts.

The script:

1. configures and builds a fresh Pico proof directory
2. prints artifact sizes and SHA-256 hashes
3. confirms the boot/ready banners plus the board contract are embedded in the ELF
4. inventories serial ports and, if `-SerialPort COMx` is provided, captures boot output for the requested window

## Boot-ready contract

The current firmware shell is intentionally small but explicit:

- emits `tasbot_eyes pico_build booting` immediately after stdio bring-up
- emits `board contract: WS2812B on GPIO15, active 8x28 (224 pixels) within physical 8x32 (256 pixels)`
- waits 2000 ms
- emits `tasbot_eyes pico_build ready` four times at 250 ms intervals
- enters the embedded `colorful.gif` loop after the ready window and presents it through a WS2812B PIO transport on the Plasma data pin (`GPIO15` / `PLASMA2350_DATA_PIN`)
- drives the physical 8x32 panel in top-left-first, column-serpentine order (down column 0, up column 1, and so on)
- treats the last four physical columns as intentionally unused, so only the active 8x28 / 224-LED window shows image content

That gives a first visible ready signal at ~2.0 s after boot and keeps the ready banner visible through ~2.75 s, which is the reproducible software contract for the Phase 1 gate. Hardware capture is still required to close the full serial-ready gate on a real board.

## Embedded colorful.gif slice

- `tools\generate-gif-asset.ps1` is the offline converter for the first Phase 3 demo target.
- The generated metadata file records the canonical source, both candidate pools, the selection rule,
  and the frame timing summary for `colorful.gif`.
- Firmware playback now consumes the generated asset contract directly, so the Pico image loops
  `colorful.gif` from UF2-resident data instead of relying on runtime filesystem access.
- The current `colorful.gif` source is 28x8, which now matches the active display contract exactly.
- Firmware still clears and transmits the full 8x32 physical chain, but leaves the final four columns dark
  so the inactive tail of the panel stays intentionally unused.
