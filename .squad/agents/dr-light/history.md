# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Dr. Light leads the Pico SDK migration architecture.

### Historical Summary (Sessions 2–6)
- Established the four-phase migration strategy around the three critical seams: LED driver, threading/timing, and assets/filesystem.
- Broke the work into executable features with checkpoints, then inserted **Phase 0** after Fortinbra clarified hard constraints: root sources stay read-only, Pico work lives in an isolated subdirectory, offline GIF preprocessing is mandatory, and `colorful.gif` is the first validation target.
- Validated Pimoroni Plasma 2350 W assumptions: RP2350A, ample flash/RAM headroom, standard WS2812B or APA102-style LED protocols, optional wireless out of scope for v1.
- Approved the documentation/submodule architecture: migration planning under `docs\migration\`, maintained animation sources in `external\TASBot-eye-animations\`, graceful fallback to local `gifs\`, and generated firmware assets under `pico_build\assets\generated\`.
- Locked the sequencing principle that Phase 0 precedes hardware work, hardware proof precedes timing refactors, and later asset/cleanup work can parallelize only after runtime stability is proven.

## Recent Updates

- 📌 Team roster finalized on 2026-04-15.
- 📌 **2026-04-15 (Session 2):** Four-phase migration strategy captured and merged to decisions.md. Team consensus on phased abstraction, conditional CMake, embedded ROM assets, and serial injection.

## Learnings

- Migration success depends more on seam control than on superficial build progress.
- The architecture should preserve the legacy host build, isolate Pico work, and make every phase exit criterion observable.
- `colorful.gif` on real hardware is the earliest meaningful proof point; smooth playback and fuller feature parity come later.

### Session 7 (2026-04-15): Decision Documentation & Architecture Finalization

**Completed:**
- Reviewed and approved Auto's docs reorganization decision
- Reviewed and approved submodule architecture with conditions
- Analyzed Phase 0 implementation impact (no code changes, structure only)
- Documented constraints in .squad/decisions/inbox/ for team merge

**Decisions Approved:**
- **docs-and-submodule-architecture.md** (Dr. Light): APPROVED with Phase 3 follow-up conditions
- **project-structure-constraints.md** (already approved Session 5): Locked into phase sequence

**Conditions for Phase 3 Implementation:**
1. Asset pipeline must handle missing submodule gracefully (no hard dependency)
2. Define asset selection algorithm (which source wins for duplicates)
3. Document .gitignore clarity (legacy vs. external vs. generated)

**Risk Revalidation:**
- Phase 0 complexity: LOW (structure + CMake setup only)
- Asset preprocessing complexity: LOW (handled in F0-2)
- Submodule optional: CONFIRMED (graceful fallback path exists)

**Status:** READY FOR PHASE 0 KICKOFF (pending Fortinbra board/naming confirmation)

### Session 8 (2026-04-15): Phase Review & Optimized Demo Path

**Requested by:** Fortinbra  
**Objective:** Review each documented phase for concrete validity and re-sequence toward fastest credible colorful.gif-on-array demo.

**Completed:**
- Reviewed all five phases (Phase 0–4) for concreteness and validatability
- Confirmed all acceptance criteria are measurable; no vague language
- Analyzed critical path for colorful.gif demo (fastest credible path)
- Identified "Demo Gate" opportunity after F1-3 (basic LED rendering works before animation system refactor)
- Approved two implementation paths: Fast Demo (3–4 days) and Full MVP (3–4 weeks)
- Recorded decision in `.squad/decisions/inbox/dr-light-phase-review-and-demo-path.md`

**Key Findings:**
1. **All phases are validatable:** No phase has vague acceptance criteria or floating dependencies.
2. **Sequencing is correct:** Phase 0 → Phase 1 → Phase 2 → [Phase 3 + Phase 4 parallel] is the right order.
3. **Critical path for colorful.gif:**
   - Fast path: F0 → F1-1 → F1-3 only (3–4 days); displays static colorful.gif frames; no animation system
   - Full path: All phases F0–F4 (3–4 weeks); smooth animation playback with timing validation
4. **Demo gate recommendation:** After F1-3, pause and validate colorful.gif on hardware before proceeding to Phase 2 threading refactor (high-risk work)
5. **Phase 2 reframe:** Not "replace threading" but "validate animation timing"; Phase 1 already proves hardware works

**Architecture Decisions Locked:**
- Phase 0 must complete first (SDK structure + asset preprocessing)
- Phase 1 hardware abstraction is blocking (LED driver is critical path)
- Demo gate after F1-3 prevents surprises in Phase 2
- Phases 3 + 4 can run in parallel after F2-3 (asset pipeline and cleanup are independent)

**Risk Profile (Unchanged):**
- Phase 0: LOW (structure only)
- Phase 1: HIGH (F1-3 requires hardware validation)
- Phase 2: HIGH (timing perception; cannot parallelize with Phase 1 final gate)
- Phase 3: MEDIUM (asset pipeline is well-understood; graceful degradation exists)
- Phase 4: LOW (cleanup; no new code paths)

**Validation Gates (All Concrete):**
- Checkpoint 1: Pico build identity (after F1-3) — Zero POSIX symbols
- Checkpoint 2: Platform seam isolation (after F2-3) — Core code compiles without POSIX headers
- Checkpoint 3: Asset path replacement (after F3-3) — startup/base/blink loadable
- Checkpoint 6: Control-path outcome (after F4-4) — Final audit confirms zero POSIX symbols
- **NEW: Demo gate** (after F1-3) — Visual validation: colorful.gif displays on hardware

**Learnings:**
- **Concreteness matters:** Phases with vague acceptance criteria invite rework. All phases here are measurable.
- **Demo-first thinking:** Early hardware validation (Demo Gate after F1-3) reduces risk in Phase 2 (threading refactor).
- **Validation gates are insurance:** Each checkpoint confirms real progress; prevents "looks done" false positives.
- **Sequencing lock is sound:** Cannot parallelize Phase 0 or Phase 1; later phases decouple after F2-3.
- **Fast Demo is valid:** Can show colorful.gif on hardware without animation system, but full MVP requires Phase 2.

**Next Steps:**
1. Fortinbra: Confirm approval of Demo Gate strategy
2. Coder: Begin Phase 0 (F0-1, F0-2, F0-3)
3. Dr. Light: Review F1-3 output before Phase 2 kickoff; gate Phase 2 start
4. Team: Use checkpoint gates to validate progress; do not proceed past gate until validation passes
