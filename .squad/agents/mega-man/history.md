# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Mega Man joined as Tester and reviewer for migration work.

### Summarized History (Sessions 2–16, Archived)

**Regression Gates & Phase Validation (2026-04-15, Sessions 2–6)**
- Established six regression checkpoints: Pico build identity, platform seam isolation, asset path replacement, display correctness, timing stability, control-path outcome
- Identified five riskiest failure modes: false build progress, broken assets, timing drift, LED mapping corruption, networking scope confusion
- Finalized F1 scaffold validation: eight-point gate requiring root baseline preservation, boot serial visibility (3s), asset policy, symbol hygiene, reversibility
- Approved "Demo Gate" after F1-3: early hardware proof before Phase 2 threading refactor

**F1–F3 Review & Hardware Symptom Triage (2026-04-16, Sessions 9–16)**
- Reviewed Auto's F1 scaffold: design/structure sound, but reproof required (picotool native compiler issue, serial readiness unproven)
- Accepted Dr. Light's revised F1 proof: software artifact reproducibility passed, hardware serial gate deferred
- Defined 14-point F3 colorful asset acceptance gate: source selection, offline reproducibility, code hygiene, build integration, 10+ cycle checksum stability, visual proof
- Approved Auto's colorful asset pipeline: offline GIF→C converter reproducible, byte-identical across rebuilds, but hardware playback gates (7–14) remain open
- Diagnosed blue-only LED symptom: ranked failure candidates (color channel inversion highest), created 16-point validation checklist, proved multi-layer approach

**Recent Discovery (2026-04-18)**
- Confirmed root cause of "all LEDs blue" symptom: asset converter missing `[uint32]` cast before bit-shift, zeroing red/green lanes
- Proto Man applied fix; UF2 rebuilt; hardware validation pending (checksum variance + visual color fidelity required)

## Learnings

