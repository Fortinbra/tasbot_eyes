# tasbot_eyes_pico

Pico firmware build root for TASBot eyes on Pimoroni Plasma 2350.

## Dependencies

Install these before building:

1. Raspberry Pi Pico SDK (2.2.0 recommended)
  - https://github.com/raspberrypi/pico-sdk
2. ARM GNU Toolchain (arm-none-eabi, version 14.2.Rel1 used in this project)
  - https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
3. CMake (3.16+)
  - https://cmake.org/download/
4. Ninja
  - https://ninja-build.org/
5. Python (required by Pico SDK helper scripts)
  - https://www.python.org/downloads/

Optional for flashing/debug flows:

1. picotool
  - https://github.com/raspberrypi/picotool
2. OpenOCD (if using SWD/debug probe workflows)
  - https://openocd.org/

## Quick Start (Windows PowerShell)

From the project root:

```powershell
$env:PICO_SDK_PATH = "C:\Users\<you>\.pico-sdk\sdk\2.2.0"
cmake -S . -B build -G Ninja -DPICO_BOARD=pimoroni_plasma2350
cmake --build build
```

Expected artifacts in `build/`:

- `tasbot_eyes_pico.elf`
- `tasbot_eyes_pico.bin`
- `tasbot_eyes_pico.hex`
- `tasbot_eyes_pico.uf2`
- `tasbot_eyes_pico.dis`

## VS Code Tasks

This workspace includes tasks for common operations:

1. `Compile Project`
2. `Run Project` (picotool load)
3. `Flash` (OpenOCD)

## Notes

- Asset generation is integrated into the build via `tools/generate-animation-registry.ps1`.
- Generated animation headers under `assets/generated/` are source artifacts used by firmware.
- Build directories and binary outputs are intentionally ignored by git.

## Proof Workflow

Use `tools/collect-proof.ps1` to generate repeatable build evidence:

```powershell
powershell -ExecutionPolicy Bypass -File tools/collect-proof.ps1
powershell -ExecutionPolicy Bypass -File tools/collect-proof.ps1 -BuildDir .\build\ws2812-proof
```
