# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Dr. Light joined as Lead for the Pico SDK migration.

## Recent Updates

📌 Team roster finalized on 2026-04-15.
📌 **2026-04-15 (Session 2):** Four-phase migration strategy captured and merged to decisions.md. Team consensus on phased abstraction, conditional CMake, embedded ROM assets, serial injection.

## Learnings

Migration work will need architecture decisions around hardware control, asset handling, and build layout.

### Session 2 (2026-04-15)

**Completed:**
- Analyzed current architecture: identified 8 core modules with varying levels of platform coupling.
- Mapped RPi-specific assumptions: ws2811 driver, POSIX threading, signals, filesystem paths, UDP networking.
- Produced **four-phase migration strategy** with highest-risk seams first.
- Flagged **critical coupling points**: LED driver (Phase 1), threading model (Phase 2), filesystem/assets (Phase 3).

**Key Findings:**
- Core animation & rendering logic is largely platform-agnostic; abstraction layer is the path forward.
- Network layer (UDP injection + WLED realtime) is infeasible on Pico without hardware additions; recommend serial alternative.
- Threading model must shift from pthreads → event loop (Pico has no preemptive scheduler).
- Filesystem: Pico has no native FS; recommend embedded ROM assets + serial injection for flexibility.

**Decision Merged:** Four decisions captured in `.squad/decisions.md`:
1. Four-Phase Pico SDK Migration Strategy (owner: Dr. Light, pending review)
2. Pico SDK Build System Rewrite (owner: Auto, ready for implementation)
3. Pico Runtime Seams (owner: Proto Man, blockers pending team clarification)
4. Migration Validation Gates (owner: Mega Man, gates approved)

### Session 3 (2026-04-15)

**Completed:**
- Reviewed migration plan scope and existing team decisions (Proto Man, Auto, Mega Man).
- Translated architecture into **executable feature breakdown** with sequencing and dependencies.
- Created `FEATURE_BREAKDOWN.md`: 14 concrete features (F1-1 through F4-4) with acceptance criteria, risk levels, and implementation notes.
- Established **critical path**: F1-1 → F1-2 → F1-3 → F2-1 → F2-2 → F2-3 → F2-4.
- Defined **parallelizable work**: F3 (assets) and F4 (deprecations) can run in parallel after F2-3.
- Documented **6 checkpoint gates** for validation and risk management.

**Key Architecture Decisions Formalized:**
1. **Hardware abstraction first (F1):** `hw_led.h` interface seals platform boundary; led.c becomes platform-neutral buffer manager.
2. **Event-loop threading (F2):** Pico main loop replaces pthreads; timer callbacks drive render/hue/animation ticks at 10 ms intervals.
3. **Embedded + serial assets (F3):** ROM budget ~1.5 MB accommodates 3–5 key animations; serial injection (`Q;animation_name\r\n`) replaces UDP.
4. **Platform-conditional stubs (F4):** Network servers, palette file I/O, CLI parsing are compile-gated behind `#ifdef PICO_SDK`, not deleted.
5. **Strict dependency enforcement:** Phase 1 (HW) → Phase 2 (threading) → Phase 3 (assets) → Phase 4 (cleanup); prevents false progress and integration disasters.

**Sequencing Rationale:**
- Phase 1 is blocking: Pico firmware depends on abstracted LED output; F1-3 (LED driver on real hardware) is highest-risk work.
- Phase 2 depends on Phase 1: Once hw_led interface is proven, we can refactor timing. F2-4 (timing validation) is perceptible regression detector.
- Phases 3–4 are decoupled from Phase 2 once event loop is stable; assets and deprecations can proceed in parallel.
- Final validation (F4-4) requires all phases complete; checkpoint gates prevent integration disasters.

**Technical Details Specified:**
- **Pico Target:** RP2040 (2 MB flash, 264 KB RAM). Estimated Pico SDK + core code = ~500 KB; ~1.5 MB available for assets.
- **LED Protocol:** WS2812B/SK6812 bit-banging via PIO or SPI. No WLED network support on bare Pico.
- **Asset Format:** RGB888 (3 bytes per pixel) embedded as C arrays. Preconvert GIFs offline with asset generator.
- **Timing Tolerance:** ±5% on frame intervals; hue updates must appear smooth to human eye.
- **Serial Protocol:** `Q;animation_name\r\n` (queue), mirrors UDP injection for familiarity.

**Risk Mitigations:**
- F1-3 (LED driver): Keep RPi build as fallback; use logic analyzer for protocol validation.
- F2-3, F2-4 (Event loop + timing): Frame-by-frame comparison with RPi baseline; detailed tolerance spec.
- F3-2 (Asset embedding): Pre-measure assets, profile final binary size; ROM budget is adequate for MVP.
- F4 (Deprecations): Conditional compile; preserve code with `#ifdef` guards.

