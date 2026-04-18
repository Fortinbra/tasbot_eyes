# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Dr. Light leads the Pico SDK migration architecture.

### Historical Summary (Sessions 2–6)
- Established the four-phase migration strategy around the three critical seams: LED driver, threading/timing, and assets/filesystem.
- Broke the work into executable features with checkpoints, then inserted **Phase 0** after Fortinbra clarified hard constraints: root sources stay read-only, Pico work lives in an isolated subdirectory, offline GIF preprocessing is mandatory, and `colorful.gif` is the first validation target.
- Validated Pimoroni Plasma 2350 W assumptions: RP2350A, ample flash/RAM headroom, standard WS2812B or APA102-style LED protocols, optional wireless out of scope for v1.
- Approved the documentation/submodule architecture: migration planning under `docs\migration\`, maintained animation sources in `external\TASBot-eye-animations\`, graceful fallback to local `gifs\`, and generated firmware assets under `pico_build\assets\generated\`.
- Locked the sequencing principle that Phase 0 precedes hardware work, hardware proof precedes timing refactors, and later asset/cleanup work can parallelize only after runtime stability is proven.

## Recent Updates

- 📌 Team roster finalized on 2026-04-15.
- 📌 **2026-04-15 (Session 2):** Four-phase migration strategy captured and merged to decisions.md. Team consensus on phased abstraction, conditional CMake, embedded ROM assets, and serial injection.
- 📌 **2026-04-18 (Session 19):** Independent crosscheck of full color pipeline after "still all blue." No software bug found. Converter fix was valid but not sufficient. Recommended decisive 4-color smoke test. Hardware proof gate remains OPEN.

## Learnings

- The Pimoroni Plasma 2350 board header (`pimoroni_plasma2350.h`) does NOT define `PICO_DEFAULT_WS2812_PIN`; board.h falls through to `PLASMA2350_DATA_PIN` (GPIO 15). This is correct but the fallback chain should be documented.
- No single WS2812B channel-order permutation (GRB↔RGB↔BRG etc.) can produce uniform "all blue" from diverse RGB input. "All blue" symptoms require either stale firmware, data corruption, or a non-color-channel root cause.
- The asset converter uint32 cast fix (Session 18) was a valid Layer 1 fix but was overclaimed as root-cause resolution. Layer 3 hardware proof was never performed. This violated the hardware-asset-validation-gate pattern.
- The decisive experiment for channel-order validation is a blocking 4-color smoke test (RED→GREEN→BLUE→WHITE, 3 seconds each) observed on physical hardware before entering the animation loop.
- Key paths for the full color pipeline: `generate-gif-asset.ps1` → `colorful_asset.generated.h` → `colorful_asset.c` → `embedded_animation.c` → `tasbot_layout.c` → `hw_led_pio.c` (pack function) → `ws2812.pio` (PIO state machine).
- Migration success depends more on seam control than on superficial build progress.
- The architecture should preserve the legacy host build, isolate Pico work, and make every phase exit criterion observable.
- `colorful.gif` on real hardware is the earliest meaningful proof point; smooth playback and fuller feature parity come later.


### 2026-04-18 (Session Current): Hardware Regression — Board Contract Audit Against 256-Pixel Panel

**Status:** HARDWARE SPECIFICATION MISMATCH; DOCS AUDIT REQUIRED

**Issue:** After matrix-order flash, hardware exhibits partial illumination (only ~two-thirds of panel lighting) with mangled image. User clarified actual panel is 8x32 = 256 pixels.

**Constraint Clarification:**
- Actual hardware: 8x32 serpentine panel
- Total pixels: 256
- Wiring: Physical LED chain snakes by logical column (down on even, up on odd)
- Mapper context: Top-left lit position is physical index 0

**Audit Scope:**
1. **Board contract:** Verify pico_build\src\firmware\board.h declares 256-pixel constraint explicitly
2. **Pipeline documentation:** Audit 	asbot_layout.c documentation for 256-pixel coverage claim
3. **Hardware manual:** Cross-check Plasma 2350 physical LED count against code assumptions
4. **Proof gates:** Update hardware-asset-validation-gate to include 256-pixel serpentine confirmation

**Critical Finding:**
The previous 	asbot_layout.c redesign assumed 154-pixel output, not 256-pixel panel input. This mismatch is the likely cause of partial illumination. The mapper must iterate all 256 physical pixels or explicitly justify why some are left dark.

**Next Action:**
Coordinate with Proto Man's pixel-mapping audit. Ensure board contract and documentation explicitly state:
- Panel: 8x32 (256 pixels total)
- Layout: Column-serpentine (even columns descend, odd columns ascend)
- Mapping validation: All 256 physical LEDs must be addressable from logical 28x8 frame

**Impact:** Cannot validate hardware until pixel count and serpentine order are fully reconciled across code, docs, and physical board.
