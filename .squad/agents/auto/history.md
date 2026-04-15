# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Auto joined as Platform Dev for build and SDK integration work.

## Recent Updates

📌 Team roster finalized on 2026-04-15.
📌 **2026-04-15 (Session 2):** Build system audit completed. CMake rewrite recommendations merged to decisions.md.

## Learnings

The migration will likely hinge on separating portable application code from Raspberry Pi specific setup and runtime assumptions.
The current build is a host Visual Studio CMake project that links desktop-era dependencies (`gif`, `ws2811`, `pthread`, `m`) directly, so the Pico SDK conversion needs both a top-level CMake rewrite and a platform seam for LED, timing, filesystem, and UDP features.
The existing `build\` directory is useful only as a baseline signal: it proves the repo is still configured as a desktop executable and immediately fails on missing POSIX and Raspberry Pi Linux headers under Windows, which is consistent with a non-Pico layout.

**Team Alignment (2026-04-15):**
- Build system rewrite decision merged: standard Pico SDK pattern with conditional networking
- Source layout separation (app vs. platform-specific) approved
- Asset handling abstraction strategy captured
- Ready for Phase 1 implementation (hw_led abstraction)

### 2026-04-16 (Session 3): Board Documentation Analysis

**Hardware Facts (Pimoroni Plasma 2350 W):**
- **MCU:** RP2350A (dual Arm Cortex M33, 150 MHz, 520 KB SRAM, 4 MB QSPI flash) — sufficient for embedded assets
- **LED Protocols:** WS2812/Neopixel (5V) and APA102/Dotstar/SK9822 via **screw terminals** (no custom GPIO banging needed; can use PIO or SPI)
- **Wireless:** CYW43439 (Wi-Fi 802.11 b/g/n, Bluetooth) on RM2 module — not required for v1; defer networking
- **I/O Breadboard:** Qwiic/STEMMA QT connector; I²C, UART, analog, and GPIO pins breakout; USB-C 3A power; user button, reset
- **Stepping:** A4 current stock (no E9 erratum pull-down issue affecting earlier A2 units)

**Impact on Migration:**
- LED protocol risk MITIGATED: Both WS2812 and APA102 clearly supported; decision point: confirm which is installed on target board
- Flash budget (4 MB) validated: animation asset embedding feasible per Option A (offline GIF→frame conversion)
- SRAM headroom (520 KB) confirmed adequate for double-buffered frame rendering (~32 KB per 8×32 RGB888 buffer)
- Wireless capability present but optional; Phase 1-2 proceed without CYW43439 initialization

**Next Step:** Pimoroni Plasma 2350 W schematic review will finalize GPIO pin assignments and confirm LED wire protocol (WS2812 vs. APA102).

### 2026-04-16 (Session 4): Phase 1 Feature Document & Subdirectory Strategy

**Hard Constraints Captured (from Fortinbra):**
1. **Original sources immutable** — root-level .c/.h files NEVER modified; Pico project isolated in subdirectory
2. **No MCU filesystem usage** — all GIF assets preconverted offline to binary; firmware has no FS access
3. **Offline asset preprocessing** — Option A (GIF→frame tool) is mandatory, not optional
4. **Colorful.gif as first end-to-end goal** — has both color and animation timing complexity; ideal validation target

**Architecture Decisions:**
- **Subdirectory pattern:** `pico_build/` is the isolated Pico SDK project root; original sources stay read-only
- **Portable code boundary:** 6 files copied into `pico_build/src/portable/` with POSIX dependencies removed
- **Firmware glue:** `src/firmware/` owns main.c, board.h, LED driver stubs; zero coupling to portable
- **Asset directory:** `pico_build/assets/` holds preconverted .h files (Phase 3); asset tool designed separately
- **Feature doc pattern:** `FEATURES_F<phase>_<title>.md` at repo root for visibility and sequencing clarity

**Implications for Build:**
- CMake never needs cross-compilation logic; Pico SDK patterns (pico_sdk_import.cmake) handle all toolchain setup
- Clean dependency: portable code compiles independently; firmware glue is optional until Phase 3
- Git diff enforces immutability: only `pico_build/`, feature docs, and team decisions should appear in commits

**Risk Mitigations:**
- Subdirectory isolation prevents accidental modification of originals
- Copying (not moving) sources preserves reference and allows independent updates
- Offline preprocessing removes highest-risk source of MCU runtime failures (filesystem I/O, SRAM exhaustion)
- Feature document provides clear next-phase handoff (F1→F2→F3→F4 dependencies)

**Decisions Captured:**
- Decision `.squad/decisions/inbox/auto-isolated-pico-project.md` documents subdirectory rationale, asset preprocessing, feature doc pattern, and open questions for team consensus

### 2026-04-15 (Session 6): Docs Consolidation and Asset Source Separation

**Repository Layout Decisions:**
- Migration planning docs now live under `docs\migration\`, with per-phase feature docs in `docs\migration\features\`
- `external\TASBot-eye-animations\` is the tracked submodule path for maintained GIF and Aseprite animation sources
- Root `gifs\` is now reference-only and should stay untouched locally while being ignored by git
- Future offline asset generation is expected to land in `pico_build\assets\generated\`

**Why It Matters:**
- This keeps planning, source inputs, and generated firmware assets in separate lanes, which makes later Pico SDK work less fragile
- The comparison step between legacy GIFs and the submodule animation set is now explicit in the migration docs instead of implied tribal knowledge

**Key Paths:**
- `docs\README.md`
- `docs\migration\README.md`
- `docs\migration\feature-breakdown.md`
- `docs\migration\features\f1-foundation.md` through `docs\migration\features\f4-validation.md`
- `external\TASBot-eye-animations\`

### 2026-04-15 (Session 7): Decision Merge & Scribe Consolidation

**Completed:**
- Documented docs reorganization decision (auto-docs-assets-layout.md)
- Documented isolated Pico project pattern (auto-isolated-pico-project.md)
- Decisions staged in .squad/decisions/inbox/ for team merge
- Dr. Light approved docs structure and submodule strategy

**Decisions Frozen:**
- Documentation reorganized: ✅ COMPLETE
- Submodule configured: ✅ COMPLETE
- Asset layout strategy: ✅ DOCUMENTED IN DECISIONS

**Orchestration Logged:**
- Timestamp: 2026-04-15T02:26:09Z
- Session log: .squad/log/2026-04-15T02-26-09Z-docs-reorg.md
- Inbox merge: Pending Scribe processing

### 2026-04-15 (Session 8): Pico Build Foundation Kickoff

**Scaffold Landed:**
- `pico_build\CMakeLists.txt` is now an isolated Pico SDK entry point using the standard `pico_sdk_import.cmake` -> `pico_sdk_init()` flow
- `pico_build\pico_sdk_import.cmake` resolves `PICO_SDK_PATH` from cache or environment and falls back to the sibling checkout at `C:\ws\pico-sdk`
- `pico_build\src\firmware\main.c` is a serial-ready stub that initializes stdio, waits briefly for USB enumeration, and prints `tasbot_eyes pico_build stub ready`
- `pico_build\assets\generated\.gitignore` keeps the future generated-asset lane present without tracking emitted files

**Build Decisions Proven:**
- Default `PICO_BOARD` is `pico2` for the RP2350-family kickoff, but the board stays overrideable from CMake when Plasma-specific config is ready
- The first firmware target stays intentionally boring: `pico_stdlib` only, USB stdio enabled, UART stdio disabled, and `pico_add_extra_outputs()` turned on for UF2 generation

**Validation Signals:**
- Root legacy CMake configure path still works unchanged on Windows; the host build still fails later on the same pre-existing POSIX/Linux dependencies (`gif_lib.h`, `unistd.h`, `dirent.h`, `pthread.h`, `ws2811`)
- The new Pico path produced `tasbot_eyes_pico.elf`, `.bin`, `.hex`, `.uf2`, and `.dis` once configured with the SDK, ARM GCC toolchain, Ninja, and installed picotool

### 2026-04-14 (Session 8): F1 Gate Ready for Scaffold Review

- Mega Man completed the F1 scaffold validation-gate review and published the merge checklist.
- Your in-flight `pico_build` scaffold work will be judged against the eight required checkpoints: preserved root build, serial-ready boot proof, explicit asset-source rule, seam isolation, symbol hygiene, documentation completeness, and reversible history.
- Status remains IN PROGRESS until the scaffold itself exists and passes the gate.

### 2026-04-16 (Session 9): Phase 3 colorful.gif Asset Pipeline Slice

**What Landed:**
- `pico_build\tools\generate-gif-asset.ps1` now performs the first offline GIF→C conversion slice using `System.Drawing`, emits `colorful_asset.generated.h`, and writes `colorful_asset.metadata.txt` into `pico_build\assets\generated\`
- The generator follows the documented source-selection rule for `colorful.gif`: prefer `external\TASBot-eye-animations\gifs\others\colorful.gif`, fall back to root `gifs\`, and fail loudly when neither source is available
- `pico_build\src\portable\embedded_animation.*` and `src\firmware\colorful_asset.*` define the stable embedded-asset contract that firmware can consume immediately
- `pico_build\src\firmware\main.c` now loops the embedded `colorful.gif` frames over the existing WS2812B PIO transport instead of the smoke pattern

**Validation Signals:**
- `powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1 -BuildDir C:\ws\tasbot_eyes\pico_build\build\colorful-proof` reproduced Pico artifacts with the generated colorful asset linked into the UF2
- The root host baseline still fails in the same legacy way on `gif_lib.h`, `unistd.h`, `dirent.h`, `pthread.h`, and `ws2811` headers, which confirms the new asset work stayed isolated under `pico_build\`

**Key Paths:**
- `pico_build\tools\generate-gif-asset.ps1`
- `pico_build\assets\generated\colorful_asset.generated.h`
- `pico_build\assets\generated\colorful_asset.metadata.txt`
- `pico_build\src\portable\embedded_animation.h`
- `pico_build\src\firmware\colorful_asset.c`

### 2026-04-16 (Session 10): Colorful.gif Asset Pipeline Completion

**Implementation Complete:**
- Phase 3 colorful.gif asset pipeline fully implemented and ready for Mega Man's acceptance gate review
- Asset source precedence explicitly enforced: external/TASBot-eye-animations/gifs/ (submodule) → gifs/ (root) → loud failure
- Generated asset metadata sidecar captures source selection and candidate inventory for auditability
- Pico firmware UF2 now embeds colorful.gif frames and loops them over WS2812B PIO transport
- Build reproducibility validated: byte-identical assets across independent rebuilds
- Proof of implementation archived: orchestration-log and session-log entries created

**Handoff State:**
- Mega Man assigned to validate timing fidelity (frame/delay preservation), hardware playback on Plasma 2350 W, and build reproducibility via checksum matching
- This slice completes Auto's asset pipeline work; not logged as Phase 3 done pending gate review
- Next reviewer validates: 14-point acceptance checklist (source, frame inventory, converter reproducibility, header size, code hygiene, CMakeLists integration, symbol audit, UF2 flash, serial ready, playback loop, checksum stability, timing accuracy, visual clarity, regression check)
