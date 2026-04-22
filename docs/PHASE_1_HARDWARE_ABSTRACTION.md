# Phase 1: Abstract Hardware Layer (CRITICAL)

**Status:** Pending Implementation  
**Scope:** Define platform-agnostic LED driver interface; implement for RPi and Pico  
**Target Validation:** LED rendering works on both platforms with identical frame output  
**Effort Estimate:** 1–2 weeks  
**Owner:** Firmware engineer / Roll  
**Date:** 2026-04-15  

---

## Problem Statement

The current `led.c` is tightly coupled to the `ws2811` RPi driver, making it impossible to port to Pico without wholesale refactoring. To maintain portability and reversibility, we must:

1. **Define a platform-agnostic LED interface** (`hw_led.h`)
2. **Isolate platform-specific code** (RPi ws2811 vs. Pico PIO/SPI)
3. **Keep core frame buffer logic portable** (color, indexing, gamma)
4. **Validate identical output** on both platforms

**End Goal (Phase 1):** A `hw_led` abstraction layer that seals the hardware boundary, allowing animation logic to ship to Pico unchanged.

---

## Features in Phase 1

### F1-1: Define Hardware LED Interface

**Description:**  
Create a platform-agnostic LED driver interface (`hw_led.h`) and stub implementations for both RPi and Pico.

**What Gets Done:**
1. Create `src/platform/hw_led.h`:
   ```c
   int hw_led_init(int gpio_pin, int count, int brightness);
   void hw_led_render(const uint8_t* frame_buffer, int count);
   void hw_led_clear(void);
   void hw_led_fini(void);
   int hw_led_is_ready(void);
   ```

2. Create `src/platform/hw_led_rpi.c`:
   - Wrapper over existing `ws2811` code
   - Minimal changes; preserve current behavior

3. Create `src/platform/hw_led_pico.c`:
   - Stub implementation (returns success, logs via serial)
   - No actual LED driver yet

4. Create `src/platform/CMakeLists.txt`:
   - Conditionally link `hw_led_rpi.c` or `hw_led_pico.c`
   - Based on `PICO_SDK_PATH` detection

**Acceptance Criteria:**
- [ ] `hw_led.h` compiles on both RPi and Pico without platform-specific includes
- [ ] `hw_led_rpi.c` implements all functions using existing `ws2811_t` struct
- [ ] `hw_led_pico.c` stub returns success without linking `ws2811`
- [ ] CMake selects correct implementation based on target
- [ ] Existing `led.c` remains unchanged in this feature

**Depends On:** F0-3 (Phase 0 complete)

**Risk Level:** Medium

---

### F1-2: Decouple `led.c` from `ws2811`

**Description:**  
Remove `ws2811` struct and driver code from `led.c`. Keep only frame buffer logic.

**What Gets Done:**
1. Audit `led.c` line by line and separate concerns:
   - Driver initialization → move to `hw_led_rpi.c`
   - Frame buffer management → keep in `led.c`
   - Pixel indexing and gamma → keep in `led.c`

2. Create new `led.c` interface:
   - `void led_init(int count);` — Allocate frame buffer only
   - `void led_set_pixel(int idx, uint32_t color);` — Set pixel color
   - `void led_show(void);` — Call `hw_led_render()` with current buffer
   - `void led_clear(void);` — Call `hw_led_clear()`

3. Update `main.c`:
   - Replace `setup_leds()` with `led_init()` + `hw_led_init()`
   - Replace `ws2811_render()` calls with `led_show()`

**Acceptance Criteria:**
- [ ] `led.c` no longer includes `ws2811.h` or calls `ws2811_*()` functions directly
- [ ] `led.c` compiles independently without `ws2811.h` available
- [ ] Pixel indexing, gamma correction, RGB ordering logic remains in `led.c` and unchanged
- [ ] `hw_led_render()` receives frame buffer in exact same format as `ws2811_render()` previously received
- [ ] RPi build produces identical LED output on test patterns (byte-for-byte comparison with pre-refactor baseline)
- [ ] **[NEW] Root source files `led.c`, `main.c` are modified only (no new files in root)**

**Depends On:** F1-1

**Risk Level:** Medium

---

### F1-3: Implement Pico LED Driver (`hw_led_pico.c`)

**Description:**  
Implement actual Pico SDK LED output using PIO or SPI for WS2812B/SK6812 strips.

