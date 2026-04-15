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
