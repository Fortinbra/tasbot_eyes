# Phase 2: Validate Animation Timing & Refactor Threading (HIGH RISK)

**Status:** Pending Implementation  
**Scope:** Replace POSIX threading with Pico event loop; validate animation timing matches RPi baseline  
**Target Validation:** Animation playback timing within ±5% of RPi baseline  
**Effort Estimate:** 1–2 weeks  
**Owner:** Firmware engineer  
**Date:** 2026-04-15  

---

## Problem Statement

After Phase 1 (hardware validated via demo gate), Pico SDK does not support POSIX threading. The current animation system uses `pthread` for:
- Render ticks (10 ms interval)
- Hue updates (rainbow mode)
- Animation state transitions

We must replace these with Pico-compatible timer callbacks while preserving timing accuracy and animation quality.

**Why Phase 2 is High-Risk:** Animation timing perception is critical. Visible glitches, jitter, or early/late transitions are regressions that degrade user experience. This phase requires careful validation.

**End Goal (Phase 2):** Event-loop animation system on Pico with timing within ±5% of RPi baseline; colorful.gif plays smoothly with correct frame durations and transitions.

---

## Features in Phase 2

### F2-1: Audit and Document Current Thread Usage

**Description:**  
Identify all `pthread` calls and timing assumptions. Create a mapping of current thread-driven behavior to Pico event-loop equivalents.

**What Gets Done:**
1. Scan codebase for:
   - `pthread_create()`, `pthread_join()`, `pthread_kill()` calls
   - `usleep()`, `sleep()` calls
   - Signal handlers (`SIGINT`, `SIGTERM`)
   - Shared state (mutexes, semaphores)

2. Document findings:
   - `renderThread` (led.c): 10 ms ticker, calls `ws2811_render()`
   - `hueThread` (tasbot.c): Rainbow mode, increments hue every 10 ms
   - `serverInject` (network.c): Blocking listen on port 8080
   - `serverRealtime` (network.c): Blocking listen on port 19446
   - Main loop (main.c): `usleep()` between animation state transitions

3. Create `THREADING.md`:
   - ASCII diagram of current threads
   - Latency budget per thread
   - Shared state and synchronization needs

**Acceptance Criteria:**
- [ ] All `pthread_*`, `usleep()`, `sleep()`, and `sigaction()` calls listed with line numbers
- [ ] Each call classified as "render-critical", "animation-state", or "network-blocking"
- [ ] `THREADING.md` provides clear refactoring roadmap for Pico

**Depends On:** F1-3 (Phase 1 complete)

**Risk Level:** Low

---

### F2-2: Implement Pico Event Loop Timer Infrastructure

**Description:**  
Create Pico-compatible timing primitives to replace `pthread` and `usleep()`.

**What Gets Done:**
1. Create `src/platform/pico_timer.h`:
   ```c
   void pico_timer_init(void);
   void pico_timer_every_ms(int ms, void (*callback)(void));
   void pico_timer_once_ms(int ms, void (*callback)(void));
   void pico_timer_cancel(timer_handle_t h);
   void pico_timer_sleep_until(absolute_time_t t);
   ```

2. Implement using Pico SDK's `alarm_pool` API:
   - Render tick: 10 ms
   - Hue tick: 10 ms
   - Animation state transitions: variable

3. Create `src/platform/network_pico.c`:
   - Stub implementations of network servers
   - Functions defined but empty on Pico

**Acceptance Criteria:**
- [ ] `pico_timer.h` compiles without `pthread.h`
- [ ] At least two independent callbacks run at different intervals
- [ ] Timer callbacks execute within ±5% of requested interval
- [ ] Network stubs compile on Pico without linking `pthread` or socket libraries

**Depends On:** F1-3

**Risk Level:** Medium

---

### F2-3: Refactor `main.c` for Event-Loop Model

**Description:**  
Replace `pthread_create()` calls with timer registrations. Adapt main loop to Pico's event-driven execution.

