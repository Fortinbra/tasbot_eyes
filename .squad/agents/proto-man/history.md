# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Proto Man joined as Backend Dev for low-level firmware work.

## Recent Updates

📌 Team roster finalized on 2026-04-15.
📌 **2026-04-15 (Session 2):** Runtime seams analysis completed. Portable core and platform shell separation scoped. Decision merged with blockers for team clarification.

## Learnings

This project's core risk is translating Raspberry Pi oriented runtime assumptions into microcontroller-friendly firmware behavior.
- 2026-04-15: The current runtime is split between portable animation logic (frame composition, TASBot index mapping, blink scheduling) and Linux/Raspberry-Pi-specific services (rpi_ws281x, pthread threads, POSIX signals, UDP sockets, filesystem-driven GIF loading).
- 2026-04-15: Biggest Pico blockers are unresolved hardware/service seams: which Pico variant is targeted, how animation assets will be stored and decoded without host filesystem/GIFLIB assumptions, and whether realtime control survives via Wi-Fi, USB, UART, or gets dropped.
- 2026-04-16: `pico_build\src\portable\` can safely start with TASBot geometry plus deterministic smoke-pattern code; `color.c/.h` is the next low-risk legacy copy when HSV fade or gamma correction is needed.
- 2026-04-16: The clean Phase 2 firmware seam is `logical 28x8 frame -> TASBot layout mapper -> 154-pixel RGB888 transport buffer -> hw_led driver`, with the firmware loop owning all cadence.
- 2026-04-16: `led.h`, `gif.h`, `filesystem.h`, and `network.h` are the hard compatibility blockers because they leak `ws2811`, GIFLIB, POSIX directory APIs, sockets, and pthreads across otherwise portable code.
- 2026-04-18: The blue-only `colorful.gif` symptom traced to `pico_build\tools\generate-gif-asset.ps1`, not the Pico PIO transport; PowerShell channel bytes must be widened to `[uint32]` before `-shl` or red/green bits collapse and the generated header under `pico_build\assets\generated\` turns multicolor frames into mostly blue output.
- 2026-04-18: The known-good multicolor firmware image is `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2`; the older `pico_build\build\review-colorful-proof\tasbot_eyes_pico.elf` still lacks orange/green/cyan frame constants, so flashing that stale artifact can preserve the blue-only symptom.
- 2026-04-18: First real Plasma 2350 hardware pass succeeded on the BOOTSEL `D:\` volume and the board re-enumerated as `USB Serial Device` on `COM10`; stable loop proof was captured in `pico_build\proof\hardware-serial-capture.log`, `hardware-validation-run.txt`, `hardware-boot-capture.log`, and `hardware-boot-summary.txt`.
- 2026-04-18: On this Windows USB-CDC path, opening the COM port after enumeration did not replay the one-shot boot/ready banners even though the runtime loop was clearly alive, so missing ready lines must be recorded as a host-capture limitation rather than guessed away.
- 2026-04-18: When the RP2350 is reachable over USB but the BOOTSEL mass-storage window has closed, `picotool reboot -f -u` reliably brings it back as `D:\` (`RP2350`) so the current validated UF2 can be copied; after copy, the drive disappears again and `picotool info -a` reports the board back on USB serial rather than BOOTSEL.
- 2026-04-18: The portable path is now proven multicolor end-to-end up to the transport buffer: `colorful_asset.generated.h` contains rainbow RGB888 values, `tasbot_embedded_animation_load_frame()` preserves them, `tasbot_layout_blit_frame()` maps them into diverse 154-pixel transport colors, and stable varying frame checksums in `pico_build\proof\hardware-serial-capture.log` mean any remaining blue-only failure is downstream of mapping.
- 2026-04-18: A fast stale-image check for this project is scanning the built ELF for little-endian RGB888 constants; the current `ws2812-proof` ELF contains `0x00FFAA00`, `0x00AAFF00`, `0x0000FFAA`, and `0x0000AAFF`, while the stale `review-colorful-proof` ELF does not.
- 2026-04-18: **HARDWARE MILESTONE: Rainbow success confirmed.** Plasma 2350 now displaying multicolor output. User visual confirmation: "We have rainbows!" Colorful animation loop actively playing on embedded ROM. Previous blue-only output resolved.

**Team Alignment (2026-04-15):**
- Portable core identified: minimal API reshaping required
- Platform shell wholesale replacement confirmed
- Architectural blockers escalated for team/hardware clarification
- Asset handling confirmed as most visible breaking change for users

### 2026-04-16 (Session 12): Runtime Seam Slice Approved for Phase 2

- Mega Man approved Proto Man's runtime seam design as Phase 2 starting point.
- Isolation rule held: `pico_build\` stays clean of POSIX/RPi headers; seam boundary concrete and measurable.
- **Approved Seam:** `logical 28x8 frame → TASBot layout mapper → 154 RGB888 transport buffer → firmware hw_led sink`
- **Portable layer:** `pico_build\src\portable\tasbot_layout.c`, `smoke_patterns.c` — deterministic frame generation and geometry mapping
- **Firmware layer:** `pico_build\src\firmware\main.c`, `hw_led_stub.c`, `board.h` — startup, timing cadence, transport boundary
- Next step: Fix Pico host-tool build path to emit `.elf`/`.uf2`; capture USB-serial ready banner plus full smoke-pattern cycle from hardware.
- WS2812B-over-PIO implementation approved for Phase 2 work once hardware board wiring is confirmed.

### 2026-04-16 (Session 13): WS2812B PIO Transport Landed

- Replaced the Pico-side stub sink with `pico_build\src\firmware\hw_led_pio.c`, using a standard WS2812 PIO program and RGB888→GRB packing at the hardware seam.
- `pico_build\src\firmware\board.h` now resolves the LED data pin from `PICO_DEFAULT_WS2812_PIN`, then `PLASMA2350_DATA_PIN`, then a hard fallback to GPIO 15 so the Plasma board path stays explicit.
- `pico_build\CMakeLists.txt` now prefers `pimoroni_plasma2350` when the SDK has that board header, otherwise falls back to `pico2`.
- Clean Pico proof build succeeded at `pico_build\build\ws2812-proof`, producing `.elf`, `.bin`, `.hex`, `.uf2`, and embedding the updated boot/ready/runtime seam banners.
- Root baseline still fails in the expected legacy ways (`ws2811/ws2811.h`, `netinet/in.h`, and `dirent.d_type`/`DT_REG`), which confirms the new work stayed isolated under `pico_build\`.
- Checked in `pico_build\src\firmware\ws2812.pio.h` beside the source `.pio` file because building SDK `pioasm` from source in this Windows environment was less reliable than using the prebuilt tool once and committing the generated header.


📌 **2026-04-16 (Session 13 Complete):** WS2812B PIO firmware implementation delivered.
- Produced: ws2812.pio + generated header, hw_led_pio.c driver, reproducible UF2 proof
- Hardware path: GPIO from board.h with fallback to GPIO 15 (Plasma alignment)
- Seam validated: 154-entry RGB888 transport → hw_led → GRB repack → GPIO 800kHz
- Decision: Committed generated .pio.h to avoid pioasm rebuild friction on Windows
- Status: COMPLETE, handed to Mega Man for review gate (build proof, serial capture, physical hardware validation)

### 2026-04-16 (Session 14): WS2812B-over-PIO Review Rejection

**Status:** REJECTED FOR SIGN-OFF; Awaiting revision by different agent  
**Reviewer:** Mega Man  
**Reason:** Three sign-off blockers: board.h contract too implicit, proof reproducibility overclaimed, hardware gate open

**What Passed:**
- Phase 2 seam held; legacy isolation confirmed
- Pico/PIO wiring structurally correct
- Local build proof exists (`.elf`, `.bin`, `.hex`, `.uf2`, `.dis`)

**Blocking Issues (Require Revision):**
1. **board.h contract:** Protocol, pin, pixel count, cadence/frame-rate must be explicit declarations, not implicit fallbacks
2. **Proof reproducibility:** Published hashes path-sensitive; command must name exact build directory or republish from default
3. **Hardware proof:** No serial transcript (10+ checksum cycles), no smoke-phase video/photo; software proof only

**Required Revision Scope (Different Agent):**
1. Tighten `board.h` — explicit board-level declarations
2. Fix `foundation-proof.md` — reproducibility claim or republish hashes
3. Flash to Plasma 2350; capture serial log + smoke-phase video

**Decision:** Proto Man locked out of revision. Next step: Different agent revises and resubmits for gate review.

### 2026-04-18 (Session Current): Blue-Only Output Root Cause Isolated & Asset Fix Applied

**Status:** RESOLVED (asset fix applied; flash validation pending)

**Problem:** First real-board flash showed all LEDs blue instead of multicolor colorful.gif animation.

**Root Cause:** `pico_build\tools\generate-gif-asset.ps1` was shifting color channel bytes without `[uint32]` cast, causing integer overflow that zeroed red and green lanes.

**Fix Applied:**
1. Cast each color channel to `[uint32]` before left-shift operators in generate-gif-asset.ps1
2. Regenerated `pico_build\assets\generated\colorful_asset.generated.h` with correct multicolor frame data
3. Rebuilt `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2`

**Evidence:**
- Generated header before fix: blue-only values (0x0000FF, 0x0000AA) for frames that should have white/red/orange/green/cyan
- Source GIF verified multicolor via external image viewer
- Failure mode matches observed symptom perfectly: WS2812 transport alive, but payload corrupted at generation

**Expected Result on Next Flash:**
- LED array displays recognizable multicolor sweep (white → red → orange → green → cyan)
- Frame checksums vary (confirms color data variation)
- Animation pacing reasonable and recognizable as source colorful.gif

**Next Action:** Fortinbra to flash regenerated UF2 and validate colorful.gif playback with serial capture + visual proof.

- 2026-04-18: The live 154-pixel mapper in `pico_build\src\portable\tasbot_layout.c` now follows the real harness order: first physical LED is the top-left lit pixel, then the chain snakes by logical column (down on even columns, up on odd columns) while preserving the existing 28x8 TASBot occupancy mask.
- 2026-04-18: For this workspace, the reliable rebuild path is the existing `pico_build\build\ws2812-proof` directory with `C:\Users\thegu\.pico-sdk\cmake\v3.31.5\bin\cmake.exe --build ...`; a fresh configure without the cached Pico host tools can fail while trying to build `picotool`.
- 2026-04-18T22:32:45Z: **Hardware flash attempt — matrix-order mapper validation.** Board re-entered BOOTSEL after mapper fix. Flashing `ws2812-proof` UF2 to D:\ (BOOTSEL volume) to validate board exit and re-enumeration on serial COM port. Mapper update ensures top-left LED now maps correctly to physical index 0 with column-serpentine transport order.
