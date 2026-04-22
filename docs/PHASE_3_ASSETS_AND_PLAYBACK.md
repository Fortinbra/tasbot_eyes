# Phase 3: Handle Filesystem & Assets (MEDIUM RISK)

**Status:** Pending Implementation  
**Scope:** Build asset preprocessing pipeline; embed and load animations  
**Target Validation:** Startup, base, and blink animations playable from ROM  
**Effort Estimate:** 1–2 weeks  
**Owner:** Firmware engineer / Roll  
**Date:** 2026-04-15  

---

## Problem Statement

Pico has no filesystem. Current animation loading relies on dynamic GIF decoding from `./gifs/` directory. We must:

1. **Preprocess GIFs offline** into compact binary format
2. **Embed animations in ROM** for startup/base/blinks
3. **Allow runtime serial injection** for additional animations
4. **Support both legacy and new asset sources** (local gifs/ and external submodule)

**End Goal (Phase 3):** Pico firmware can play embedded animations without filesystem access, with serial protocol for runtime injection.

---

## Features in Phase 3

### F3-1: Analyze Asset Storage Requirements

**Description:**  
Assess ROM budget and determine asset embedding strategy.

**What Gets Done:**
1. Measure current assets:
   - `startup.gif`, `base.gif` (sizes, frame counts)
   - Sample blink animations (sizes)
   - Sample animations from `others/` (sizes)

2. Calculate ROM requirements:
   - Pico RP2040: 2 MB flash, ~500 KB for Pico SDK + firmware
   - Available for assets: ~1.5 MB
   - RP2350A: 4 MB flash, available: ~3.5 MB

3. Prototype asset embedding:
   - Convert `colorful.gif` to C array
   - Measure binary size impact
   - Test that generated header compiles

4. Document three options:
   - **Option A (Recommended):** Embed 3–5 key animations in flash; allow serial injection
   - **Option B:** Use external QSPI flash (requires additional HW)
   - **Option C:** Require SD card (adds HW complexity)

**Acceptance Criteria:**
- [ ] Asset size inventory documented with GIF file sizes and converted header sizes
- [ ] ROM budget analysis completed; minimum flash available quantified for RP2040 and RP2350A targets
- [ ] Prototype `colorful.gif.h` generated and compiles successfully as C header
- [ ] Binary size impact of `colorful.gif` asset measured and confirmed < 250 KB firmware total
- [ ] **[NEW] Recommended storage strategy documented with explicit trade-off analysis (ROM overhead vs. count)**
- [ ] **[NEW] Verification: Generated colorful.gif header frame count matches original GIF frame count within ±1**

**Depends On:** F2-4 (Phase 2 complete)

**Risk Level:** Low

---

### F3-2: Create Asset Embedding Pipeline

**Description:**  
Build tools to convert GIF files to Pico-compatible embedded headers.

**What Gets Done:**
1. Create asset generator script:
   - Input: `.gif` file from `gifs/` or `external/TASBot-eye-animations/`
   - Output: `.h` header with compiled frame data
   - Format: `uint32_t animation_frame_0[] = { ... };`

2. Make format choices:
   - Store GIF frames as RGB888 (3 bytes per pixel) for simplicity
   - Optimize format later if needed
   - Include frame duration metadata

3. Create `pico_sdk/assets/CMakeLists.txt`:
   - Runs generator on GIFs in `gifs/` and submodule
   - Outputs headers to `pico_sdk/build/generated/assets/`
   - Adds generated headers to include path

4. Integrate into main build:
   - Add `add_subdirectory(assets)` to root CMakeLists.txt
   - Link generated asset library to firmware

**Acceptance Criteria:**
- [ ] Asset generator tool created and documented
- [ ] `assets/CMakeLists.txt` automatically generates headers from GIFs
- [ ] Generated headers compile without errors
- [ ] Firmware build succeeds with embedded assets
- [ ] Generated asset data byte-for-byte identical to original GIF frames

**Depends On:** F3-1

**Risk Level:** Medium

---

### F3-3: Replace Runtime Filesystem with Embedded Assets

**Description:**  
Modify animation loading logic to use embedded assets instead of filesystem.

**What Gets Done:**
1. Create `src/platform/asset_loader.h`:
   ```c
   const uint8_t* asset_load(const char* name, int* size);
   const char** asset_list(int* count);
   ```