**File Layout Post-Migration:**
- Platform abstraction: `src/platform/{hw_led.h, hw_led_pico.c, hw_led_rpi.c, asset_loader.h, serial_control.h}`
- Core (unchanged): `tasbot.c/h, gif.c/h, color.c/h, stack.c/h, led.c (refactored), palette.c`
- Main entry: `main_pico.c` (event loop), `main_rpi.c` (preserved)
- Assets: `assets/{CMakeLists.txt, embedded_assets.c, gifs/}`
- Build: Root `CMakeLists.txt` with conditional `PICO_SDK_PATH` detection.

**Next Steps (For Team):**
1. **Fortinbra:** Review `FEATURE_BREAKDOWN.md` and sign-off; confirm timeline and resources.
2. **Coder:** Implement F1-1 (Define Hardware LED Interface); this is the foundation.
3. **Dr. Light:** Gate F1-2 and F1-3 after architecture review; checkpoint gates prevent rework.
4. Continue through phases; all work flows through checkpoint gates to ensure quality.

### Session 4 (2026-04-15)

**Completed:**
- Reviewed Pimoroni Plasma 2350 W board specification (shop page + Pimoroni plasma GitHub repo).
- Analyzed impact on architecture, sequencing, and risk profile.
- Cross-checked board specs against Phase 1–4 assumptions in FEATURE_BREAKDOWN.md.
- Recorded decision in `.squad/decisions/inbox/dr-light-board-analysis.md`.

**Board Analysis Summary:**

**Key Findings:**
1. ✅ **Plasma 2350 W validates all three critical assumptions:**
   - Standard LED protocols: WS2812B + APA102 (as designed in hw_led abstraction)
   - Adequate flash: 4 MB (vs. 2 MB RP2040)
   - Mature reference code: Pimoroni plasma MicroPython repo available

2. ✅ **RP2350A processor is a straight upgrade:**
   - Faster (150 MHz vs. 125 MHz); larger RAM (520 KB vs. 264 KB)
   - No architectural changes needed; net positive on headroom
   - Zero code impact

3. ✅ **Asset ROM budget risk drops from "medium" to "low":**
   - 4 MB flash enables 3.5 MB asset budget (vs. 1.5 MB on RP2040)
   - Eliminates F3 blocker "flash space too tight"
   - Allows 10+ embedded animations without SD card fallback

4. ✅ **Wireless (CYW43439 RM2) is out-of-scope bonus:**
   - Not required for v1 (local playback)
   - Hardware ready if v2 wants UDP injection or WLED realtime
   - No code changes; Phase 4 cleanup documents for future reference

5. ⚠️ **RP2350 A4 stepping is current; A2 variants have errata (not code-relevant):**
   - A2 errata (E9) affects input pull-down sensitivity
   - Not applicable to LED output; procurement awareness only

**Architecture Impact:** NONE. hw_led.h abstraction was designed for exactly these protocols. No code changes needed.

**Sequencing Impact:** NONE. Phase 1 → 4 remains locked. No reordering required.

**Risk Profile Impact:**
| Risk | Before → After |
|------|--------|
| Flash space insufficient | Medium → Low |
| LED timing too strict | Low → Low (RP2350A @ 150 MHz makes bitbanging easier) |
| RAM for animation buffers | Low → Very Low (520 KB vs. 264 KB) |
| Board electrical spec unclear | Medium → Low (Pimoroni docs are authoritative) |

**Technical Reference (Phase 1 Kickoff):**
- LED protocols: WS2812B (single-wire, ~800 kHz) or APA102 (clock+data, SPI-like)
- Phase 1 owner must document exact GPIO pins by cross-referencing Plasma 2350 W schematic + Pimoroni plasma MicroPython source

**Decision for Team:**
✅ Proceed with FEATURE_BREAKDOWN.md as-is; no sequencing changes.
✅ Downgrade Phase 3 asset risk from "medium" to "low" in risk register.
✅ Update decisions.md with improved risk profile snapshot.
⚠️ Confirm procurement sources RP2350 A4 stepping (current stock, not older A2 variants).

**Learnings:**
- Board selection risk was premature: vendor documentation and reference implementations exist and validate our assumptions.
- Sequencing lock is sound: the critical path (F1 HW → F2 threading → F3 assets → F4 cleanup) is the right order.
- Risk mitigation works: by deferring board-specific details to Phase 1, we've kept architecture flexible and can adapt to RP2350A advantages without cascade.
- Reference code matters: Pimoroni's MicroPython plasma repo is a good validation source for Phase 2 (timer/ISR) and Phase 3 (asset loading patterns).
