# Squad Decisions

## Active Decisions

### 1. Dr. Light: 154→256 Pixel Contract Audit — Full 8×32 Panel
**Date:** 2026-04-19  
**Owner:** Dr. Light  
**Status:** ACTIVE — blocks hardware validation

The user reports "about 2/3 are lighting up, and the image is still mangled." This is correct: 154 / 256 = 60.2% ≈ "about 2/3." The entire firmware pipeline — constants, layout mapper, generated assets, converter, smoke patterns, and documentation — is built around the legacy TASBot face-mask geometry (28 logical columns, 154 occupied pixels out of 224 grid positions). The physical hardware is a full 8×32 = 256 rectangular serpentine panel with no holes.

**Root Cause:** Every constant, table, and validation check in the Pico firmware reflects the old TASBot face shape, not the physical panel. See full breakdown:
- `runtime_types.h`: `TASBOT_LOGICAL_WIDTH 28u` should be `32u`, `TASBOT_PHYSICAL_LED_COUNT 154u` should be `256u`
- `board.h`: `TASBOT_EYES_LED_PIXEL_COUNT 154` should be `256`
- `tasbot_layout.c`: `kTasbotIndex[8][28]` (sparse face mask, 154 entries) must become `kTasbotIndex[8][32]` (full rectangle, 256 entries)
- `generate-gif-asset.ps1`: Hard rejects anything ≠ 28×8, must accept 32×8
- Generated assets, smoke patterns, nose mapping: all hardcoded for 28-wide face

**Why 2/3 Light Up and Image Is Mangled:**
1. Firmware pushes exactly 154 GRB packets to WS2812B chain, then asserts reset. Pixels 154–255 receive no data and stay dark. 154/256 ≈ 60%.
2. `kTasbotIndex` maps a non-rectangular face shape to physical indices 0–153. Even though remapped for column-serpentine, the face-shaped occupancy with gaps produces a garbled image on full-panel.

**GIF Source Problem:** Upstream animations are 28×8 because they were designed for face-mask shape. Full 8×32 panel needs 32×8 content.

**Two Paths:**
- **A. Quick fix (padding):** Keep 28×8 source GIFs, center in 32-wide panel (2 blank columns each side), rewrite mapper for 256 full-rectangle. Gets all 256 LEDs driven and content centered, but 4 edge columns dark.
- **B. Correct fix (new content):** Create 32×8 source art, update converter to accept 32×8, update all constants. Long-term right answer.

**Recommendation:** Implement option A first as unblocking step (all 256 LEDs driven, recognizable), then pursue option B when content available.

**Proto Man Tier 1 (Required for any 256 LEDs):**
1. `runtime_types.h`: `TASBOT_LOGICAL_WIDTH` → 32, `TASBOT_PHYSICAL_LED_COUNT` → 256
2. `board.h`: `TASBOT_EYES_LED_PIXEL_COUNT` → 256
3. `tasbot_layout.c`: Rewrite `kTasbotIndex[8][32]` as full 256-entry column-serpentine. No -1 entries.

**Tier 2 (Required for image display):**
4. `embedded_animation.c`: Width check now expects 32. Update converter to emit 32-wide frames (padded from 28) or add centering path in loader.
5. `generate-gif-asset.ps1`: Remove/relax 28×8 hard check.
6. Regenerate `colorful_asset.generated.h` with updated converter.

**Tier 3 (Validation tooling):**
7. `smoke_patterns.c`: Shift right-eye patterns from column 20–27 to 24–31 for centering.
8. `tasbot_nose_field_to_logical()`: Adjust nose constants for 32-wide grid.
9. `collect-proof.ps1`: Update string patterns to "256 pixels" and "32x8 frame".

**Gate Status:** Hardware validation gate remains **OPEN** until all 256 LEDs driven and displayed image visually recognizable on physical panel.

---

### 2. Proto Man: Pico Panel Contract Must Target Full 8×32 / 256 LEDs
**Date:** 2026-04-18  
**Owner:** Proto Man  
**Status:** PROPOSED

Fortinbra confirmed the real Pico-driven matrix is a full 8×32 panel with 256 LEDs, wired top-left first and snaked by column.

**Decision:**
- Change Pico runtime contract to 32 columns, 8 rows, 256 transport pixels
- Map logical (x,y) to physical LED index with direct column-serpentine math
- Keep current 28×8 generated assets by clearing 32×8 frame and copying smaller assets top-left until asset pipeline widened

**Why:**
- Old 154-pixel contract truncated loops and left ~2/3 of panel reachable
- Legacy sparse occupancy table wrong for full rectangular matrix
- Padding in firmware unblocks hardware validation without forcing asset-pipeline redesign

**Evidence:**
- Updated runtime now targets `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2`
- Rebuilt UF2 SHA-256: `D55128FDE5396663238FB4978BC3AA6D82110D1DC5FB1FB768BD1D1C2827B108`
- Mapper formula sanity-check: covers each physical index 0..255 exactly once

**Impact:**
- After reflash, panel stops truncating at 154-pixel boundary
- Image order follows real harness: down column 0, up column 1, alternating across 32 columns
- 28×8 art still renders, extra panel area black until wider assets generated

---

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

### 11. Implementation Directive: Demo-First Phase Validation
**Date:** 2026-04-15T02:27:30Z  
**Owner:** Fortinbra (via Copilot)  
**Status:** ACTIVE DIRECTION

**Summary:** Review each migration phase for a validatable end state, optimize the sequence to demo `colorful.gif` on hardware as quickly as possible, and defer fine-tuning plus the rest of the animation set until after that milestone.

---

### 12. Fastest Credible `colorful.gif` Demo Gate
**Date:** 2026-04-15 (Session 8)  
**Owner:** Dr. Light  
**Status:** DECISION APPROVED

**Summary:** The earliest credible hardware demo is an explicit Demo Gate after F1-3: colorful.gif frames render on the Plasma 2350 W before Phase 2 timing refactor begins.

**Critical Path:** F0-1 → F0-2 → F0-3 → F1-1 → F1-3 → Demo Gate

**Gate Requirements:**
- Colorful.gif frames display on hardware with correct colors and sequence
- Reasonable frame pacing is acceptable for the gate; full playback smoothness remains a Phase 2 concern
- Demo Gate must pass before F2 work is treated as unblocked

**Rationale:** Early hardware proof reduces risk, keeps the implementation reversible, and lets the team validate the LED path before deeper runtime refactors.

---

### 13. Demo-First Phase Validation Refinements
**Date:** 2026-04-16  
**Owner:** Dr. Light, Mega Man  
**Status:** APPROVED FOR IMPLEMENTATION

**Summary:** Phase validation criteria were tightened so every phase has an objective exit gate while preserving the fastest credible path to a `colorful.gif` hardware demo.

**Refinements Locked:**
- Phase 0 must prove boot visibility with a serial-ready message within 3 seconds
- Phase 1 acceptance explicitly allows `led.c` refactoring while requiring hardware proof and symbol hygiene
- Phase 2 timing validation must use measurable instrumentation, not subjective comparison
- Phase 3 owns the first true playback demo and serial queue proof
- Phase 4 sign-off requires a concrete hardware stability checklist

**Sequencing Outcome:** Phase order remains intact; a Demo Gate after F1-3 is the earliest credible visual milestone.

---

### 14. F1 Scaffold Validation Gate
**Date:** 2026-04-16 (Session 7)  
**Owner:** Mega Man (Tester)  
**Status:** READY FOR IMMEDIATE IMPLEMENTATION

**Summary:** Mega Man formalized an eight-point reviewer checklist for F1 completion so the Pico scaffold cannot be merged on build theater alone.

