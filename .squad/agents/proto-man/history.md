# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Proto Man: Backend Dev for Pico SDK firmware migration. Project architecture: portable animation core (frame composition, mapping, smoke patterns) separates cleanly from platform shell (ws2811/PIO, USB-CDC, timing cadence). Phase 2 seam established and validated: `logical 28×8 frame → TASBot layout mapper → RGB888 transport buffer → hw_led driver`. WS2812B PIO transport implemented. Hardware validated on Plasma 2350 (RP2350A, GPIO 15 data line).

**Critical Hardware Facts:**
- Physical panel: 8×32 serpentine (256 LEDs), not face-mask (154 pixels)
- Column-serpentine order: down even columns (0,2,4…), up odd columns (1,3,5…)
- Top-left LED maps to logical (0,0), physical index 0
- BOOTSEL recovery: `picotool reboot -f -u` reliably restores D:\ volume when stuck
- Windows USB-CDC: One-shot boot banner easily missed; treat as capture limitation

**Technical Stable Findings:**
- Asset generation: generate-gif-asset.ps1 must cast to `[uint32]` before shift ops to prevent integer overflow
- Source GIFs: 28×8 until wider assets generated; firmware pads into 32×8 frame
- Stale UF2 check: Scan ELF for RGB888 constants (e.g., `0x00FFAA00`, `0x0000FFAA`)
- Frame data integrity: colorful_asset.generated.h preserves colors through mapping; RGB→GRB packing happens at hw_led driver

**Current Status (2026-04-18):**
- ✅ Hardware validation loop CLOSED — user confirmed latest flash working on 256-pixel panel
- ✅ All 256 LEDs now driven (firmware constants: TASBOT_LOGICAL_WIDTH 28→32, TASBOT_PHYSICAL_LED_COUNT 154→256)
- ✅ Mapper rewritten for full-rectangle column-serpentine coverage (kTasbotIndex[8][32], no sparse entries)
- ✅ Multicolor output confirmed ("We have rainbows!")
- Current validated build: `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2` (SHA-256: `D55128FDE5396663238FB4978BC3AA6D82110D1DC5FB1FB768BD1D1C2827B108`)

## Recent Updates

📌 **2026-04-18 Session (Current):** Hardware validation loop closed. User confirmed latest flash working on 256-pixel 8×32 serpentine panel. All 256 LEDs now driven. Firmware constants updated. Mapper rewritten for full-rectangle coverage. Multicolor output confirmed.

📌 **2026-04-18T23:07:14Z (Scribe Update):** New display contract established — 224-LED panel-size directive (8×28). This is the canonical active display area for future GIF source generation. Proto Man updating contract/mapper; Dr. Light auditing interpretation and docs.


**Session History (Archived):**
- 2026-04-15: Team roster finalized. Proto Man assigned Backend Dev for Pico firmware work.
- 2026-04-16: Phase 2 runtime seam approved by Mega Man. WS2812B PIO transport implemented.
- 2026-04-16: WS2812B-over-PIO submission rejected for review. Mega Man identified three blockers: board.h contract implicit, proof reproducibility overclaimed, hardware gate open. Revision assigned to different agent.
- 2026-04-18: Blue-only asset generation root cause isolated (PowerShell uint32 cast missing). Fix applied. Hardware rainbow output confirmed.
- 2026-04-18: Critical regression discovered — physical panel is 256 pixels (8×32), firmware assumed 154 (face-mask). User reported ~2/3 panel lit with scrambled image. Contract audit and mapper rewrite completed.
- 2026-04-18: Hardware validation loop CLOSED — user confirmed latest flash working on all 256 pixels.

## Learnings

- 2026-04-18: The Pico runtime now treats `pico_build\src\portable\runtime_types.h` as an active-window contract (`28x8`, `224` LEDs) while `pico_build\src\firmware\board.h` keeps the physical WS2812 chain explicit (`8x32`, `256` LEDs).
- 2026-04-18: Preserve the existing column-serpentine wiring by blitting the active frame into the first 224 physical indices and clearing the remaining 32-LED tail dark each frame.
- 2026-04-18: `pico_build\tools\collect-proof.ps1` is the reliable rebuild path for Pico artifacts; it regenerates `colorful_asset.generated.h`, rebuilds `pico_build\build\ws2812-proof\`, and surfaces the UF2 SHA-256 needed for flash handoff.
- 2026-04-19: Fresh build with 224-active / 256-physical LED constants confirmed. UF2 SHA-256: `73EC6FE6685241ED9775C56E7336CAF3E41F426936351908910D346A1AD8B9EE`. Board flashed via D:\ BOOTSEL — drive ejected cleanly on reboot. Python3 for CMake must be resolved from `C:\Users\thegu\.pico-sdk\python\3.12.6\python.exe` (add to PATH before running collect-proof.ps1).
- 2026-04-19: Animation registry pattern established — `generate-animation-registry.ps1` calls `generate-gif-asset.ps1` for each of the 21 animations and writes `animation_registry.generated.h` containing all struct instances and the `kTasbotAnimationPlaylist[]` array. `animation_registry.c` exposes the playlist via `g_tasbot_animation_playlist` / `g_tasbot_animation_playlist_count`. `main.c` cycles animations on frame-count exhaustion using modular arithmetic. Legacy `gifs/` pool ambiguity (root `blink.gif` vs `blinks/blink.gif`) is handled by try/catch in `generate-gif-asset.ps1` — external submodule selection takes precedence; legacy ambiguity only fails hard when external is also absent. New UF2 SHA-256: `164CB03E00AC3E7F9451445B316E29627431FA0CDD3C1CCAEF068DF0CBECEBC9`.
