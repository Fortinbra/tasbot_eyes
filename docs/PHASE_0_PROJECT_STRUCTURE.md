# Phase 0: Project Structure & SDK Setup

**Status:** Pending Implementation  
**Scope:** Establish isolated Pico SDK project, configure build system, preprocess first asset  
**Target Validation:** Pico firmware builds and basic asset header generated  
**Effort Estimate:** 2–4 hours  
**Owner:** Firmware engineer  
**Date:** 2026-04-15 (Session 5)  

---

## Problem Statement

The original `tasbot_eyes` repository is a Raspberry Pi desktop application with Linux dependencies. To migrate to Pico SDK firmware while keeping original sources immutable, we need:

1. **Isolated subdirectory** for Pico SDK project (no pollution of root sources)
2. **Standard Pico SDK toolchain** configured and building
3. **Asset preprocessing pipeline** (offline GIF → binary header conversion)
4. **Clear boundary** between portable application code and firmware-specific glue

**End Goal (Phase 0):** A standalone Pico SDK CMake project that builds and integrates one preconverted asset (colorful.gif).

---

## Features in Phase 0

### F0-1: Create Pico SDK Subdirectory & CMake Structure

**Description:**  
Establish the `pico_sdk/` directory with standard Pico SDK layout and CMakeLists.txt that imports the Pico SDK without modifying root sources.

**What Gets Done:**
1. Create `pico_sdk/` directory with structure:
   ```
   pico_sdk/
   ├── CMakeLists.txt         (Pico root CMake)
   ├── pico_sdk_import.cmake  (Pico SDK importer)
   ├── src/
   │   ├── main.c            (Pico entry point stub)
   │   └── board.h           (GPIO configuration)
   ├── assets/
   │   └── CMakeLists.txt    (Asset generation rules)
   └── build/                (CMake output, .gitignore'd)
   ```

2. Create `pico_sdk/CMakeLists.txt`:
   - Import Pico SDK via `pico_sdk_import.cmake`
   - Detect `PICO_SDK_PATH` environment variable
   - Create firmware target (`.elf` and `.uf2` artifacts)
   - Link only Pico SDK libraries (zero RPi/POSIX dependencies)

3. Create `pico_sdk/pico_sdk_import.cmake`:
   - Standard Pico SDK import pattern (copies from official examples)

4. Create `pico_sdk/src/board.h`:
   - GPIO pin definitions for Plasma 2350 W
   - LED protocol selection (WS2812B or APA102)
   - Placeholder comments for later implementation

**Acceptance Criteria:**
- [ ] `pico_sdk/` directory exists with all required files
- [ ] `cmake -DPICO_SDK_PATH=/path/to/sdk -B pico_sdk/build` succeeds
- [ ] No modifications to root-level source files (verify with `git diff`)
- [ ] Firmware target produces `.elf` and `.uf2` artifacts in `pico_sdk/build/`
- [ ] Root CMakeLists.txt remains unchanged (RPi build unaffected)
- [ ] **[NEW] Firmware boots and prints "Pico Firmware Ready" or equivalent to USB serial within 3 seconds**

**Depends On:** None (first feature)

**Risk Level:** Low (structure only, no code changes)

**Implementation Notes:**
- Do NOT yet integrate portable code; this feature is structure only
- Follow official Pico SDK CMake examples for consistency
- Use absolute paths for PICO_SDK_PATH to avoid portability issues

---

### F0-2: Preprocess Startup Asset (colorful.gif)

**Description:**  
Create the asset preprocessing pipeline and convert colorful.gif to a C header file.

**What Gets Done:**
1. Create asset generator tool (`assetgen` or Python script):
   - Input: GIF file from `gifs/colorful.gif`
   - Output: `pico_sdk/build/generated/colorful.gif.h`
   - Format: `uint32_t colorful_frames[frame_count][256]` (RGB888 per pixel for 8×32 layout)

2. Create `pico_sdk/assets/CMakeLists.txt`:
   - Detects GIF files in `gifs/`
   - Runs generator on each GIF
   - Outputs headers to `pico_sdk/build/generated/assets/`
   - Adds to include path for firmware build

3. Implement asset generator logic:
   - Use GIFLIB to decode frames
   - Convert frame pixels to 8×32 LED layout (matching TASBot mapping)
   - Output C arrays with clear naming (e.g., `colorful_frame_0`, `colorful_frame_1`, ...)
   - Include metadata (frame count, frame duration from GIF)

4. Integrate into main Pico build:
   - Add `add_subdirectory(assets)` to `pico_sdk/CMakeLists.txt`
   - Generated headers available to firmware code

**Acceptance Criteria:**
- [ ] Asset generator tool compiles and runs without errors
- [ ] `colorful.gif` → `colorful.gif.h` conversion succeeds
- [ ] Generated header defines `colorful_frames[][]` array
- [ ] Header compiles as part of Pico firmware build
- [ ] Frame count and timing metadata are accessible to code
- [ ] No modification to root `gifs/colorful.gif` file