**Acceptance Gate Highlights:**
1. Root build baseline remains untouched and compilable
2. `pico_build\` contains the expected firmware and generated-asset layout
3. Firmware emits a visible serial-ready boot signal within 3 seconds
4. Asset-source precedence is explicit and documented
5. Portable seam isolation is demonstrable
6. Symbol audit excludes forbidden POSIX headers and cross-contamination
7. F1 documentation is complete for the next implementer
8. Git history stays reversible by removing `pico_build\` only

**Status Note:** This review is complete; it is a merge gate for Auto's ongoing scaffold implementation, not evidence that implementation is done.

---

### 15. Pico Build Foundation Kickoff
**Date:** 2026-04-15  
**Owner:** Auto  
**Status:** IMPLEMENTED; READY FOR TEAM REVIEW  
**Risk:** Low

**Summary:** Phase 1 kickoff starts from an isolated Pico SDK firmware root under `pico_build\`, not from a rewrite of the legacy root build. The initial target proves the SDK/toolchain path, emits real Pico artifacts, and provides a USB-serial ready stub for later hardware bring-up.

**Locked Decisions:**
1. Keep all Pico build glue inside `pico_build\` while root CMake and root sources stay untouched reference inputs
2. Resolve the SDK with `pico_build\pico_sdk_import.cmake` via cache entry, environment variable, then sibling checkout fallback at `C:\ws\pico-sdk`
3. Default the kickoff board to `pico2`, while keeping board selection overrideable until Plasma-specific wiring is confirmed
4. Keep the first firmware target minimal: `pico_stdlib` only, USB stdio enabled, UART stdio disabled, and `pico_add_extra_outputs()` on for immediate UF2 proof
5. Track `pico_build\assets\generated\` with a `.gitignore` so the generated-asset lane is explicit without checking in generated files

**Validation:**
- Root host configure path stays unchanged; Windows host failure modes remain the same pre-existing POSIX/Linux dependency gaps
- Isolated Pico path produces `.elf`, `.bin`, `.hex`, `.uf2`, and `.dis` artifacts when configured with Pico SDK, ARM GCC, Ninja, and picotool

**Follow-Up:**
- Add board-specific pin selection and protocol configuration once Plasma 2350 W wiring is confirmed
- Introduce copied portable runtime code under `pico_build\src\portable\` while keeping firmware glue isolated in `src\firmware\`

---

### 16. Phase 2 Runtime Seams: Portable Core vs. Platform Shell (Completed)

**Date:** 2026-04-15  
**Owner:** Proto Man  
**Status:** COMPLETED  
**Risk:** Low (architecture validated; implementation pending F1 proof revision)

**Summary:** Phase 2 runtime separation has been validated through architectural scoping. The portable core (TASBot geometry, animation rendering, blink timing) is clearly separated from the Pico platform shell (board startup, frame cadence, `hw_led` sink, USB serial).

**Locked Decisions:**
1. Copy TASBot geometry and mapping logic into `pico_build\src\portable\` as the first Phase 2 deliverable
2. Host-only modules (`main.c`, `led.*`, `network.*`, `filesystem.*`) remain outside the Pico tree
3. Replace `ws2811_led_t` transport at Pico seam with a local RGB888 type
4. Validate display correctness with four-phase deterministic smoke pattern (left eye → right eye → nose field → alignment markers) before GIF playback

**Critical Path:** F2-1 (portable seam) → F2-2 (smoke pattern) → F2-3 (runtime tick instrumentation) → Demo Gate

**Architectural Fit:** 
- Smoke pattern validation requires **no asset pipeline dependency** — deterministic proof path independent of GIF preprocessing
- Legacy runtime's multi-concern call paths (rendering + timing + GIF + sockets + filesystem) are now explicitly segregated
- Real `hw_led` backend implementation awaits Plasma 2350 W wiring confirmation

**Blockers for Implementation:**
- F1 proof revision must complete (Mega Man review gate); Auto revision work in progress
- Plasma 2350 W LED protocol wiring confirmation (Fortinbra action item) before real `hw_led` backend coding

**Follow-Up Actions:**
1. Implement real `hw_led` backend once board wiring is confirmed
2. Port `color.c/.h` when Phase 2 timing instrumentation is in place
3. Smoke pattern serves as Demo Gate milestone before Phase 3 GIF playback work begins

**Reference:** `.squad/orchestration-log/2026-04-15T02-46-33Z-proto-man-runtime-seam.md`

---

### 17. F1 Proof Review Verdict (Mega Man)

**Date:** 2026-04-16  
**Owner:** Mega Man (Reviewer)  
**Status:** REVIEW REJECTION (awaiting revision)  
**Risk:** Medium (scaffold must be revised before Phase 2 unblock)

**Summary:** Mega Man's acceptance review of Auto's Pico scaffold foundation flagged artifact proof gaps. The design and structure passed validation, but build evidence could not be independently reproduced in the review environment.

**What Passed Review:**
1. Isolation: `pico_build\` is independent from root host build
2. SDK pattern: CMakeLists.txt follows Pico SDK conventions (`pico_sdk_import.cmake`, `pico_sdk_init()`, `pico_stdlib`, USB stdio on, UART off, extra outputs enabled)
3. Serial stub: `main.c` and `board.h` define a plausible 2-second-delay boot banner
4. Asset policy: `.gitignore` and documentation make asset-source precedence explicit
5. Symbol hygiene: No forbidden POSIX/RPi headers detected

**Blocking Failures:**
1. **Artifact proof not reproduced:** Pico configure succeeded, but build halted before `.elf`/`.uf2` due to `picotool` nested configure failing on native compiler availability
2. **Boot visibility unproven:** Serial banner is code; no captured log or hardware flash evidence demonstrates the ready signal within the 3-second gate
3. **Reviewer mandate:** Evidence required, not inherited claims; Auto history does not substitute for fresh local validation

**Required Revision:**
- Different agent (not Auto) must perform reproof
- Produce fresh `.elf`, `.bin`, `.hex`, `.uf2`, `.dis` artifacts with local verification
- Capture exact serial boot proof with timing measurement
- Validate root baseline remains untouched and compilable
- Re-check symbol hygiene and documentation completeness

**Sequencing Impact:**
- Phase 1 F1-1, F1-2, F1-3 final merge blocked until reproof passes
- Phase 2 unblock depends on F1 completion; smoke pattern proof cannot start until F1 seam is validated

**Reference:** `.squad/decisions/inbox/mega-man-review-pico-scaffold.md`

---

### 18. F1 Proof Revision & Approval (Dr. Light + Mega Man)

**Date:** 2026-04-16  
**Owner:** Dr. Light (revision), Mega Man (review approval)  
**Status:** APPROVED FOR F1 COMPLETION  
**Risk:** Low (hardware gate deferred to Phase 2)

**Summary:** Dr. Light revised the Pico foundation proof to separate software artifact reproducibility from hardware serial readiness. Mega Man approved the software-side proof as honest, reproducible, and suitable for F1 completion.

**Proof Lanes:**
1. **Local build identity:** Fresh `pico_build\` configure/build reproduces `.elf`, `.bin`, `.hex`, `.uf2`, `.dis`, and records artifact hashes (software proof lane PASSED)
2. **Hardware serial-ready gate:** Explicitly deferred to Phase 2; only closes when a captured board transcript shows the ready banner within 3 seconds of reset (OPEN pending hardware capture)

**Evidence:**
- `pico_build\tools\collect-proof.ps1` reproduces Pico SDK configure and firmware artifacts

---

### 19. Proto Man: Debug Blue-Only Colorful.GIF Output

**Date:** 2026-04-18  
**Owner:** Proto Man  
**Status:** RESOLVED  
**Risk:** Low (root cause isolated; fix applied)

**Summary:** The "all LEDs blue" symptom on real hardware was traced to an asset-packing bug in `pico_build\tools\generate-gif-asset.ps1`, not to the Pico WS2812 PIO transport.

**Root Cause:** The PowerShell script was shifting `System.Drawing.Color` channel bytes without widening them first, causing integer overflow that zeroed the red and green lanes during `0xRRGGBB` assembly.

**Decision:**
1. Keep the existing Pico-side runtime seam and WS2812 PIO driver unchanged
2. Fix the asset converter by casting each color channel to `[uint32]` before `-shl`
3. Regenerate `pico_build\assets\generated\colorful_asset.generated.h` and rebuild UF2 artifacts

**Evidence:**
- Generated header before fix contained blue-only values like `0x0000FF`, `0x0000AA` for frames that should contain white, red, orange, green, cyan
- Source GIF verified as multicolor via independent image inspection
- Failure mode matches observed symptom: transport alive, but payload already wrong at generation time

**Expected Observable After Flash:**
- LED array displays recognizable multicolor sweep (white → red → orange → green → cyan) instead of uniform blue
- Frame checksums vary per frame (confirms color data variation)
- Animation pacing reasonable and recognizable as source `colorful.gif`

**Next Validation:** Flash rebuilt UF2 to real Pico hardware and capture serial output + visual proof of color fidelity.

---

### 20. Mega Man: Triage — All LEDs Blue Symptom

**Date:** 2026-04-16  
**Owner:** Mega Man  
**Status:** CLOSED (root cause confirmed by Proto Man)  
**Risk:** Mitigated

**Summary:** Systemic triage of the "all LEDs blue" failure mode, ranking root cause candidates and prescribing validation checkpoints at four layers: asset proof, link/symbol proof, hardware boot readiness, and playback/color fidelity.

**Root Cause Candidates (Ranked by Probability):**
1. **Color Channel Inversion/Bit Field Corruption [HIGHEST]** — Asset generator or driver packing logic inverts/permutes channels (CONFIRMED as Proto Man's finding)
2. **Asset Not Generated or Loaded [MEDIUM]** — CMake asset generation failed silently
3. **Frame Buffer Corruption or Layout Mapping [MEDIUM]** — Blit path misaligns logical/physical pixel layout
4. **WS2812B Timing or Reset Failures [LOW]** — If any color shows, timing is likely intact

**Validation Checklist (Multi-Layer):**
- **Layer 1 (Asset Proof):** Generated header size ≥ 500B, first frame NOT all 0x0000FF
- **Layer 2 (Link & Symbol):** Firmware ELF contains colorful symbols; UF2 ≥ 50 KB
- **Layer 3 (Hardware Boot):** Serial log shows "tasbot_eyes pico_build booting" + asset metadata within 2 seconds
- **Layer 4 (Playback & Color Fidelity):** Frame 0 checksum ≠ Frame 1 checksum; visual shows ≥2 distinct colors in first 5 frames

**Minimum Observable Evidence for Fix Confirmation:**
- Serial: Frame checksums vary; pixel counts vary per frame
- Visual: ≥2 distinct colors; animation recognizable as source GIF

**Disposition:** Multi-layer validation approach provides early-stage confidence before hardware flash. Proto Man's debug output and asset regeneration satisfy Layer 1 requirements.
- `pico_build\proof\foundation-proof.md` honestly scopes software proof and leaves hardware gate open
- Fresh root build still fails on missing legacy dependencies (isolation confirmed)
- Mega Man reproduced proof script in both default and explicit build directories; artifact hashes match

**Consequences:**
- F1 software foundation proof-of-concept **ACCEPTED**
- Root legacy build remains uncontaminated; continues serving as regression evidence
- Hardware-attached gate will use `pico_build\tools\collect-proof.ps1 -SerialPort COMx` path in Phase 2
- Reviewers can approve F1 scaffold as reproducible even with Pico absent

**Non-blocking Note:** Proof command and hashes should be aligned (current `foundation-proof.md` lists hashes from specific `-BuildDir` argument but reproduction command omits it)

**Reference:** `.squad/decisions/inbox/dr-light-revise-pico-proof.md`, `.squad/decisions/inbox/mega-man-review-proof-revision.md`

---

### 19. Runtime Seam Slice Approval (Mega Man)

**Date:** 2026-04-16  
**Owner:** Mega Man (reviewer)  
**Status:** APPROVED AS PHASE 2 STARTING POINT  
**Risk:** Low (seam is concrete; implementation ready)

**Summary:** Mega Man approved Proto Man's Phase 2 runtime seam design. Isolation held, host dependency leak check passed, seam boundary is concrete and aligned with feature documentation.

**What Passed:**
1. **Isolation rule held:** Root legacy sources untouched; new work under `pico_build\`
2. **Host dependency leak check:** No forbidden POSIX/RPi headers in `pico_build\src\firmware\` or `pico_build\src\portable\`
3. **Seam is concrete:** Agreed boundary: `logical 28x8 frame → TASBot layout mapper → 154 RGB888 transport buffer → firmware hw_led sink`
4. **Geometry carryover:** `pico_build\src\portable\tasbot_layout.c` and `smoke_patterns.c` preserve legacy TASBot mapping in portable code

**Seam Design (Approved):**
- **`pico_build\src\portable\`** — deterministic frame generation and TASBot-specific layout mapping
- **`pico_build\src\firmware\`** — board startup, timing cadence, `hw_led_present_rgb888()` transport boundary

**Build Status Note:** Fresh Pico configure in `pico_build\build\review-4\` shows correct target includes; build blockage in `picotool` host sub-build does NOT invalidate seam decision but keeps runtime slice short of full validation gate.

**Next Measurable Step:** Fix Pico host-tool build path to emit `tasbot_eyes_pico.elf` and `.uf2`, then capture USB-serial ready banner plus one full four-phase smoke-pattern cycle from hardware.

**Reference:** `.squad/decisions/inbox/mega-man-review-runtime-seam.md`

---

### 20. User Directive: WS2812B + PIO LED Output

**Date:** 2026-04-15  
**By:** Fortinbra (user, via Copilot)  
**Status:** CAPTURED FOR TEAM MEMORY  
**Risk:** None (foundational requirement clarification)

---

### 21. WS2812B-over-PIO Implementation Rejection (Mega Man)

**Date:** 2026-04-16  
**Owner:** Mega Man (Reviewer)  
**Status:** REJECTED FOR SIGN-OFF; AWAITING REVISION BY DIFFERENT AGENT  
**Risk:** Medium (blocking Phase 3 hardware gates)

**Summary:** Proto Man delivered WS2812B-over-PIO firmware backend (Phase 2.1 → Phase 3 bridge) with seam isolation confirmed and phase 2 boundary holding. Mega Man review identified three sign-off blockers requiring revision by a different agent before completion.

**What Passed:**
1. **Phase 2 seam held.** `pico_build\src\portable\` owns logical-frame mapping; `pico_build\src\firmware\hw_led_pio.c` consumes RGB888 transport and repacks to WS2812 GRB locally. Isolation confirmed.
2. **Legacy root path stayed isolated.** Fresh Visual Studio root build still fails on expected host-only gaps (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`). No new contamination detected.
3. **Pico/PIO wiring structurally correct.** `pico_build\CMakeLists.txt` links `hardware_pio`; `hw_led_pio.c` uses checked-in `ws2812.pio(.h)`, one state machine, 800 kHz timing, explicit reset gap. Pin resolves to `PLASMA2350_DATA_PIN` / GPIO 15.
4. **Local build proof exists.** `pico_build\tools\collect-proof.ps1` produced `.elf`, `.bin`, `.hex`, `.uf2`, `.dis`, and embedded-banner proof in `pico_build\build\proof-run` and `pico_build\build\ws2812-proof`.