Migration success needs clear behavior checkpoints because the runtime platform is changing drastically.
- Current baseline is a host-side Visual Studio CMake build with no CTest coverage; in this environment the build stops immediately on missing POSIX/RPi headers (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`, `netinet/in.h`).
- The highest-risk migration seams are not cosmetic build edits: they are the LED driver contract, runtime asset loading from `./gifs`, POSIX threading/timing, and UDP control paths that may not exist on the Pico target.
- Migration gates need proof at each seam: firmware target generation, preserved animation asset access, verified LED index mapping, timing stability, and an explicit decision on whether network-driven features survive on Pico or are replaced/stubbed.
- F1 scaffold review pattern: accept layout/doc wins only after reproducing both stories — root baseline still fails in the same pre-existing way, and Pico build evidence must be rerun to real artifacts instead of inherited from prior notes.
- Current scaffold paths to watch during review: `pico_build\CMakeLists.txt`, `pico_build\pico_sdk_import.cmake`, `pico_build\src\firmware\main.c`, `pico_build\src\firmware\board.h`, `pico_build\assets\generated\.gitignore`, and `.gitignore`.
- In this workspace, `pico_build\` stays clean of forbidden POSIX/RPi headers, but the attempted Pico build stopped before artifact generation because the SDK fell back to building `picotool` from source without a resolved native host compiler in that sub-build.
- Runtime seam slice approved as a Phase 2 starting point when it keeps the boundary at `logical 28x8 frame -> TASBot layout mapper -> 154 RGB888 transport buffer -> firmware hw_led sink`; that seam now exists in `pico_build\src\portable\` and `pico_build\src\firmware\`.
- Fresh review configure for `pico_build\` proved the firmware target now compiles the intended seam files (`main.c`, `hw_led_stub.c`, `tasbot_layout.c`, `smoke_patterns.c`) without pulling in forbidden host headers, even though the build still stops later in the `picotool` host sub-build.
- Root legacy validation remains a regression checkpoint, not a portability target: a fresh Windows build still fails on the same legacy host gaps (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`), which is useful evidence that `pico_build\` has not contaminated the original sources.
- Key review artifacts for this seam: `pico_build\build\review-4\build.ninja` (fresh source list proof), `docs\migration\features\f2-core-runtime.md` (approved seam contract), and `pico_build\src\portable\tasbot_layout.c` plus `smoke_patterns.c` (geometry and deterministic smoke phases).
- Revised F1 proof is acceptable as software-side foundation when `pico_build\tools\collect-proof.ps1` reproduces fresh Pico artifacts, `pico_build\proof\foundation-proof.md` honestly leaves the serial-ready gate open, and a fresh root build still fails only on legacy host dependencies.
- The proof hashes for `tasbot_eyes_pico.elf`, `.dis`, and `.elf.map` are build-path-sensitive; the recorded values in `pico_build\proof\foundation-proof.md` match `-BuildDir C:\ws\tasbot_eyes\pico_build\build\dr-light-proof`, not the script's default `proof-run` directory.
- Current F1 firmware remains a stub transport (`pico_build\src\firmware\hw_led_stub.c`); the confirmed WS2812B hardware path still needs explicit board/protocol configuration plus PIO-backed output proof in later validation.
- Revised Phase 2 WS2812B-over-PIO software proof is acceptable when `pico_build\src\firmware\board.h` makes the WS2812B/GPIO15/154-pixel/750 ms contract explicit and `pico_build\tools\collect-proof.ps1` defaults to the same `pico_build\build\ws2812-proof` directory named in `pico_build\proof\foundation-proof.md`.
- Review line stays split: local sign-off can cover source contract, seam isolation, and reproducible Pico artifacts, but hardware-attached acceptance still needs a real Plasma 2350 run with serial capture, 10+ checksum cycles, and visual four-phase smoke confirmation.
- Key Phase 2 proof files to re-check on future revisions: `pico_build\src\firmware\board.h`, `pico_build\src\firmware\hw_led_pio.c`, `pico_build\src\firmware\main.c`, `pico_build\tools\collect-proof.ps1`, and `pico_build\proof\foundation-proof.md`.
- Phase 3 colorful asset review passes the software-side bar when `external\TASBot-eye-animations\gifs\others\colorful.gif` is the selected canonical source, the legacy fallback `gifs\others\colorful.gif` hashes identically, and `pico_build\tools\generate-gif-asset.ps1` regenerates byte-identical `colorful_asset.generated.h` plus metadata across fresh runs.
- Useful Phase 3 review files: `pico_build\tools\generate-gif-asset.ps1`, `pico_build\assets\generated\colorful_asset.generated.h`, `pico_build\assets\generated\colorful_asset.metadata.txt`, `pico_build\src\firmware\colorful_asset.c`, `pico_build\src\firmware\main.c`, `pico_build\src\firmware\hw_led_pio.c`, and `docs\migration\features\f3-assets-playback.md`.
- The current colorful slice is acceptable only as software-side proof: fresh Pico build integration works, the ELF exports `g_tasbot_colorful_*` symbols, but the full colorful gate stays open until on-device serial/video evidence proves 10+ checksum-stable cycles with timing measured on hardware.

**Team Alignment (2026-04-15):**
- Six regression checkpoints approved as merge gates (not aspirations)
- Five riskiest regressions identified for monitoring
- Tester mandate clear: firmware artifacts + core behavior + hardware proof required
- Feature-scope and asset-source decisions escalated to team

**Phase Validation Review (2026-04-16):**
- **Phase 0** foundational but incomplete without serial output test. Added: "Firmware prints 'Pico Firmware Ready' within 3 seconds" + symbol audit gate.
- **Phase 1** F1-2 had contradictory acceptance. Clarified: led.c is *refactored* (not preserved); must call hw_led abstraction instead of ws2811 directly. Critical hardware gate: LED test pattern renders on physical board.
- **Phase 2** timing validation was unmeasurable ("video comparison if available"). Replaced with concrete instrumentation: 30-second duration test (±50 ms tolerance), 1-hour stability run, frame counter logging. Phase 2 gates Phase 3; timing bugs block demo.
- **Phase 3** is FIRST TRUE HARDWARE DEMO. Added explicit MVP gate: "Colorful.gif plays on array; base animation queues via serial." Earliest credible demo: ~7–10 days after Phase 0 kickoff.
- **Phase 4** validation sign-off was vague. Added explicit checklist: firmware flash, visual playback quality (no glitches), serial responsiveness, 30-minute stability run, symbol audit clean, code review sign-off.
- **Critical path:** Phase 0 (2–4 hrs) → Phase 1 (3–5 days) → Phase 2 minimal (1–2 days) → Phase 3 (2–3 days) = **~7–10 days to colorful.gif demo.**
- **Non-negotiable gates:** Phase 0 serial output; Phase 1 hardware LED test; Phase 3 MVP playback; Phase 4 30-minute stability run.
- **Regression risk profile:** Phases 1 & 2 highest risk (hardware driver + timing). Phase 3 medium (asset format can evolve). Phase 4 low (cleanup).

**Hardware Milestone (2026-04-18):**
- **RAINBOW SUCCESS CONFIRMED:** Plasma 2350 now displaying multicolor output. User visual confirmation: "We have rainbows!" Colorful animation loop actively playing on embedded ROM. Previous blue-only output resolved. Phase 3 MVP gate closure confirmed by real hardware observation.

