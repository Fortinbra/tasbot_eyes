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