**Blocking Issues (Must Revise):**

1. **Explicit board/protocol configuration gate not met.** `pico_build\src\firmware\board.h` defines timing constants and pin fallback chain but does NOT make protocol, pixel count, and frame-rate contract explicit per the approved "board/protocol config lives in board.h" requirement. Current state is too implicit.

2. **Proof package overclaims reproducibility.** `pico_build\proof\foundation-proof.md` claims `powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1` reproduces published hashes, but published `.elf`, `.dis`, and `.elf.map` hashes only match when script runs with explicit `-BuildDir C:\ws\tasbot_eyes\pico_build\build\ws2812-proof`. Default `proof-run` directory yields different hashes for path-sensitive artifacts.

3. **Hardware proof gate still open.** No captured Pico serial transcript with 10+ checksum cycles; no photo/video evidence of four smoke phases (RED, GREEN, BLUE, WHITE); current evidence is software-side build proof only. Gate explicitly requires hardware capture on Plasma 2350.

**Required Revision (By Different Agent):**

This slice must be revised by an agent other than Proto Man. Revision scope:

1. **Tighten `board.h`** — protocol, pin, pixel count, and smoke cadence/frame-rate must be explicit board-level declarations (not implicit fallbacks)
2. **Fix `foundation-proof.md`** — either name exact build directory in reproduction command OR republish hashes from script's default `proof-run` directory
3. **Rerun proof with hardware capture** — flash `tasbot_eyes_pico.uf2` to Plasma 2350, capture 30-second serial transcript (10+ checksum cycles), add visual proof (photo/video) of four smoke phases

**Sequencing Impact:**
- Phase 3 hardware playback unblocked only after this revision completes and passes follow-up gate review
- Proto Man locked out of this revision cycle per team decision

**Next Measurable Step:**
Different agent revises board/proof contract and captures hardware evidence (serial log + smoke-phase video) on Plasma 2350, then resubmits for gate review.

---
**Directive:** The LED array is all WS2812B LEDs, and the Pico implementation should use PIO for LED output.

**Consequence:** Phase 2 WS2812B-over-PIO runtime work is now the active next slice.

**Reference:** `.squad/decisions/inbox/copilot-directive-2026-04-15T02-46-38Z.md`

---


--- MERGED FROM INBOX 2026-04-15T03:11:42Z ---

# Proto Man: WS2812B PIO Backend for Phase 2

**Date:** 2026-04-16 (Session 13)  
**Requested by:** Fortinbra

## Summary

The Pico-side `hw_led` seam now moves real pixels instead of swallowing metrics. The implementation uses a single-state-machine WS2812 PIO program, takes the existing 154-entry RGB888 transport buffer unchanged, repacks to WS2812 GRB order in the driver, and keeps the rest of the runtime seam untouched.

## Decision

1. Keep the approved seam exactly as-is: `logical 28x8 frame -> TASBot mapper -> 154 RGB888 transport buffer -> hw_led sink`
2. Implement the transport in `pico_build\src\firmware\hw_led_pio.c`
3. Drive WS2812B with Pico PIO at 800 kHz and an 80 µs reset window
4. Resolve the data pin from board macros first, with a hard fallback to GPIO 15 for Plasma hardware alignment
5. Track both `ws2812.pio` and the generated `ws2812.pio.h` in-tree so the Pico build does not depend on rebuilding `pioasm` from source on this Windows workstation

## Why

