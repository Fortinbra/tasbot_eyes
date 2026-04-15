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

### 5. Pico SDK Migration Feature Breakdown
**Date:** 2026-04-15  
**Owner:** Dr. Light  
**Status:** APPROVED (for team review and implementation)

**Summary:** Operationalized the Four-Phase Pico SDK Migration Strategy into 14 concrete executable features (F1-1 through F4-4) with explicit acceptance criteria, sequential/parallel dependencies, testable exit criteria, and risk assessment.

**Key Components:**
- **Phase 1 (Hardware Abstraction):** 3 features; high-risk LED driver implementation
- **Phase 2 (Threading Model):** 4 features; event loop refactor; high-risk timing changes
- **Phase 3 (Assets):** 4 features; asset embedding pipeline; medium-risk ROM budget
- **Phase 4 (Cleanup):** 3 features; low-risk optional features; final validation

**Critical Path:** F1-1 → F1-2 → F1-3 → F2-2 → F2-3 → F2-4. Phases 3 & 4 can parallelize after F2-3.

**Checkpoint Gates:** 6 mandatory validation gates ensure build progress is real and prevent integration disasters.

**Artifact:** `FEATURE_BREAKDOWN.md` (repository root) with full specifications, acceptance criteria, dependencies, gate descriptions, risk mitigation, and timeline.

**Effort:** 3–4 weeks at typical team velocity.

**Next Steps:** 
1. Fortinbra: Review FEATURE_BREAKDOWN.md; sign-off on scope and timeline
2. Coder: Implement F1-1 (Define Hardware LED Interface)
3. Dr. Light: Architecture review of F1-1 output; gate F1-2 start

---

### 6. Asset Pipeline for Pico Migration
**Date:** 2026-04-15  
**Owner:** Roll  
**Status:** Pending Team Consensus

**Summary:** Analyzed three asset delivery options for Phase 3 (Assets) within Pico's ~256 KB usable flash budget:

**Options Considered:**
- **Option A: Offline GIF→Frame Conversion (Recommended)** — Convert GIFs to raw frame buffers at build time. Fast startup, no runtime decoding, deterministic memory. Flash cost: 10–50 KB per animation. Effort: Medium.
- **Option B: Embed Small GIFs in Flash** — Ship GIFs as raw bytes; decode on first boot. Requires GIFLIB porting (complex). Flash cost: 30 KB per GIF + 10 KB GIFLIB. Effort: High.
- **Option C: SD Card with On-Demand Decoding** — Load from SD; decode frame-by-frame. Unlimited assets but requires SD driver, timing overhead. Effort: High.

**Recommendation:** Start with Option A (offline conversion) in Phase 3; plan Option C fallback if flash budget becomes tight.

**Rationale:** Aligns with Pico SDK best practices, avoids porting large libraries, deterministic performance, asset format can evolve independently.

**Blocking Issue:** Plasma 2350 W LED protocol specification (pins, protocol, timing) needed before Phase 3 begins.

**Action Items:**
- [ ] Team consensus on Option A vs. alternatives
- [ ] If Option A: design frame buffer binary format
- [ ] Create GIF→buffer conversion tool
- [ ] Verify Plasma 2350 W I/O contract

---

### 7. Plasma 2350 W Hardware Specification — Risk Mitigation & Board Selection Validation
**Date:** 2026-04-15  
**Owner:** Auto, Dr. Light  
**Status:** RISK MITIGATED; ARCHITECTURE VALIDATED

**Summary:** Research into Pimoroni's official Plasma 2350 W documentation confirms board specifications align with migration strategy and closes two medium-impact risks.

**Hardware Facts:**
- **MCU:** RP2350A (dual Cortex M33 @ 150 MHz, 520 KB SRAM, 4 MB QSPI flash)
- **LED Protocols:** WS2812B/Neopixel (single-wire, ~800 kHz) and APA102/Dotstar (dual-wire, SPI-like) via labeled screw terminals
- **Wireless (Optional):** CYW43439 module (802.11 b/g/n + Bluetooth); out-of-scope for v1
- **Power:** USB-C 3A; adequate for 256 LEDs @ 60 mA typical draw
- **Stepping:** A4 current stock (no errata); A2 variants have pull-down sensitivity issue (not relevant to LED output)

**Risk Mitigations:**

| Risk | Before | After | Reason |
|------|--------|-------|--------|
| Board electrical spec unclear | Medium | Low | Pimoroni documentation authoritative; WS2812B/APA102 are standard, not proprietary |
| Flash space insufficient for assets | Medium | Low | 4 MB QSPI flash supports 10–20 embedded animations @ 50–100 KB each; Option A (offline GIF→frame) now high-confidence |
| SRAM for animation buffers | Low | Very Low | 520 KB vs. RP2040's 264 KB; double-buffered 8×32 RGB rendering needs ~32 KB |
| LED timing too strict for Pico | Low | Low | RP2350A @ 150 MHz and PIO make WS2812B/APA102 bitbanging straightforward |

