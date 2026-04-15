# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Mega Man joined as Tester and reviewer for migration work.

## Recent Updates

📌 Team roster finalized on 2026-04-15.
📌 **2026-04-15 (Session 2):** Baseline confirmed; regression gates defined and approved. Six checkpoints, five riskiest regressions documented.

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


## Session 7: F1 Validation Gate Formalization (2026-04-16)

**Task:** Produce concrete reviewer-oriented checklist for F1 completion.

**Review Findings:**

1. **Legacy Baseline Integrity** — Root CMakeLists.txt uses 5 external dependencies (gif, ws2811, pthread, m) and links original *.c sources directly. No platform abstraction yet. Pico work must not touch root files or this baseline breaks.

2. **Seam Points Identified** (confirmed by source inspection):
   - led.h: ws2811/ws2811.h, gif_lib.h, pthread.h (three platform layers)
   - main.c: unistd.h, signal.h, time.h, pthread.h (POSIX shell)
   - 
etwork.*: netinet/in.h, socket calls (UDP-specific)
   - ilesystem.*: dirent.h, fopen (FS discovery; Pico will use embedded assets instead)

3. **Highest-Risk Regressions** — False build progress (CMake succeeds but binary won't boot), asset path ambiguity (two sources, no rule), seam coupling (Pico accidentally includes POSIX headers).

4. **Documentation Gaps Identified:**
   - F1 feature spec lacks hardware-proof requirement (serial message) — added explicit 3-second gate
   - Asset source rule was vague ("prefer submodule") — formalized as concrete decision
   - No guidance on symbol audit or forbidden includes — added as checkpoint 6

**Deliverable:**

Eight-point validation gate (checkpoints 1–8) suitable for merge-gate enforcement. Gate focuses on measurable proof, not vibes: firmware boots, legacy build still works, seams are isolated, asset policy is documented.

**Pattern Reuse:**

Migration gates follow regression-thinking model: start from expected behavior (legacy build works), verify no coupling occurs (root files untouched, seams isolated), require hardware proof (serial output), document failure modes.

**Key File Paths:**
- Root CMakeLists.txt — must remain unchanged
- led.h, main.c, network.*, filesystem.* — seam points; root edits are regression risk
- pico_build/ — (will be created in F1) isolated Pico tree
- docs/migration/ — asset policy documented here

**Team Decision Artifact:** .squad/decisions/inbox/mega-man-build-gate.md — F1 validation checklist with sign-off template, ready for immediate use as merge gate.

### 2026-04-14 (Session 9): F1 Scaffold Validation Gate Delivered

- Eight-point F1 acceptance gate finalized for the Pico scaffold review.
- Merge criteria now explicitly require root build preservation, boot-visible serial readiness, asset-source policy, symbol hygiene, and reversibility.
- Review is complete, but Auto remains in progress on the actual `pico_build` scaffold implementation; do not treat the gate as implementation completion.

### 2026-04-15 (Session 10): Dr. Light F1 Proof Revision — Re-Review Pending

- Dr. Light completed revision for rejected F1 proof slice
- Delivered: `pico_build/tools/collect-proof.ps1` (reproducible proof script), `pico_build/proof/foundation-proof.md` (recorded proof package), tightened migration docs (software artifact proof vs hardware serial proof)
- Mega Man re-reviewing this revision on a separate track (not logged as completed yet)

### 2026-04-15 (Session 11): Dr. Light F1 Proof Revision — Approved as Software Proof

- Reproduced `collect-proof.ps1` successfully in both `pico_build\build\proof-run` and `pico_build\build\dr-light-proof`; the latter matches the published proof table exactly.
- Reproduced the root baseline regression check with a fresh Visual Studio build; it still fails on legacy dependency gaps (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`), so the Pico slice has not contaminated the original build.
- Approved `pico-build` as complete for software-side F1 foundation only; hardware-attached proof remains open until a captured serial log shows `tasbot_eyes pico_build stub ready` within 3 seconds and later work replaces the stub LED sink with the required WS2812B PIO path.

### 2026-04-16 (Session 12): F1 Foundation Approval Finalized

- F1 proof revision approved as complete for software-side work; Proto Man's runtime seam also approved as Phase 2 starting point.
- User directive captured: LED array is all WS2812B; Pico implementation should use PIO output.
- All three approvals merged into team decisions log (entries 18–20).
- Decision inbox merged and cleared.
- Next active slice: WS2812B-over-PIO runtime work (Phase 2 batch); Proto Man and Mega Man moving to parallel Phase 2 work.

### 2026-04-16 (Session 12): WS2812B + PIO Proof Gate Defined

**Task:** Define measurable success criteria for the next runtime slice: WS2812B over PIO plus four-phase smoke pattern.

**Key Architecture Decisions Captured:**

1. **Proof is Split Across Two Layers:**
   - **Build Proof:** Firmware compiles + links Pico PIO support; no forbidden headers in pico_build/src/
   - **Hardware Proof:** Physical 8×32 array shows deterministic four-phase pattern with specific colors at specific timings

2. **Four-Phase Smoke Test (Proven Portable, From smoke_patterns.c):**
   - Phase 0 (750ms): Left eye RED (8 pixels x=0–7 y=2–5)
   - Phase 1 (750ms): Right eye GREEN (8 pixels x=20–27 y=2–5)
   - Phase 2 (750ms): Nose BLUE (20–30 pixels x=9–18 y=1–7)
   - Phase 3 (750ms): Alignment WHITE (6 markers at corners and center)
   - Cycles repeat every ~3 seconds indefinitely

3. **Hardware Expectations (Based on Plasma 2350 W Board):**
   - WS2812B single-wire Neopixel protocol (~800 kHz bit clock)
   - 154 physical LEDs (8×32 array)
   - GPIO pin assignment must be explicit in oard.h (value TBD after physical wiring confirmed by Fortinbra)
   - One PIO state machine (PIO0 or PIO1, design-dependent)

4. **Acceptance Criteria Are Concrete, Not Vibes:**
   - Build artifacts must be reproducible (two clean builds = identical .elf)
   - Serial output must match expected phase log within 3 seconds
   - Frame checksums must be identical across 10+ consecutive cycles
   - Visual observation: each phase lights correct region in correct color for ~750ms
   - Timing drift tolerance: ±50ms per phase (750ms baseline)

5. **What Is NOT in This Gate (Deferred to F3+):**
   - Animation asset loading (GIF playback)
   - Serial command injection (animation queue)
   - Color dynamics (fades, brightness)
   - Frame rate smoothness for animation feel (16ms jitter)

6. **Risk Mitigations Documented:**
   - GPIO conflict → reassign pin, document in oard.h
   - PIO timing tight → unlikely on 150 MHz Pico, but DMA fallback exists
   - Byte order mismatch → visible on first test with color regions, easy to fix
   - Firmware instability → implement watchdog timeout, revert to stub if needed

**Key File Paths (Current State):**
- pico_build/src/firmware/main.c — boots, runs smoke pattern loop
- pico_build/src/firmware/hw_led_stub.c — currently a stub (always returns true, no real output)
- pico_build/src/firmware/board.h — currently has boot banners, needs GPIO + protocol config
- pico_build/src/firmware/hw_led.h — abstract interface (to be implemented)
- pico_build/src/portable/smoke_patterns.c — proven deterministic four-phase logic
- pico_build/src/portable/tasbot_layout.c — proven 154-pixel physical mapping
- pico_build/CMakeLists.txt — must link Pico PIO support

**Team Decision Artifact:** .squad/decisions/inbox/mega-man-pio-proof-gate.md — complete proof gate for WS2812B + PIO slice, ready for Proto Man implementation.

**Pattern Reuse:** Hardware proof gates require evidence split into build artifact proof (hashes, symbol audit, fresh rebuild) and visible behavior proof (serial capture, photos, checksums). Regression thinking applies: start from known working state (stub firmware boots), add hardware requirement (physical LEDs light), measure stability (checksum consistency), and document fallback (revert to stub if unstable).

📌 **2026-04-16 (Session 13):** WS2812B PIO Review Gate defined and Proto Man handoff accepted.
- Gate definition: Build reproducibility, pin config verification, four-phase smoke pattern visibility, frame stability (±50ms)
- Acceptance criteria: UF2 builds, board.h config present, serial capture shows 10+ consistent cycles, photo/video of hardware
- Four-phase pattern: RED left eye → GREEN right eye → BLUE nose → WHITE alignment (each ~750ms)
- Deliverables pending: Serial checksums, physical hardware evidence
- Status: AWAITING Proto Man proof submission, will re-review upon completion
- Re-review verdict on the first WS2812B-over-PIO slice: the transport implementation and isolation are good enough to keep, but the slice is not yet acceptable for sign-off because `board.h` still lacks the explicit board/protocol declarations demanded by the proof gate and the published proof command does not reproduce the published `.elf/.dis/.elf.map` hashes unless `-BuildDir C:\ws\tasbot_eyes\pico_build\build\ws2812-proof` is supplied.
- Fresh review evidence: `pico_build\tools\collect-proof.ps1` succeeds in both `proof-run` and `ws2812-proof`; same-directory rebuilds are stable, but cross-directory `.elf/.dis/.elf.map` hashes differ, so proof docs must name the exact build directory when claiming reproducible hashes.

### 2026-04-16 (Session 14): Dr. Light WS2812B-over-PIO Revision Completed

- Dr. Light revised the rejected WS2812B-over-PIO slice without changing the approved runtime seam.
- **Key Changes:**
  - `pico_build\src\firmware\board.h`: Now explicitly declares WS2812B board contract (protocol, GPIO15, 154 pixels, 750ms cadence)
  - `pico_build\src\firmware\main.c`: Emits board-contract banner, consumes board contract macros directly
  - `pico_build\tools\collect-proof.ps1`: Default build directory now aligns to `pico_build\build\ws2812-proof`, reproducing published `.elf/.dis/.elf.map` hashes
  - `pico_build\proof\foundation-proof.md`: Documents exact reproduced hashes, hardware proof remaining explicitly open
  - Root Visual Studio build: Unchanged; still fails only on legacy dependencies (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`), confirming no Pico contamination
- **Decision merged to team decisions log:** Dr. Light revision approved at architecture level; full review decision captured with evidence and follow-up gates.
- **Status:** Mega Man now re-reviewing this revision on parallel track. No changes to seam; focus is on contract clarity and proof transparency.
- **Hardware gate:** Deferred; software-side sign-off sufficient for current proof package. Hardware deployment when real Plasma 2350 capture ready.