- The hardware statement is now clear: the array is WS2812B, so the transport should use the Pico's deterministic hardware path rather than a fake sink.
- The portable runtime already stops at RGB888; repacking inside `hw_led` keeps geometry and transport concerns separate.
- Clean builds mattered more than purity here. The local Pico SDK could build firmware fine, but rebuilding SDK `pioasm` from source was blocked by host C++ environment friction. Committing the generated header preserves the standard PIO program while keeping the build reproducible.

## Evidence

- `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2` now builds successfully
- `pico_build\proof\foundation-proof.md` refreshed with the new artifact hashes
- Legacy root build still fails only on pre-existing host dependencies, not because of `pico_build\` contamination

## Follow-up

1. Flash the UF2 to real hardware and capture the four-phase smoke pattern plus serial checksums
2. If refresh rate needs to increase later, the next obvious extension is DMA-fed PIO without changing the current seam


---

# Mega Man: WS2812B + PIO Proof Gate Definition

**Date:** 2026-04-16 (Session 12)
**Task:** Define measurable success criteria for WS2812B over PIO + four-phase smoke pattern slice (Phase 2.1 → Phase 3 bridge)
**Requested by:** Fortinbra

## Summary

The next runtime slice must deliver real LED output backed by Pico PIO instead of the current stub. Success is NOT "the code compiles" or "it might work." Success is:

1. **Build proof:** Firmware compiles + links PIO support; no forbidden headers
2. **Pin/timing expectations:** oard.h declares GPIO, protocol (WS2812B), and frame rate explicitly
3. **Visible hardware behavior:** Physical 8x32 array shows deterministic four-phase pattern (RED left eye, GREEN right eye, BLUE nose, WHITE alignment) at ~750ms per phase
4. **Frame rate stability:** ±50ms drift tolerance; checksums are identical across cycles

## Four-Phase Smoke Pattern (Proven Portable Logic)

From \pico_build\src\portable\smoke_patterns.c\:
- **Phase 0:** Left eye RED outline (8 pixels, x=0–7 y=2–5)
- **Phase 1:** Right eye GREEN outline (8 pixels, x=20–27 y=2–5)
- **Phase 2:** Nose field BLUE (20–30 pixels in logical center, x=9–18 y=1–7)
- **Phase 3:** Alignment WHITE (6 corner/center markers: (2,0), (5,7), (22,0), (25,7), (13,1), (14,6))

Each phase lasts ~750ms; all 4 cycle repeats every ~3 seconds indefinitely.

## Hardware Expectations

- **Board:** Plasma 2350 W (RP2350A, 150 MHz, 256 KB SRAM, 4 MB flash)
- **LED Protocol:** WS2812B single-wire Neopixel (~800 kHz bit clock)
- **Pixel Count:** 154 physical LEDs (8×32 array, 8 bits mapped to logical 28×8 frame)
- **GPIO Pin:** Defined in \oard.h\ after physical wiring confirmed (Fortinbra action)
- **PIO Resource:** One PIO state machine (PIO0 or PIO1, TBD based on final driver design)

## Acceptance Criteria Map

| Gate | Proof | Verifiable By |
|---|---|---|
| Build | \	asbot_eyes_pico.elf\ exists, reproducible across clean builds; no forbidden headers | \ile build/tasbot_eyes_pico.elf\; \grep -r ws2811\\\\|pthread\\\\|gif_lib pico_build/src/\ |
| Pin Config | \oard.h\ defines \TASBOT_WS2812B_DATA_PIN\, \TASBOT_WS2812B_PIXEL_COUNT\, \TASBOT_FRAME_RATE_HZ\, \TASBOT_LED_PROTOCOL\ | Read source; cross-check pin vs. Plasma schematic |
| Visible | Each phase lights correct region in correct color on physical array for ~750ms | USB serial log (4+ cycles) + photo/video of each phase |
| Stability | Frame checksums identical across 10+ consecutive cycles; no crashes | Serial output consistency; 30-second observation with no resets |

## Deliverables Expected from Proto Man

- Revised \src/firmware/hw_led.c\ (real impl, no stub)
- Updated \oard.h\ with concrete GPIO + protocol config
- Clean \pico_build/build/\ artifacts (\.elf\, \.uf2\)
- Serial capture log (timestamp, phases, checksums) from physical board
- Visual evidence (photo/video) of four-phase pattern on 8×32 array

## Out of Scope for This Gate (Deferred to F3+)

- Animation asset loading from GIF files
- Serial command injection (UART animation queue)
- Color dynamics (fades, brightness modulation)
- Frame rate smoothness (timing jitter < 16ms for 60 Hz feel)

These remain Phase 3+ work; this gate only validates that the physical transport layer works correctly and is ready for asset integration.

## Risk Mitigations & Fallback

If PIO approach hits unexpected blocker:
- GPIO conflict → reassign pin (documented in \oard.h\)
- PIO timing margin too tight → unlikely (800 kHz on 150 MHz Pico is not critical), but can switch to DMA-assisted bitbang as fallback
- Byte order mismatch (RGB vs BGR) → swap in hardware mapping table, visible on first test
- Firmware instability → implement watchdog timeout in LED emission loop; revert to stub if needed

## Sign-Off

**Gate is OPEN.** Proto Man to implement.

**Mega Man will re-review** once:
1. Build proof artifact is submitted
2. Serial capture + photos of physical hardware behavior are provided
3. All checksums from 10+ consecutive cycles match within serial log

Rejection or re-request for revision will be documented in project history with root cause.
---

## Dr. Light: Revise WS2812B-over-PIO Sign-Off (Merged 2026-04-16)

# Dr. Light: Revise WS2812B-over-PIO Sign-Off

**Date:** 2026-04-16  
**Requested by:** Fortinbra

## Summary

The rejected WS2812B-over-PIO slice did not need a new architecture. It needed the board contract made explicit in one place and the proof package made honest about path-sensitive artifacts. This revision keeps the approved runtime seam intact, upgrades `board.h` into the single source of truth for the hardware-facing contract, and aligns the proof script's default build directory with the published hash table.

## Decision

1. Keep the approved seam unchanged: `portable logical frame -> TASBot mapper -> 154 RGB888 transport buffer -> firmware WS2812B PIO sink`
2. Require `pico_build\src\firmware\board.h` to declare the runtime-facing board contract explicitly: protocol, resolved GPIO pin, physical pixel count, and smoke cadence/frame-rate
3. Guard the contract with compile-time checks so board constants cannot silently drift away from portable geometry or cadence math
4. Make `pico_build\tools\collect-proof.ps1` default to the published proof path (`pico_build\build\ws2812-proof`) so the documented bare command reproduces the path-sensitive `.elf`, `.dis`, and `.elf.map` hashes
5. Keep hardware-attached proof explicitly open until a real Plasma 2350 capture exists; software proof is sufficient for software-side sign-off only

## Why

- Reviewers should not have to reverse-engineer the real hardware contract from fallback macros spread across firmware code.
- Path-sensitive hashes are not a problem by themselves; pretending they are path-independent is the problem. Aligning tool defaults with the published package removes that ambiguity.
- The transport design was already sound. The correction is about making the contract and evidence reviewable without widening the slice.

## Evidence

- `pico_build\src\firmware\board.h` now declares `WS2812B`, GPIO15, 154 pixels, and 750 ms smoke cadence explicitly

---

## Proto Man: Hardware Proof Pass (Inbox merge 2026-04-18)

# Proto Man hardware proof pass

- **Date:** 2026-04-18
- **Status:** Partial hardware gate closure; flash + runtime loop proven, boot-banner/video gate still open

### What was proven

1. Flashed `pico_build\build\review-colorful-proof\tasbot_eyes_pico.uf2` to the Plasma 2350 while it was mounted as `D:\` in BOOTSEL mode.
2. The board left BOOTSEL, then enumerated a USB CDC serial interface as `COM10`.
3. Serial capture proved the current firmware image is the embedded `colorful.gif` loop, not the older smoke-pattern image:
   - 18 frames
   - 100 ms per frame
   - 15 full cycles captured in `pico_build\proof\hardware-serial-capture.log`
   - per-frame checksums stayed stable across captured cycles
4. Follow-up reflash from BOOTSEL reproduced `COM10` and another asset-loop capture in `pico_build\proof\hardware-boot-capture.log`.

### What stayed blocked

1. The one-shot boot/ready banners were not observed on the host serial captures.
   - The board was clearly alive and streaming frame logs.
   - In this Windows USB-CDC path, opening the COM port after enumeration did not replay the early boot text.
2. No photo/video proof was possible from this environment, so visual playback and color fidelity still need human confirmation on the array.
3. Timing proof is approximate only from host receive timestamps; firmware-side timestamps are still absent.

### Reviewer-relevant takeaway

Treat this pass as honest hardware progress:

- **Closed now:** flash success, serial enumeration, colorful-loop execution, repeated checksum stability
- **Still open:** ready-banner capture, visual proof, firmware-timestamped timing proof

---

## Proto Man: Blue Output Trace Follow-Up (Inbox merge 2026-04-18)

**Date:** 2026-04-18  
**Status:** RESOLVED BY HARDWARE SUCCESS

I traced the live Pico path from generated asset through mapping and transport. The multicolor data is present in `pico_build\assets\generated\colorful_asset.generated.h`, preserved by `pico_build\src\portable\embedded_animation.c`, mapped into diverse 154-pixel transport colors by `pico_build\src\portable\tasbot_layout.c`, and the running firmware reports stable varying frame checksums in `pico_build\proof\hardware-serial-capture.log`.

**Decision:** Treat **stale UF2 selection** as the highest-probability remaining software root cause for a "still all blue" report.

- `pico_build\build\ws2812-proof\tasbot_eyes_pico.elf` contains representative multicolor frame constants
- `pico_build\build\review-colorful-proof\tasbot_eyes_pico.elf` does **not**
- Flash `ws2812-proof` UF2 specifically for next physical validation
- If display is still blue, next suspect is final strip color order at `hw_led_pio.c`, not the asset or mapper path

---

## Proto Man Decision: Matrix order follows physical column serpentine (Inbox merge 2026-04-18)

**Date:** 2026-04-18  
**Owner:** Proto Man  
**Status:** HARDWARE VALIDATION IN PROGRESS

Fortinbra clarified the actual wire order on the Pico-driven LED matrix: the first LED is the top-left lit position, then the chain travels downward in the first logical column, upward in the second, downward in the third, and so on.

**Decision:** Keep the existing 28x8 TASBot occupancy mask, but renumber the Pico-side logical-to-physical map in `pico_build\src\portable\tasbot_layout.c` to a top-left-first column-serpentine order.

**Why:**
- Matches the real hardware harness instead of the previous legacy index order
- Preserves all existing logical frame composition and only changes the transport-buffer index mapping
- Keeps the hot path as a static lookup table rather than introducing runtime coordinate math

**Evidence:**
- Updated mapper verified to cover each physical index exactly once: 0..153
- Rebuilt UF2: `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2`
- UF2 SHA-256: `8A9118FE568CCF6D8F4605D3919344AC5579A0797F3CC511FE9647356FEFB6FB`

**Impact:** After reflashing, logical imagery should align with the real column-snaked hardware: the top-left logical pixel now lands on LED 0 and the image should stop appearing spatially scrambled by the old order.

---

## Dr. Light: Blue-Symptom Crosscheck — Converter Fix Was Necessary but Not Sufficient (Inbox merge 2026-04-18)

**Date:** 2026-04-18  
**Owner:** Dr. Light  
**Status:** ACTIVE — requires follow-up experiment

Independent crosscheck of the entire colorful.gif-to-WS2812 pipeline found **no software bug** that explains "still all blue." The asset converter uint32 cast fix was real (data is now correct), but it was **not the root cause of the blue display**. The actual display symptom persists because it was never caused solely by wrong pixel values.

**Pipeline Verified Correct (6 stages):**

| Stage | Component | Verified |
|-------|-----------|----------|
| 1 | Converter: GIF → RGB888 `0xRRGGBB` | ✓ Pixel values correct in generated header |
| 2 | Animation loader: pixel copy into frame | ✓ Correct indexing |
| 3 | Layout mapper: 28×8 logical → 154 physical | ✓ Index table correct |
| 4 | WS2812B packer: RGB888 → GRB | ✓ `(G<<24)|(R<<16)|(B<<8)` |
| 5 | PIO: 24-bit MSB-first, 800 kHz | ✓ Correct for WS2812B |
| 6 | Build: timestamps align (header, .obj, .uf2 all 2026-04-18 17:21) | ✓ |

**Critical Observation:** No single channel-order permutation produces "all blue" from diverse RGB input. The colorful.gif contains red, orange, green, cyan, blue, magenta, white, and black. A channel swap would produce a *mix* of wrong colors, not uniform blue.

**Remaining Suspects (in likelihood order):**
1. **Stale UF2 on device**: Rebuild happened but reflash did not (or failed silently). Most common "still broken after fix" cause.
2. **Never-validated GRB assumption**: `hw_led_pack_ws2812` hard-codes GRB channel order. The physical LEDs have never been probed for channel order.
3. **Signal integrity**: RP2350 GPIO 15 → Plasma 2350 screw terminal → LED strip. No scope/logic analyzer validation.

**Required Action: Decisive Smoke Experiment**

Proto Man should implement a **blocking pre-animation smoke test** in `main.c`:
1. Set all 154 LEDs to solid RED (`0xFF0000`), hold 3 seconds
2. Set all to solid GREEN (`0x00FF00`), hold 3 seconds
3. Set all to solid BLUE (`0x0000FF`), hold 3 seconds
4. Set all to solid WHITE (`0xFFFFFF`), hold 3 seconds
5. Then enter the animation loop

**Observe and report:**
- If RED→RED, GREEN→GREEN, BLUE→BLUE: packing is correct; issue is stale UF2 or animation path bug
- If RED→GREEN or RED→BLUE: channel order mismatch; adjust `hw_led_pack_ws2812`
- If nothing lights up: wiring/pin/flash issue

**Gate Status:** OPEN. Hardware proof remains unvalidated.

---

## Copilot Directive: Matrix layout clarification (Inbox merge 2026-04-18)

**Timestamp:** 2026-04-18T17:25:09.929-05:00  
**By:** Fortinbra (via Copilot)  
**Status:** CAPTURED

The LED matrix layout is top-left first, then snakes by column: down the first column, right, up the next column, right, down the next, and so on. This user direction was captured for team memory and informed the matrix-order mapper fix.

- `pico_build\build\ws2812-proof\tasbot_eyes_pico.elf` contains representative multicolor frame constants (`0x00FFAA00`, `0x00AAFF00`, `0x0000FFAA`, `0x0000AAFF`)
- `pico_build\build\review-colorful-proof\tasbot_eyes_pico.elf` does **not**
- Therefore, flashing the older `review-colorful-proof` artifact can preserve the pre-fix symptom even though source and generated header are now correct

### Fastest Decisive Experiments

1. Flash `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2` specifically, not `review-colorful-proof`.
2. If the display is still blue after that exact UF2, next suspect is final strip color order at `pico_build\src\firmware\hw_led_pio.c`, not the asset or mapper path.

### Consequence

Do not treat `review-colorful-proof` as source-matching for current colorful playback validation. Use `ws2812-proof` as the canonical rebuilt UF2 for the next physical reflash.

### Outcome (2026-04-18)

**DECISION VALIDATED BY HARDWARE SUCCESS:** The "rainbows" confirmation proves multicolor output is now rendering correctly across the array. This trace-back analysis identified the asset-path integrity and correct UF2 selection as the decisive factors. Hardware milestone achieved.

---

## Proto Man: BOOTSEL Window Flash Decision (Inbox merge 2026-04-18)

# Proto Man — BOOTSEL window flash decision

- **Date:** 2026-04-18
- **Context:** User reported the board was "in bootsel mode again, drive d:" and asked for the newest valid UF2, preferring the blue-output debug/fix build if available.
- **Decision:** Flash `C:\ws\tasbot_eyes\pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2` (SHA-256 `4561963937A6CDA5B836793B5428CE29D17A9589B7EA7ECE6C46069F59C692C8`) because it is the newest validated UF2 under `pico_build\build\` and is newer than the pre-fix `review-colorful-proof` image set.
- **Why:** The latest `ws2812-proof` artifact was built on 2026-04-18 after the blue-output asset fix was identified and after fresh hardware-proof artifacts (`pico_build\proof\hardware-validation-run.txt`, `hardware-serial-capture.log`) were generated the same afternoon. That makes it the best post-fix candidate available in-repo during this BOOTSEL window.
- **Operational note:** `D:\` was gone on first check because the board had already left BOOTSEL; `picotool reboot -f -u` brought it back as `D:\` labeled `RP2350`, the UF2 copy completed, the drive disappeared again, and `picotool info -a` then saw the RP2350 on USB serial instead of BOOTSEL.
- **Immediate blocker:** Late-attached COM10 capture produced no lines, so there is no fresh boot-banner transcript from this flash; on this setup the boot/ready text appears to be one-shot and easy to miss unless capture is armed before reset.
- `pico_build\src\firmware\main.c` emits a board-contract banner and consumes the board contract macros directly
- `pico_build\tools\collect-proof.ps1` reproduced the same `ws2812-proof` artifact hashes from both the bare command and explicit `-BuildDir`
- `pico_build\proof\foundation-proof.md` now documents the exact reproduced hashes and keeps hardware proof separate
- Fresh root Visual Studio build still fails only on legacy host dependencies (`gif_lib.h`, `unistd.h`, `ws2811/ws2811.h`, `dirent.h`, `pthread.h`)

## Follow-up

1. Flash `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2` to the Plasma 2350
2. Capture USB-serial output with 10+ deterministic checksum cycles
3. Capture visual proof of the four smoke phases on the real array



---

### Merged from Inbox\n
# Mega Man: Dr. Light WS2812B-over-PIO Re-review

**Date:** 2026-04-15  
**Requested by:** Fortinbra  
**Verdict:** Software/build proof ACCEPTED in this environment; hardware-attached proof remains OPEN

## What passed

1. `pico_build\src\firmware\board.h` now makes the board/protocol contract explicit enough for review: WS2812B-only, 154 pixels, 750 ms smoke cadence, and GPIO resolved through board macros with GPIO15 as the documented fallback.
2. `pico_build\src\firmware\hw_led_pio.c` keeps the Phase 2 seam intact: portable code hands over a 154-entry RGB888 transport buffer, and the firmware layer alone performs WS2812B/PIO packing and transmission.
3. `pico_build\tools\collect-proof.ps1` now defaults to `C:\ws\tasbot_eyes\pico_build\build\ws2812-proof`, which matches the build directory named in `pico_build\proof\foundation-proof.md`; a fresh local rerun reproduced the published artifact sizes and hashes.
4. Fresh root baseline review still failed only on the expected legacy host gaps (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`), which is the right regression story for this migration phase.

