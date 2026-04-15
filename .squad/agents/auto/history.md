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