**Architecture Impact:** NONE. Four-Phase migration strategy (Decisions #1, #5) remains unchanged. Phases 1–4 dependencies, critical path, acceptance criteria all unchanged.

**Sequencing Impact:** NONE. Board selection risk was a dependency blocker for Phase 1 kickoff; now cleared without reordering.

**Outstanding Action Item:**
- Confirm LED protocol wiring (WS2812B vs. APA102) by physical unit inspection or schematic review (Fortinbra)
- Phase 1 deliverable: `src/firmware/board.h` with GPIO pin assignments and protocol selection

**Reference Materials:** Pimoroni Plasma 2350 W shop page; Pimoroni plasma MicroPython repo (github.com/pimoroni/plasma); target board schematic (PDF from shop).

---

### 9. Documentation Reorganization & Asset Source Layout
**Date:** 2026-04-15 (Session 6)  
**Owner:** Auto  
**Status:** APPROVED  
**Risk:** Low

**Summary:** Migration planning docs consolidated under `docs/migration/`, maintained animation sources tracked via Git submodule in `external/TASBot-eye-animations/`, and legacy `gifs/` folder marked as ignored reference material.

**Architecture:**
```
docs/
├── README.md
└── migration/
    ├── README.md
    ├── feature-breakdown.md
    └── features/
        ├── f0-structure.md         (Phase 0: Pico SDK setup)
        ├── f1-foundation.md        (Phase 1: Hardware LED interface)
        ├── f2-core-runtime.md      (Phase 2: Threading/event loop)
        ├── f3-assets-playback.md   (Phase 3: Asset embedding)
        └── f4-validation.md        (Phase 4: Cleanup & validation)
```

**Key Decisions:**
- All phases (0–4) documented with acceptance criteria and validation gates
- Submodule path: `external/TASBot-eye-animations/` (read-only source)
- Legacy path: `gifs/` (untracked, local reference only)
- Generated assets: `pico_build/assets/generated/` (git-ignored)

**Sequencing Impact:** None. Documentation structure supports existing Phase 0–4 architecture without changes.

**Next Steps:** Fortinbra approval on submodule + naming; Phase 3 implementer to document asset selection strategy.

---

### 10. Submodule Architecture & Asset Pipeline Strategy
**Date:** 2026-04-15 (Session 6)  
**Owner:** Dr. Light (reviewer), Auto (implementer)  
**Status:** APPROVED WITH CONDITIONS  
**Risk:** Low

**Summary:** Dr. Light reviewed and approved submodule strategy for `external/TASBot-eye-animations/` as read-only external source. Asset pipeline will support graceful degradation if submodule unavailable.

**Approved Conditions (Phase 3 responsibility):**
1. Asset pipeline must continue building if `external/TASBot-eye-animations/` not initialized
2. Asset selection algorithm must be documented (which source wins for duplicate animations?)
3. `.gitignore` rules must clearly separate legacy (gifs/), external (submodule), and generated (pico_build/assets/)

**Architecture Fit:** 
- Submodule is optional, not required for local builds
- Supports eventual migration to external repo as canonical source
- Phase 3 asset preprocessor will compare both sources and select intelligently

**Risk Mitigations:**
- No hard build-time dependency on submodule
- Graceful fallback to local `gifs/` if submodule unavailable
- Pattern is standard Git practice (read-only external)

**References:** `external/TASBot-eye-animations` → github.com/Inverted/TASBot-eye-animations

---

## Governance

- All meaningful changes require team consensus
- Document architectural decisions here
- Keep history focused on work, decisions focused on direction

---

## 8. Pico SDK Project Structure & Constraints (Phase 0 Requirement)

**Date:** 2026-04-15 (Session 5)  
**Owner:** Dr. Light  
**Status:** APPROVED  
**Risk:** Low (structure only, no code changes)

**Summary:** Fortinbra has clarified hard constraints that require a new Phase 0 before existing Phase 1.

**Constraints:**
1. **Original sources are read-only** — No in-place refactoring
2. **Pico SDK lives in subdirectory** — `pico_sdk/` as independent Pico root
3. **GIF preprocessing is offline** — Asset pipeline is foundational (Phase 0), not Phase 3
4. **colorful.gif is first validation target** — MVP acceptance requires colorful.gif playback

**Architecture Decision:**

New **Phase 0: Project Structure & SDK Setup** becomes mandatory first phase:
```
F0-1: Create pico_sdk/ subdirectory + CMake structure
F0-2: Preprocess colorful.gif to binary asset header
F0-3: Pico CMake build linking portable code from parent directory
 └─→ F1-1: Define Hardware LED Interface (existing phases 1–4 follow unchanged)
```

**Directory Layout:**
```
tasbot_eyes/
├── CMakeLists.txt, *.c, *.h, gifs/  (original, read-only)
└── pico_sdk/                         (NEW: Pico firmware root)
    ├── CMakeLists.txt
    ├── src/ (platform-specific)
    └── assets/ (preprocessed headers)
```

**Build Isolation:**
- Pico: `cd pico_sdk/build && cmake .. && make`
- RPi: `cmake . && make` (unchanged)
- Zero interference; independent parallel work

**Risk Mitigation:**
- Reversibility: Pico build can be discarded without touching original code
- Clarity: Explicit boundary between original and adaptation
- Early validation: colorful.gif success gate detects issues before later phases

**Impact on Sequencing:**
- Original critical path (F1-1 → F1-2 → F1-3 → F2-x → F3-x → F4-x) remains unchanged
- Phase 0 (F0-1, F0-2, F0-3) is now blocking predecessor to Phase 1
- Estimated effort: +2 hours (mainly CMake setup + asset preprocessing tool)

**Next Steps:**
1. Fortinbra: Confirm colorful.gif location in gifs/ directory
2. Auto/Coder: Implement Phase 0 features before Phase 1
3. Dr. Light: Update FEATURE_BREAKDOWN.md with Phase 0 prepended
4. Team: Architecture review of Phase 0 deliverables before Phase 1 kickoff

**Reference:** `.squad/decisions/inbox/dr-light-project-structure-constraints.md`