**Depends On:** F0-1

**Risk Level:** Low (asset format is internal; can change later)

**Implementation Notes:**
- Store raw RGB888 (3 bytes per pixel) for simplicity; optimize format later if needed
- Document the 8×32 pixel layout mapping in code comments
- Consider future: handle both internal `gifs/` AND external submodule `external/TASBot-eye-animations/`

---

### F0-3: Pico CMake Build Integration & Validation

**Description:**  
Integrate portable code into Pico build and validate that firmware can be built end-to-end.

**What Gets Done:**
1. Copy portable sources to Pico build:
   - Copy `color.c/h`, `stack.c/h`, `utils.c/h`, `tasbot.c/h`, `gif.c/h` to `pico_sdk/src/portable/`
   - Do NOT modify originals; these are copies
   - Verify with `git diff` that root files are untouched

2. Create `pico_sdk/src/main.c`:
   - Minimal entry point that:
     - Initializes Pico stdio (USB or UART serial)
     - Prints "Pico Firmware Ready"
     - Does NOT yet drive LEDs (stub only)
   - Later phases will add animation/LED logic

3. Update `pico_sdk/CMakeLists.txt`:
   - Add `src/portable/` to include path
   - Link portable sources into firmware target
   - Ensure no POSIX/RPi headers are included

4. Create `pico_sdk/build.sh` (optional convenience script):
   ```bash
   #!/bin/bash
   export PICO_SDK_PATH=${1:-$HOME/pico-sdk}
   mkdir -p build && cd build
   cmake -DPICO_SDK_PATH=$PICO_SDK_PATH ..
   make -j4
   ```

5. Validate build:
   - `mkdir pico_sdk/build && cd pico_sdk/build`
   - `cmake -DPICO_SDK_PATH=/path/to/sdk ..`
   - `make`
   - Confirm `.elf` and `.uf2` are produced
   - Confirm zero linking errors

**Acceptance Criteria:**
- [ ] Pico firmware builds successfully without errors or warnings
- [ ] Final binary is < 250 KB (headroom for future features)
- [ ] Root source files verified unchanged (git diff shows no modifications)
- [ ] `.elf` artifact contains only Pico SDK symbols (no POSIX/RPi symbols)
- [ ] Build artifacts are in `pico_sdk/build/` only (root remains clean)
- [ ] Firmware can be flashed to Pico/Plasma board (basic bootloader test)
- [ ] **[NEW] Symbol audit passes: `nm pico_sdk/build/tasbot_eyes.elf | grep -E 'ws2811|pthread|getgrent|getserv' returns no results**

**Depends On:** F0-2

**Risk Level:** Low (build validation only)

**Implementation Notes:**
- Use `nm` or `readelf` to audit symbol table for unexpected POSIX libraries
- Keep a baseline build artifact for comparison in future phases
- Document any toolchain setup requirements in README

---

## Cross-Feature Dependencies

```
F0-1 (Create Pico SDK subdirectory)
  └─→ F0-2 (Preprocess colorful.gif)
      └─→ F0-3 (Pico CMake integration & validation)
          └─→ F1-1 (Define Hardware LED Interface) [Phase 1 begins]
```

---

## Success Criteria for Phase 0

The phase is **COMPLETE** when:

1. **Project structure is clean and isolated:**
   - `pico_sdk/` subdirectory contains Pico-specific code
   - Root files remain untouched
   - Two independent build systems (RPi at root, Pico in subdirectory)

2. **Asset pipeline is operational:**
   - Colorful.gif successfully converts to C header
   - Header embeds in firmware binary
   - Frame data is accessible to firmware code

3. **Firmware builds successfully:**
   - `cmake -DPICO_SDK_PATH=... && make` in `pico_sdk/` succeeds
   - Output is valid `.elf` and `.uf2`
   - Binary size is reasonable (< 250 KB at this phase)

4. **No platform mixing:**
   - Zero POSIX or RPi-specific symbols in final firmware
   - Linker produces no warnings about missing libraries

---

## Effort & Timeline

**Estimated Effort:** 2–4 hours  
- F0-1: 1 hour (structure + CMake boilerplate)
- F0-2: 1 hour (asset generator tool)
- F0-3: 1–2 hours (integration + validation)

**Timeline:** 1 half-day sprint

---

## Notes for Team

- Phase 0 is foundational; correctness here prevents later rework
- Any issues in this phase should block Phase 1 until resolved
- **After Phase 0 is complete, Phase 1 (Hardware Abstraction) begins immediately**
- **Important:** Phase 1 will include a Demo Gate after F1-3 where we validate colorful.gif rendering on actual hardware before proceeding to threading refactor (Phase 2)