**What Gets Done:**
1. Create new `main_pico.c`:
   - Initialize Pico platform (stdio, timers, LED driver)
   - Register render tick callback (10 ms)
   - Register hue update callback (10 ms)
   - Register animation state machine callback (variable intervals)
   - Enter idle loop (yield or WFI instruction)

2. Keep `main_rpi.c` as-is (current `main.c`):
   - Preserves thread-based behavior for RPi/Linux
   - Can be deprecated after Pico is stable

3. Update CMakeLists.txt:
   - Conditionally compile `main_pico.c` or `main_rpi.c`
   - Link Pico timer APIs for Pico target

**Acceptance Criteria:**
- [ ] `main_pico.c` compiles without `pthread.h`, `signal.h`, or `unistd.h`
- [ ] Firmware boots and enters event loop without errors
- [ ] Render callback executes at 10 ms intervals (verified by LED heartbeat)
- [ ] Animation state transitions fire at expected intervals
- [ ] No memory leaks or timer saturation (test for 1+ hour)

**Depends On:** F2-2

**Risk Level:** High (animation timing perception)

---

### F2-4: Validate Timing and Blink Behavior

**Description:**  
Confirm animation playback, blink timing, and hue updates match RPi baseline within acceptable tolerance.

**What Gets Done:**
1. Create deterministic test animation:
   - 1 second solid red
   - 1 second solid green
   - 1 second solid blue
   - 10 second sweep hue change (rainbow)

2. Measure on both RPi and Pico:
   - Total animation duration
   - Frame presentation jitter (stddev of frame intervals)
   - Hue transition smoothness

3. Document acceptable tolerance:
   - Duration: ±5% of expected
   - Jitter: <10% of frame interval
   - Hue: no visible discontinuities

**Acceptance Criteria:**
- [ ] Pico animation duration measured over 30-second test run is within ±50 ms of RPi baseline (5% tolerance)
- [ ] Hue updates appear smooth on both platforms with no visible frame-duration discontinuities
- [ ] Blink timing follows programmed intervals with ±100 ms deviation (max) from expected fire time
- [ ] No early/late playback transitions; animations queue and dequeue predictably
- [ ] **[NEW] Test harness created: Deterministic animation duration measured with host-side timer, results logged to CSV**
- [ ] **[NEW] Extended stability test: Pico firmware runs for 1 hour without timing drift accumulation or frame drops (verified by frame counter)**

**Depends On:** F2-3

**Risk Level:** Medium (measurement/validation)

---

## Cross-Feature Validation

### Checkpoint 2: Platform Seam Isolation
**Gate:** Must pass before Phase 3 starts

- Core animation/render logic compiles without POSIX/network/RPi driver headers
- Pico-specific code owns all timing I/O
- Unsupported features are compile-gated

---

## Success Criteria for Phase 2

The phase is **COMPLETE** when:

1. **Threading model is replaced:**
   - Pico firmware uses event loop instead of pthreads
   - Timer callbacks drive render, hue, and animation ticks

2. **Timing is validated:**
   - Animation playback within ±5% of RPi
   - No visible glitches or timing drift
   - Extended runs (1+ hours) stable

3. **Code is portable:**
   - Core animation logic unchanged
   - Platform-specific timing is isolated in timer layer

---

## Effort & Timeline

**Estimated Effort:** 1–2 weeks  
- F2-1: 1 day (analysis + documentation)
- F2-2: 2–3 days (timer infrastructure)
- F2-3: 3–4 days (main loop refactor)
- F2-4: 2–3 days (timing validation + testing)

**Timeline:** 1 full sprint

---

## Notes for Team

- Phase 2 is high-risk because timing changes are perceptible
- Use serial logging to verify callback execution order
- Compare frame-by-frame output with RPi baseline if possible
- Blink quality is key to user experience; prioritize accuracy