## What did NOT pass yet

This is **not** full hardware sign-off. In this environment there was no attached Pico serial/RP2 device, and the proof package does not include:

- a captured serial transcript from real hardware showing the ready banner inside 3 seconds,
- 10+ consecutive smoke-cycle checksum lines from the physical board,
- visual confirmation (photo/video or equivalent reviewer-visible evidence) that the four smoke phases render correctly on the real 8x32 array.

## Decision

Approve this slice as **software-side Phase 2 WS2812B-over-PIO proof only**. Do **not** represent it as hardware-complete sign-off.

## Next measurable validation step

Flash `pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2` to the Plasma 2350 hardware, then run `powershell -ExecutionPolicy Bypass -File pico_build\tools\collect-proof.ps1 -SerialPort COMx` and submit:

1. a ~30-second serial transcript covering at least 10 checksum-bearing smoke cycles,
2. evidence that the ready banner appears within 3 seconds of reset,
3. visual proof of RED left eye, GREEN right eye, BLUE nose, and WHITE alignment phases at ~750 ms each.


--- MERGED FROM INBOX 2026-04-16T03:51:30Z ---

### 22. # Auto Decision: colorful.gif asset pipeline

## Summary

For the first honest Phase 3 slice, the Pico build now owns an offline `colorful.gif` conversion path that emits generated C data under `pico_build\assets\generated\` and links that asset into the firmware image.

## Decision

1. Keep the conversion flow isolated under `pico_build\tools\` and `pico_build\assets\generated\`.
2. Use a build-time PowerShell converter (`generate-gif-asset.ps1`) with `System.Drawing` so the Windows Pico path can regenerate the first asset without adding new third-party tooling.
3. Make source selection explicit and reproducible:
   - prefer `external\TASBot-eye-animations\gifs\`
   - fall back to root `gifs\`
   - fail loudly if the requested asset is missing from both pools
4. Emit both generated frame data and a metadata sidecar so review can see which source was selected, what candidates existed, and what timing contract was compiled into the UF2.
5. Treat the generated asset contract (`embedded_animation.*` + `colorful_asset.*`) as the seam for future playback work, so later phases can add more animations without changing the WS2812B PIO transport shape.

## Why

- This is the fastest reproducible path to get `colorful.gif` into the UF2 while keeping legacy sources untouched.
- It matches the documented source-precedence rule and makes fallback behavior reviewable instead of implicit.
- It gives firmware a stable asset API now, which reduces churn when the next pass expands beyond the first demo target.

## Evidence

- `pico_build\CMakeLists.txt`
- `pico_build\tools\generate-gif-asset.ps1`
- `pico_build\src\portable\embedded_animation.h`
- `pico_build\src\firmware\colorful_asset.c`
- `pico_build\assets\generated\colorful_asset.metadata.txt`


---

### 23. ---
author: "Mega Man (Tester)"
date: 2026-04-16
subject: "Acceptance Gate: colorful.gif Asset Slice (F3 MVP Demo)"
status: "published"
---

# Acceptance Gate: colorful.gif Asset Slice

## Charter

F3 is the first honest hardware demo phase. The gate closes when colorful.gif loops on the physical Plasma 2350 W array with verifiable frame fidelity, timing precision, and build reproducibility.

## Canonical Source Selection

**Rule 1: Source Precedence**
1. Primary: xternal/TASBot-eye-animations/gifs/others/colorful.gif (submodule)
2. Fallback: gifs/colorful.gif (legacy local, git-ignored)
3. Failure: Explicit build error if neither exists

**Required Evidence:**
- Submodule state confirmed (.gitmodules + HEAD)
- Comparison inventory (frame count, timing) in docs/migration/features/f3-assets-playback.md
- CMakeLists.txt documents which source was selected

---

## Generated Asset Reproducibility

**Rule 2: Offline Pipeline Byte-Identity**

Conversion pipeline must produce byte-identical binary frames across two independent builds on the same machine.

**Output Location:** pico_build/assets/generated/colorful_frames.h (or .c/.h pair)

**Reproducibility Proof:**
- Run converter twice in separate build directories
- Compare SHA256 checksums of generated files
- Requirement: Checksums must match exactly
- Document reproduced hash in pico_build/proof/colorful-assets-proof.md

**Size Constraint:**
- Plasma 2350: ~252 KB usable flash
- Single 60-frame animation @ 462 bytes/frame = ~28 KB (acceptable)
- Warn if > 50 KB per animation

---

## Frame-Count and Timing Fidelity

**Rule 3: Timing Contract**

Generated animation must preserve original GIF's frame count and per-frame delays.

**Proof Checklist:**
1. Frame count match: COLORFUL_FRAME_COUNT == source_frame_count
2. Delay preservation: ±1 unit (±10ms) acceptable
3. Hardware timing: 10+ complete cycles, ±50ms tolerance per cycle

**Serial Log Expected:**
`
[colorful] frame=0 delay=100ms ts=0
[colorful] frame=1 delay=100ms ts=100
...
[colorful] cycle_complete total_ms=5900 expected_ms=6000 drift_ms=100 PASS
`

---

## Build Integration

**Rule 4: Firmware Embedding (Zero Runtime FS)**

1. CMakeLists.txt includes custom command to auto-generate header
2. Target dependency: if source GIF changes, header rebuilds
3. tasbot_eyes_pico.elf links generated symbols (verify with objdump)
4. NO forbidden includes in generated files (gif_lib, dirent, unistd, pthread, ws2811)

**Verification:**
`powershell
grep -r "gif_lib|dirent|unistd|pthread|ws2811" pico_build/assets/generated/
# Expected: 0 matches
`

---

## First Hardware Demo

**Rule 5: Visible Playback on Real Plasma 2350 W**

**Prerequisites:**
- Plasma 2350 W + 8×32 LED array wired to GPIO15
- Serial port accessible (COM port on Windows)
- UF2 flashes without error

**Test Procedure:**
1. Flash UF2 to device
2. Capture serial output for 5+ minutes (10+ cycles)
3. Verify frame checksums: identical across all cycles
4. Measure cycle duration: ±100ms of expected GIF duration
5. Record video: smooth transitions, no corruption, recognizable pattern

**Expected Serial Pattern:**
`
[playback] anim=colorful frame=0 delay=100ms checksum=0xABCD1234
...
[playback] anim=colorful frame=59 delay=100ms checksum=0x11223344
[playback] cycle_count=1 total_ms=6000 PASS
`

---

## Reviewer Checklist (14 Criteria)

| # | Criterion | Evidence | ☐ |
|---|-----------|----------|---|
| 1 | Source selected | docs/migration/features/f3-assets-playback.md lists chosen GIF + reason | ☐ |
| 2 | Frame inventory documented | Submodule frame count, delays logged | ☐ |
| 3 | Converter reproducible | Two builds produce identical .h checksums | ☐ |
| 4 | Generated header size OK | < 100 KB total; logged in proof | ☐ |
| 5 | Code hygiene | No forbidden headers in pico_build/assets/generated/ | ☐ |
| 6 | CMakeLists.txt integrated | Custom command auto-generates on rebuild | ☐ |
| 7 | Symbol audit clean | objdump shows colorful symbols, correct sizes | ☐ |
| 8 | UF2 flashes | Bootloader accepts, no hang on boot | ☐ |
| 9 | Serial ready | "tasbot_eyes pico_build ready" within 3 seconds | ☐ |
| 10 | Playback loop runs | Frame sequence appears, no crash (first 10 seconds) | ☐ |
| 11 | Checksum stable | 10+ cycle identical checksums | ☐ |
| 12 | Timing accurate | Total cycle ±100ms of expected duration | ☐ |
| 13 | Visual clear | Video shows pattern, stable colors, no corruption | ☐ |
| 14 | No regression | Smoke pattern still works if invoked afterward (optional) | ☐ |

---

## Risks & Mitigations

| Risk | Likelihood | Mitigation |
|------|------------|-----------|
| GIF decoder complexity | Medium | Reuse legacy gif.c parsing; limit to RGB888 + delay |
| Flash size blowup | Low | Pre-scan asset size; warn > 50 KB |
| Timing drift | Medium | Per-frame timestamp logging (not just cycle-level) |
| Cross-machine variance | Low | Pin tool/CMake/compiler versions in decisions |
| Submodule absent | Medium | CMakeLists checks path, falls back or fails loudly |
| Asset overwrite | Low | .gitignore in pico_build/assets/generated/ |

---

## Out of Scope (Defer to F4)

- Serial command injection / animation queue
- Other animations (base, blink, startup)
- Color post-processing, gamma, brightness
- Smooth frame rate (16ms jitter)
- Power measurement
- SD card loading

---

## Deliverables (To Merge)

1. **Offline Converter** — pico_build/tools/convert-assets.ps1
2. **Generated Header** — pico_build/assets/generated/colorful_frames.h (auto-generated)
3. **CMakeLists.txt Update** — Custom command + link integration
4. **Proof Document** — pico_build/proof/colorful-integration-proof.md + colorful-playback-proof.md
   - Source GIF path + selection reason
   - Generated asset size & checksums
   - Serial playback log (10+ cycles)
   - Video/photo evidence
5. **Feature Doc** — Update docs/migration/features/f3-assets-playback.md with final design
6. **Team Decision** — This memo

---

## Rationale

This gate treats the colorful.gif asset slice as a regression checkpoint, not a hope. It requires:

1. **Canonical source** (no ambiguity on which GIF was selected)
2. **Reproducible generation** (asset format is locked, build is deterministic)
3. **Timing fidelity** (embedded delays are preserved, not guessed)
4. **Hardware proof** (not just "looks right in simulation"; real Plasma 2350 playback with serial capture)

This is Mega Man: no "probably still works." We build the evidence wall.



---

## MERGED DECISION (2026-04-15T03:56:30Z)

### Mega Man Review: colorful.gif Asset Pipeline - APPROVED

**Date:** 2026-04-15T03:56:30Z  
**Reviewer:** Mega Man  
**Status:** ACCEPTED (Phase 3 Software Milestone, Hardware Gate Open)

# Mega Man review: colorful pipeline

## Verdict

Accept the current `colorful.gif` implementation as the **first software-side Phase 3 asset milestone only**. Do **not** treat it as the full colorful gate closure.

## What passed

1. **Canonical source rule holds.**
   - `pico_build\tools\generate-gif-asset.ps1` prefers `external\TASBot-eye-animations\gifs\` and falls back to root `gifs\`.
   - For this slice it selected `external\TASBot-eye-animations\gifs\others\colorful.gif`.
   - The legacy fallback `gifs\others\colorful.gif` hashes identically, so the current choice is reproducible and non-ambiguous.

2. **Generated asset reproducibility holds.**
   - Two fresh generator runs produced identical SHA-256 values for `colorful_asset.generated.h` and `colorful_asset.metadata.txt`.
   - Those hashes also match the current generated files in `pico_build\assets\generated\`.

3. **Frame-count and timing claims are honest at software level.**
   - Source GIF inspection confirmed `28x8`, `18` frames, and `100 ms` per frame (`1800 ms` total).
   - The generated metadata records the same frame count and delay table.

4. **Pico integration is real.**
   - Fresh `collect-proof.ps1` build succeeded in `pico_build\build\review-colorful-proof`.
   - Firmware links the colorful asset into the ELF and exports `g_tasbot_colorful_animation`, `g_tasbot_colorful_frame_delays_ms`, and `g_tasbot_colorful_pixels`.
   - The runtime is on the WS2812B PIO path, not the old stub path.

5. **Evidence is honest about remaining hardware proof.**
   - The docs still make real hardware playback the actual F3 exit.
   - No reviewed file falsely claims that 10+ cycle timing/checksum/video proof already exists.

## Open gate

Hardware proof is still open. Current source logs per-frame checksum and delay, but the colorful gate still requires captured on-device evidence before anyone can claim full acceptance.

## Reviewer note

`pico_build\CMakeLists.txt` currently regenerates the asset through a custom target and I observed the generation step twice in one clean build. That is not a blocker for this software milestone, but tighten it before full gate closure so the dependency edge is cleaner and the proof story stays boring.

## Next measurable validation step

Flash the built UF2 to the Plasma 2350 setup and capture:

1. serial output covering **10+ full colorful cycles**
2. measured cycle timing against the expected **1800 ms**
3. short video/photo evidence showing stable colors and no frame corruption

That is the next boss fight.


---


## Proto Man — BOOTSEL Reflash (Inbox merge 2026-04-18)

**Date:** 2026-04-18  
**Owner:** Proto Man  
**Status:** EXECUTED; BLOCKED BY HARDWARE REGRESSION

**Context:** Fortinbra reported the Plasma 2350 was back in BOOTSEL on \D:\ and asked for the current matrix-order build to be flashed.

**Decision:** Flash \C:\ws\tasbot_eyes\pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2\ after verifying artifact existence and SHA-256 match \8A9118FE568CCF6D8F4605D3919344AC5579A0797F3CC511FE9647356FEFB6FB\.

**Rationale:** This is the validated matrix-order UF2 in the active \ws2812-proof\ build output; hash matches expected post-fix image.

**Operational Result:**
- UF2 copied successfully to BOOTSEL \D:\ (RP2350)
- BOOTSEL volume disappeared; board re-enumerated as USB Serial Device (COM10)
- Post-enumeration serial probe on COM10 captured no fresh banner bytes
- Confirms BOOTSEL exit and COM-port recovery

**Next Blocker:** Hardware regression discovered post-flash: actual panel is 8x32 (256 pixels), but only ~two-thirds illuminate with mangled image. Proto Man and Dr. Light now tasked to audit pixel-count and serpentine-order mapping.

---

## Hardware Regression Discovery — 8x32 Panel (256 Pixels)

**Date:** 2026-04-18  
**Owner:** Proto Man, Dr. Light  
**Status:** REGRESSION ACTIVE; TEAM RECOVERY IN PROGRESS

**Event:** After matrix-order reflash, user reported hardware symptom: actual panel is 8x32 = 256 pixels, but only ~two-thirds illuminate and image is mangled.

**Constraint Clarification:**
- Hardware: 8x32 serpentine panel = 256 pixels total
- Wiring: Column-serpentine layout (top-left-first)
- Observed Symptom: Partial illumination + scrambled image

**Recovery Actions:**
1. **Proto Man:** Audit and fix pixel-count and serpentine-order mapping in \	asbot_layout.c\ and related indexing code
2. **Dr. Light:** Cross-validate board contract and documentation against 256-pixel serpentine constraint; audit pipeline for index assumptions

**Root Cause Hypothesis:** Pixel mapping or indexing regression in matrix-order code path; likely off-by-one or incomplete serpentine traversal in layout table.

**Impact:** Prevents F3 hardware proof validation until pixel mapping is restored.

---

### Hardware Validation: 256-Pixel 8×32 Panel — SUCCESS
**Date:** 2026-04-18  
**Status:** CLOSED  

User confirmed latest flashed build working on hardware. The 256-pixel 8×32 serpentine panel validation loop is now closed after successful application of the 154→256 contract fix and final flash deployment.

**Summary:**
- ✅ 154→256 pixel contract fix implemented and verified
- ✅ Latest firmware build successfully flashed to hardware
- ✅ Hardware validation confirmed by user
- ✅ Bring-up cycle complete

**Next Steps:** Proceed with 32×8 asset pipeline expansion for full-panel content display.

---

### 17. Proto Man — Flash Latest 256-Pixel UF2 (BOOTSEL Deployment)

**Date:** 2026-04-18  
**Owner:** Proto Man  
**Status:** DECISION — EXECUTED  

**Context:** Fortinbra requested an immediate BOOTSEL flash while the Plasma 2350 was mounted on D:\, using the latest verified 256-pixel Pico build.

**Decision:** Flash C:\ws\tasbot_eyes\pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2 after re-verifying the file exists and SHA-256 matches D55128FDE5396663238FB4978BC3AA6D82110D1DC5FB1FB768BD1D1C2827B108.

**Result:**
- UF2 copied successfully to BOOTSEL D:\ (RP2350)
- BOOTSEL volume disappeared; board re-enumerated as USB Serial Device (COM10)
- Post-enumeration serial attach on COM10 captured no fresh banner bytes
- Confirms BOOTSEL exit and COM-port recovery

**Blocker:** Late serial attach on COM10 captured no fresh banner bytes, so there is still no new boot transcript from this flash.

---

### 18. Display Contract: 224-LED Panel-Size Directive (8×28)

**Date:** 2026-04-18T23:07:14Z  
**Owner:** Proto Man, Dr. Light  
**Status:** ACTIVE — IMPLEMENTATION PASS  

**Context:** User established new active display rule: future GIFs will be 8×28 (224 LEDs). The active display area should be treated as 224 LEDs rather than 154 or the full 256-pixel panel.

**Decision:**
- Canonical active display footprint is now **8 wide × 28 tall = 224 LEDs**
- This becomes the new contract for GIF source generation and firmware mapping
- Physical panel still has 256 positions; the 224-LED constraint defines the semantic "active" area

**Implementation Pass:**
- **Proto Man:** Updating contract/mapper for 224-LED dimensions
- **Dr. Light:** Auditing interpretation and docs to ensure 224-LED contract propagates through pipeline

**Impact:** Future animations target 224 LEDs as canonical; panel geometry and asset pipeline must respect this constraint for consistency.

**Status:** In progress — agents executing contract and documentation updates.

---

### 19. Proto Man: 21-Animation Playlist Registry & Cycling

**Date:** 2026-04-19  
**Owner:** Proto Man  
**Status:** IMPLEMENTED

**Context:** Replaced single-animation loop (colorful.gif) with a generalized 21-animation sequential playlist cycling through all animations in order, then wrapping.

**Architecture Decisions:**

- **Asset Generation:** generate-gif-asset.ps1 now accepts -SymbolPrefix parameter. All hardcoded colorful/COLORFUL names are parameterized. Output filename: {SymbolPrefix}_asset.generated.h.
- **Registry Generator:** generate-animation-registry.ps1 is the new top-level orchestrator. It calls generate-gif-asset.ps1 for each of the 21 animations in order, then writes nimation_registry.generated.h containing all 21 	asbot_embedded_animation_t struct instances and kTasbotAnimationPlaylist[].
- **CMake Target:** generate_colorful_asset → generate_all_assets. Calls generate-animation-registry.ps1. Primary BYPRODUCT: nimation_registry.generated.h.
- **Firmware:** nimation_registry.c/h expose g_tasbot_animation_playlist and g_tasbot_animation_playlist_count. main.c uses these instead of g_tasbot_colorful_animation. Frame exhaustion triggers animation advance with modular wrap.
- **Deleted:** colorful_asset.c, colorful_asset.h — superseded by registry.

**Playlist Order (21 animations):**
startup → base → blink → colorful → heart_eyes → coin_eyes → smile → smirk → uwu → disc → asteroids → portal_eyes → whirl → loading → look_left_up → look_strangly_up → dead → file → testbot → gray → twink

**Bug Fixed:** Legacy gifs/ pool has blink.gif at root AND blinks/blink.gif — both rank 0, causing ambiguity. Fixed with try/catch around legacy Select-AssetCandidate: if external submodule succeeds, legacy ambiguity is demoted to comparison_state=legacy-ambiguous and does not fail the build.

**Validation:**
- Registry generator ran clean for all 21 animations
- collect-proof.ps1 build succeeded
- ELF contains all required banners including runtime seam: logical 28x8 active frame → column-serpentine mapper → 256 LED physical transport
- New UF2 SHA-256: 164CB03E00AC3E7F9451445B316E29627431FA0CDD3C1CCAEF068DF0CBECEBC9

**Impact:** All 21 animations now cycle automatically on board startup. Single-animation loop fully replaced.

---

### 20. Dr. Light: Active Display Area Is 8×28 = 224 LEDs on 8×32 Physical Panel

**Date:** 2026-04-19  
**Owner:** Dr. Light  
**Status:** ACTIVE — new authoritative contract

**Ambiguity Resolution:**
Fortinbra clarified: "We can subtract the last 4 rows off the matrix, as the gif images will all be 8x28. which is still 224 leds, not the 154 that were there originally."

The word "rows" means columns (confirmed by arithmetic: 8 × 28 = 224 ✓). The active display area is **8 rows × 28 columns = 224 LEDs**. The last 4 physical columns (28–31) are intentionally dark. The physical panel remains 8×32 = 256 LEDs on the WS2812B chain.

**Authoritative Contract:**
1. GIF content size is 28 wide × 8 tall. This is the permanent source art size. All GIF assets will be 8×28.
2. Firmware drives all 256 physical LEDs. Transport buffer stays 256 pixels. Columns 28–31 are always zeroed (dark).
3. Content is left-aligned. The 28-wide content occupies columns 0–27. No centering. Dark zone is on right edge.
4. 224 is the new active pixel count, replacing both 154 (sparse face mask) and 256 (full rectangle).

**Code Status — Firmware Already Correct:**
| File | Contract | Status |
|---|---|---|
| runtime_types.h | TASBOT_LOGICAL_WIDTH 28u, TASBOT_ACTIVE_LED_COUNT 224u, TASBOT_PHYSICAL_WIDTH 32u, TASBOT_PHYSICAL_LED_COUNT 256u | ✅ |
| board.h | Active 8×28 = 224. Physical 8×32 = 256. Six static asserts tie both layers. | ✅ |
| board.h banner | "board contract: WS2812B on GPIO15, active 8x28 (224 pixels) within physical 8x32 (256 pixels)" | ✅ |
| main.c | 256-pixel physical buffer; banner says "28x8 active frame → 256 LED physical transport" | ✅ |
| tasbot_layout.c | Clears all 256, maps only columns 0–27 (indices 0–223). Columns 28–31 stay dark. | ✅ |
| embedded_animation.c | Rejects content wider than 28. Loads into 28×8 logical frame. | ✅ |
| hw_led_pio.c | Pushes all 256 physical LEDs via PIO. | ✅ |
| generate-gif-asset.ps1 line 182 | Enforces exactly 28×8 input. | ✅ |
| collect-proof.ps1 line 92 | Updated findstr patterns match new ELF banners. | ✅ |
| smoke_patterns.c | All coordinates within 0–27 column range. | ✅ |

**Documentation — Needs Refresh:**
| File | Issue |
|---|---|
| foundation-proof.md lines 40–41, 58–69 | Stale 154-pixel contract with old artifact hashes and banners. Must regenerate after next proof build. |
| docs/migration/features/f2-core-runtime.md line 59 | Still says "logical 32x8 frame" and "256-pixel RGB888 buffer." Should say "logical 28x8 active" and clarify 256-pixel physical transport. |
| docs/migration/features/f3-assets-playback.md lines 39–40 | Says "current colorful.gif art is 28x8" (implies temporary). Should state 28×8 is permanent. |
| pico_build/README.md lines 64–66 | Says "still 28x8" (transitional). Reword to confirm 28×8 is permanent content contract. |
| decisions.md decision #1 | Recommended centering 28×8 in 32-wide panel (2 blank columns each side). Per Fortinbra: no centering. Left-aligned at column 0. Last 4 columns permanently dark. |

**Directive to Proto Man:**
Treat 8×28 = 224 as the active display area on the physical 8×32 panel. Firmware is already correct. Remaining work:
1. Regenerate foundation-proof.md after next successful proof build
2. Update four documentation files above to reflect permanent 28×8 content and 224/256 split
3. No code changes needed — active/physical split properly implemented and static asserts enforce it

**Gate Status:** Hardware validation gate remains OPEN until all 256 physical LEDs are driven (224 lit with content, 32 dark) and displayed image is visually recognizable.

---

### 21. Proto Man: 224-Active-LED Flash — UF2 SHA-256 Record

**Date:** 2026-04-19  
**Owner:** Proto Man  
**Status:** COMPLETE

**Summary:** Firmware rebuilt with 224-active-LED / 256-physical-LED constants and flashed to Plasma 2350 board in BOOTSEL mode.

**Constants in Build:**
- TASBOT_LOGICAL_WIDTH = 28
- TASBOT_ACTIVE_LED_COUNT = 224
- TASBOT_PHYSICAL_LED_COUNT = 256
- Board contract banner: active 8x28 (224 pixels) within physical 8x32 (256 pixels)

**Artifact:**
| File | SHA-256 |
|------|---------|
| pico_build\build\ws2812-proof\tasbot_eyes_pico.uf2 | 164CB03E00AC3E7F9451445B316E29627431FA0CDD3C1CCAEF068DF0CBECEBC9 |

**Flash Result:**
- UF2 copied to D:\ (BOOTSEL drive)
- D:\ ejected automatically — board rebooted successfully
- Flash: ✅ SUCCESS

**Build Note:** CMake requires Python3. Must add C:\Users\thegu\.pico-sdk\python\3.12.6 to PATH before invoking collect-proof.ps1 (Windows App Store Python stubs do not satisfy FindPython3).