2. Implement platform-specific versions:
   - `asset_loader_pico.c`: Looks up embedded arrays
   - `asset_loader_rpi.c`: Falls back to filesystem

3. Modify `filesystem.c`:
   - Add `asset_load()` call before directory scanning
   - On Pico: only embedded assets available
   - On RPi: embedded assets + filesystem fallback

4. Update `tasbot.c`:
   - Replace `loadGIFFromPath()` with `asset_load()`
   - Handle missing assets gracefully

**Acceptance Criteria:**
- [ ] `asset_loader.h` compiles on both RPi and Pico
- [ ] Pico firmware loads startup.gif from ROM (verified by LED output)
- [ ] Pico firmware loads base.gif without filesystem access
- [ ] RPi build loads from `./gifs/` if available, falls back to ROM
- [ ] Missing animation asset fails gracefully

**Depends On:** F3-2

**Risk Level:** Medium

---

### F3-4: Implement Serial Animation Injection

**Description:**  
Add serial protocol for runtime animation injection (replacement for UDP on Pico).

**What Gets Done:**
1. Design serial protocol:
   - Format: `Q;animation_name\r\n` (queue animation)
   - Example: `Q;startup\r\n` queues the startup animation
   - Same syntax as UDP for familiarity

2. Create `src/platform/serial_control.h`:
   ```c
   void serial_control_init(void);
   void serial_control_process(void);
   ```

3. Implement `serial_control_pico.c`:
   - Uses Pico SDK's `getchar()` (non-blocking)
   - Buffers input until `\r\n` detected
   - Parses command and injects animation into stack

4. Implement `serial_control_rpi.c`:
   - Stub (RPi uses UDP); kept for parity

**Acceptance Criteria:**
- [ ] Serial protocol defined in `SERIAL_PROTOCOL.md` with format and example commands
- [ ] Pico firmware accepts commands over USB/UART CDC without blocking animation rendering
- [ ] Command `Q;base\r\n` correctly queues the base animation within 50 ms
- [ ] Invalid commands are logged (if serial logging enabled) or silently ignored
- [ ] Command input does not interfere with render tick timing (measured jitter < ±10% of 10 ms tick)
- [ ] Commands work at any time during playback (tested while animation active and during blanking)
- [ ] **[MVP GATE] Pico firmware boots, displays colorful.gif animation, and accepts `Q;base\r\n` to switch to base animation, all without errors**

**Depends On:** F3-3

**Risk Level:** Medium

---

## Cross-Feature Validation

### Checkpoint 3: Asset Path Replacement
**Gate:** Must pass before Phase 4 starts

- Startup, base, and at least one blink animation available via Pico asset mechanism
- Firmware can play animations without relying on host filesystem
- Serial injection works end-to-end

**Test:**
```bash
# Compile firmware with embedded assets
make
# Boot firmware; verify startup animation plays
# Send via serial: Q;base\r\n
# Verify base animation plays next
```

---

## Success Criteria for Phase 3

The phase is **COMPLETE** when:

1. **Asset pipeline is operational:**
   - Offline GIF → C header conversion works
   - Both local `gifs/` and external submodule sources supported
   - Colorful.gif and other test assets embedded successfully

2. **Firmware loads and plays animations:**
   - Startup animation plays on boot
   - Base animation displays correctly
   - Blink animations available via serial injection

3. **User experience is smooth:**
   - Animations play without glitches
   - Serial injection is responsive
   - Asset loading is deterministic

4. **ROM budget is respected:**
   - Binary size < 250 KB (leaves room for growth)
   - Asset count supports MVP (startup, base, 3–5 blinks)

---

## Effort & Timeline

**Estimated Effort:** 1–2 weeks  
- F3-1: 1–2 days (analysis + measurement)
- F3-2: 2–3 days (asset generator tool)
- F3-3: 2–3 days (asset loader + integration)
- F3-4: 1–2 days (serial protocol + handler)

**Timeline:** 1 full sprint (can overlap with Phase 2 after F2-3)

---

## Parallelization Note

Phase 3 can start after F2-3 (event loop is stable) without waiting for F2-4 (timing validation). This allows asset work to proceed in parallel with Phase 2 final validation.

---

## Notes for Team

- Consider future: SD card fallback if ROM budget becomes tight
- Asset format can evolve; RGB888 is stable for MVP
- Serial protocol mirrors UDP for user familiarity
- External submodule allows asset reuse across projects
