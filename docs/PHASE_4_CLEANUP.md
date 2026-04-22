# Phase 4: Remove/Stub Unsupported Features (MEDIUM)

**Status:** Pending Implementation  
**Scope:** Disable unsupported features; final cleanup and validation  
**Target Validation:** Pico firmware contains zero POSIX or RPi-specific symbols  
**Effort Estimate:** 1 week  
**Owner:** Firmware engineer  
**Date:** 2026-04-15  

---

## Problem Statement

Pico does not support certain RPi/Linux-only features:
- WLED UDP realtime control (no network on bare Pico)
- Palette file I/O (no filesystem)
- CLI argument parsing (no command-line interface)

Rather than delete code, we'll preserve it with conditional compilation for future maintainability. Phase 4 is cleanup and final validation.

**End Goal (Phase 4):** Pico binary is clean, feature-complete for MVP, and documented for future expansion.

---

## Features in Phase 4

### F4-1: Disable WLED Realtime Control on Pico

**Description:**  
Stub out WLED UDP realtime control for Pico (leave functional on RPi).

**What Gets Done:**
1. Identify WLED code in `network.c` and `led.c`

2. Create `#ifdef PICO_SDK` guards:
   - `startRealtimeControlServer()` → no-op on Pico
   - `handleRealtimeCommand()` → removed from Pico build
   - LED nose color update callback → no-op on Pico

3. Document in README:
   - "Pico version does not support WLED UDP realtime control"
   - "Use serial animation injection instead"

4. Optional: Plan serial variant of WLED protocol (future work)

**Acceptance Criteria:**
- [ ] Pico firmware compiles without WLED code
- [ ] RPi firmware still supports WLED realtime control
- [ ] README clearly states Pico limitation
- [ ] No build warnings or linker errors

**Depends On:** F2-3 (Phase 2 complete)

**Risk Level:** Low

---

### F4-2: Disable Palette File I/O on Pico

**Description:**  
Stub palette file loading on Pico; use built-in default palette instead.

**What Gets Done:**
1. Modify `palette.c`:
   - On Pico: Use built-in hard-coded default palette
   - On RPi: Support file loading as before

2. Identify palette file usage:
   - CLI argument handling
   - Palette loading in `main.c` or `arguments.c`

3. Create default palette:
   - Hard-code 8–16 vivid colors for Pico
   - Document in code comments

**Acceptance Criteria:**
- [ ] Pico firmware does not attempt to load palette files
- [ ] Pico uses default built-in palette
- [ ] RPi firmware preserves file-based palette loading
- [ ] CLI argument parsing adapts to platform

**Depends On:** F2-3

**Risk Level:** Low

---

### F4-3: Remove CLI Argument Parsing on Pico

**Description:**  
Stub or remove CLI argument handling on Pico (not applicable in firmware).

**What Gets Done:**
1. Identify argument parsing in `arguments.c` and `main.c`

2. Create platform-specific entry points:
   - `main_rpi.c`: Full argument parsing
   - `main_pico.c`: Hard-coded defaults

3. Define compile-time configuration for Pico:
   - GPIO pin (define)
   - LED count (define)
   - Brightness (define)
   - Animation mode (compile-time or serial override)

**Acceptance Criteria:**
- [ ] Pico firmware does not link `arguments.c`
- [ ] Pico defaults documented in code or README
- [ ] RPi firmware preserves argument parsing
- [ ] Configuration changes on Pico require recompilation (acceptable for MVP)

**Depends On:** F4-2

**Risk Level:** Low

---

### F4-4: Final Validation and Cleanup

**Description:**  
Verify Pico build contains zero POSIX/RPi symbols. Clean up dead code and documentation.

**What Gets Done:**
1. Perform final build audit:
   - `nm` or `objdump` on final `.elf` to confirm no POSIX symbols
   - Check linker map for unexpected libraries (ws2811, pthread, etc.)
   - Verify firmware includes only Pico SDK + core code

2. Clean up dead code (comment out, do NOT delete):
   - Network server code not used on Pico
   - POSIX threading code
   - Filesystem scanning (if not needed)

3. Update documentation:
   - README: "Pico version limitations"
   - BUILD.md: Platform-specific build instructions
   - FEATURES.md: Feature matrix (RPi vs. Pico)

4. Create final validation checklist:
   - [ ] Build succeeds on Pico
   - [ ] No POSIX library symbols in firmware
   - [ ] No ws2811 symbols in firmware
   - [ ] LED rendering verified on hardware
   - [ ] Animation playback verified on hardware
   - [ ] Serial injection verified on hardware

**Acceptance Criteria:**
- [ ] Pico binary contains zero POSIX or RPi-specific symbols (verified: `nm tasbot_eyes.elf | grep -E 'pthread|ws2811|getgrent|stdio|dirent'` returns nothing)
- [ ] Final binary is < 250 KB (leaves space for future features)
- [ ] Documentation is complete: README lists Pico limitations, BUILD.md has platform-specific instructions, FEATURES.md shows feature matrix
- [ ] Hardware validation checklist completed and signed by developer:
    - [ ] Firmware flashed to target board without errors
    - [ ] Colorful.gif plays visually without glitches or frame corruption
    - [ ] Animation transitions smooth (no black frames, color shifts, or stuttering)
    - [ ] Serial injection accepted: `Q;base\r\n` queues next animation within 1 second
    - [ ] Extended run (30 minutes) completes without timing drift, LED flickering, or crash
    - [ ] Code review sign-off on platform separation (hw_led abstraction, no platform mixing)

**Depends On:** F4-1, F4-2, F4-3

**Risk Level:** Low

---

## Cross-Feature Validation

### Checkpoint 6: Control-Path Outcome
**Gate:** Must pass before release

- Serial injection protocol documented and tested
- Firmware behaves safely when commands unavailable
- Upgrade path clear (UDP → Serial migration documented)

---

## Success Criteria for Phase 4

The phase is **COMPLETE** when:

1. **Unsupported features are cleanly stubbed:**
   - WLED, palette I/O, CLI all conditionally compiled
   - Code is preserved with `#ifdef` guards
   - RPi version unchanged

2. **Platform separation is verified:**
   - Zero POSIX/RPi symbols in Pico binary
   - No compile warnings
   - Clean linker output

3. **Documentation is complete:**
   - README lists all Pico limitations
   - BUILD.md has step-by-step Pico build instructions
   - FEATURES.md explains what works where
   - Architecture.md explains the split

4. **Hardware validation is complete:**
   - Pico firmware has been flashed and tested
   - All core features work on actual board
   - Performance meets expectations

---

## Effort & Timeline

**Estimated Effort:** 1 week  
- F4-1: 1 day (WLED stubbing)
- F4-2: 1 day (palette file I/O)
- F4-3: 1 day (CLI argument parsing)
- F4-4: 2–3 days (audit + documentation)

**Timeline:** 1 half-to-full sprint

---

## Notes for Team

- Phase 4 is low-risk cleanup; unlikely to cause regressions
- Documentation is as important as code in this phase
- Use this as an opportunity to write a comprehensive architecture guide
- Consider creating a "Future Ideas" section documenting potential enhancements
