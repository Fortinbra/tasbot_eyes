# Squad Decisions

## Active Decisions

### 1. Four-Phase Pico SDK Migration Strategy
**Date:** 2026-04-15  
**Owner:** Dr. Light  
**Status:** PENDING REVIEW  
**Risk:** Medium

**Summary:** RPi → Pico SDK migration requires phased abstraction approach due to three critical seams: ws2811 LED driver, POSIX threading/signals, and filesystem/assets.

**Phases:**
1. **Phase 1 (CRITICAL):** Abstract hardware layer (hw_led interface)
2. **Phase 2 (HIGH):** Replace threading model (pthreads → event loop)
3. **Phase 3 (MEDIUM):** Handle filesystem/assets (embedded ROM + serial injection)
4. **Phase 4 (MEDIUM):** Remove/stub unsupported features

**Key Decisions:**
- Embed 3–5 key GIFs (startup, base, blinks) in Pico ROM
- Serial ASCII protocol replaces UDP animation injection
- Conditional CMake based on `PICO_SDK_PATH` detection
- Platform abstraction via hw_led interface

**Effort:** 3–4 sprints  
**Next Steps:** Phase 1 implementation + architecture review

---

### 2. Pico SDK Build System Rewrite
**Date:** 2026-04-15  
**Owner:** Auto  
**Status:** RECOMMENDED FOR IMPLEMENTATION

**Summary:** Current CMakeLists.txt is desktop/Linux-focused. Migration requires:
- Standard Pico SDK pattern: `pico_sdk_import.cmake`, `pico_sdk_init()`, firmware targets
- Source layout separation: `src/app/` (animation logic), `src/platform/pico/` (hardware glue)
- Asset handling abstraction (move from `./gifs/` FS discovery to generated headers or flash-resident data)
- Optional networking gates (Pico W + lwIP for UDP injection, not mandatory)

**Concrete Changes:**
- Add `pico_sdk_import.cmake` at repo root (or require `PICO_SDK_PATH`)
- Rewrite CMakeLists.txt around firmware target
- Reorganize sources without changing behavior yet
- Define asset pipeline (GIF → header or storage mechanism)

**Baseline:** Desktop host project; Pico SDK conversion pending  
**Blocker:** None; ready for implementation

---

### 3. Pico Runtime Seams: Portable Core vs. Platform Shell
**Date:** 2026-04-15  
**Owner:** Proto Man  
**Status:** BLOCKERS REQUIRE TEAM CLARIFICATION

**Summary:** Pico migration hinges on clear separation:
- **Portable Core:** `playAnimation*`, `showFrame`, TASBot pixel mapping, blink timing, nose LED mapping (minimal API reshaping needed)
- **Platform Shell:** `rpi_ws281x`, `pthread`, POSIX signals, UDP sockets, CLI argument parsing, filesystem asset discovery (wholesale replacement required)

**Architectural Blockers:**
- Exact target board: RP2040 Pico, Pico W, or other?
- LED output method: PIO-only, PIO+DMA, external driver, level shifting?
- UDP realtime control: retain, replace, or drop?
- Animation storage: firmware-baked, external flash, SD, or preconverted frames?

**Risk:** Asset handling is the most visible breaking change for end users  
**Recommendation:** Decide hardware/service seams before implementation

---

### 4. Migration Validation Gates & Regression Prevention
**Date:** 2026-04-15  
**Owner:** Mega Man  
**Status:** GATES APPROVED FOR IMPLEMENTATION

**Summary:** Prevent false build progress with six concrete regression checkpoints:

1. **Pico Build Identity:** CMake imports Pico SDK; firmware artifacts (`.elf`, `.uf2`) produced; desktop-only link targets optional
2. **Platform Seam Isolation:** Core animation logic compiles without POSIX/network/RPi driver headers; Pico-specific code owns hardware/timing
3. **Asset Path Replacement:** Startup, base, and blink assets available via new Pico mechanism; startup → base → blink loop works
4. **Display Correctness:** LED index mapping matches TASBot eye layout; nose region boundaries correct; RGB ordering preserved
5. **Timing & Loop Stability:** Render cadence/hue updates use Pico-safe primitives; no starvation or runaway busy-waits
6. **Control-Path Outcome:** If control features survive, prove end-to-end on hardware; if removed, clearly state new contract

**Riskiest Regressions:**
1. False build progress (CMake looks ready; source still has hidden coupling)
2. Broken asset assumptions (runtime FS → firmware can't adapt)
3. Timing/concurrency drift (threading → event loop; blink cadence changes)
4. LED protocol/mapping mismatch (index swap, channel scramble, nose region misalignment)
5. Networking scope confusion (UDP features fake parity instead of being explicit)

**Tester Mandate:** Do not call migration healthy until firmware builds + core behavior survives + hardware proof validates eye mapping

---

## Governance

- All meaningful changes require team consensus
- Document architectural decisions here
- Keep history focused on work, decisions focused on direction