**What Gets Done:**
1. Choose LED transport:
   - **Option A:** Pico PIO state machine (recommended)
   - **Option B:** SPI with custom timing (simpler)

2. Implement `hw_led.h` functions in `hw_led_pico.c`:
   - `hw_led_init()`: Initialize Pico PIO/SPI, allocate DMA
   - `hw_led_render()`: DMA frame buffer to LED output
   - `hw_led_clear()`: Fill buffer with 0x000000
   - `hw_led_fini()`: Disable PIO/SPI, free DMA
   - `hw_led_is_ready()`: Return PIO/SPI state

3. Create `src/platform/pico_ws281x.pio` (if using PIO):
   - Bit-bang WS2812B protocol (1.25 µs bit period)

4. Link Pico SDK dependencies:
   - `pico_stdlib`, `hardware_pio`, `hardware_dma`

**Acceptance Criteria:**
- [ ] `hw_led_pico.c` compiles against Pico SDK
- [ ] `hw_led_render()` correctly bit-bangs WS2812B protocol (verified by logic analyzer)
- [ ] Test pattern (solid red → green → blue) lights LEDs correctly
- [ ] Timing tolerance: LED frame rate ≥ 60 Hz (16.7 ms per frame)
- [ ] No POSIX or RPi symbols in final firmware

**Depends On:** F1-1, F1-2

**Risk Level:** High (hardware-specific; requires board testing)

---

## Cross-Feature Validation

### Demo Gate: Hardware Proof-of-Concept (After F1-3)
**Gate:** RECOMMENDED to pause here before Phase 2

**Purpose:** Validate that Pico can actually render to LEDs before proceeding to threading refactor (Phase 2 is high-risk timing work).

**What to Do:**
1. Flash firmware to Plasma 2350 W board
2. Observe colorful.gif frames displaying in sequence
3. Verify LED color accuracy (red, green, blue, etc.)
4. Confirm frame rate is reasonable (~10 fps or better)

**Why Pause Here:**
- Phase 2 (threading refactor) is highest-risk work
- Demo validates hardware integration early
- No surprises in Phase 2; hardware is proven
- Gives team confidence before major refactoring

**Test:**
```c
// Minimal demo main for Phase 1
int main(void) {
    hw_led_init(GPIO_PIN, LED_COUNT, BRIGHTNESS);
    for (int i = 0; i < colorful_frame_count; i++) {
        hw_led_render(colorful_frames[i], LED_COUNT);
        sleep_ms(colorful_frame_duration[i]); // Use Pico SDK sleep
    }
    hw_led_fini();
    return 0;
}
```

**Proceed to Phase 2 After:** Visual inspection confirms colorful.gif displays correctly

---

### Checkpoint 1: Pico Build Identity
**Gate:** Must pass after Phase 1 complete (after demo gate)

- CMake imports Pico SDK correctly
- Main target produces `.elf` and `.uf2` artifacts
- Zero POSIX/ws2811 symbols in Pico binary
- Hardware validation (demo gate) confirms LED rendering works

**Test:**
```bash
cd pico_sdk/build
cmake -DPICO_SDK_PATH=/path/to/sdk ..
make
nm tasbot_eyes.elf | grep -E 'ws2811|pthread'  # Should be empty
```

---

## Success Criteria for Phase 1

The phase is **COMPLETE** when:

1. **Hardware interface is defined and validated:**
   - `hw_led.h` seals the hardware boundary
   - RPi and Pico implementations pass independently

2. **Core logic is portable:**
   - `led.c` compiles without platform-specific headers
   - Animation/color logic remains in portable code

3. **LED output works on both platforms:**
   - RPi: existing behavior preserved byte-for-byte
   - Pico: test pattern displays correctly on hardware

4. **No mixing of platform code:**
   - Zero POSIX/RPi symbols in Pico binary
   - Platform-specific code is isolated and testable

---

## Effort & Timeline

**Estimated Effort:** 1–2 weeks  
- F1-1: 2–3 days (interface definition + stub)
- F1-2: 3–4 days (decoupling led.c)
- F1-3: 5–7 days (Pico LED driver; hardware testing)

**Timeline:** 1 full sprint

---

## Notes for Team

- Phase 1 is highest-risk; LED output is critical path
- Keep RPi build as fallback throughout development
- F1-3 requires access to target hardware (Plasma 2350 W)
- Use logic analyzer to validate LED protocol timing
