---
name: "hardware-proof-gates"
description: "Define and validate measurable hardware behavior gates separate from build-artifact proof"
domain: "testing"
confidence: "high"
source: "earned"
tools: []
---

## Context

When a firmware migration touches hardware transport (LED drivers, serial output, timing-critical peripherals), split proof into two lanes:
- **Build proof:** Artifacts compile, link correctly, no forbidden headers, reproducible across clean builds
- **Hardware proof:** Physical devices exhibit expected behavior on real hardware or in a captured simulation

Build proof and hardware proof are separate gates. Do not conflate them. A stub that compiles is not evidence that the real driver works.

## Patterns

### Define Hardware Contracts Explicitly in Source

- GPIO pin assignment must be in a single, editable header (board.h or equivalent)
- Protocol choice must be explicit: `#define PROTOCOL WS2812B` (not a comment or a guess)
- Timing requirements must be documented: e.g., "WS2812B ~800 kHz, achievable on 150 MHz Pico"
- Add guard clauses: `#ifndef PROTOCOL #error "Protocol not selected"`

### Split Visible Behavior into Observable Phases

- Break hardware behavior into discrete, testable phases (e.g., red LED → green LED → blue LED → white LED)
- Each phase should be visually distinct and reproducible
- Use checksums or simple counters to prove determinism (frame counter, pixel-checksum logging)
- Timing should be loose enough to test on bench (750ms phase is easier than 16ms), then tighten later

### Require Reproducible Evidence for Hardware Gates

- **Build proof:** Fresh rebuild, symbol audit, forbidden-header grep, artifact hashes
- **Serial proof:** Captured UART/USB log showing boot message, phase progression, expected checksums
- **Visual proof:** Photo or video of physical LEDs matching expected colors and timing
- **Stability proof:** 10+ consecutive cycles with identical checksums (detects non-deterministic rendering)

### Document Fallback Paths

- If real hardware driver is unstable, revert to stub to unblock software integration
- Provide one-line command to rebuild with stub vs. real driver
- Record which gate is open vs. closed in the decision log, not in code comments

### Separate "Smoke Test" Proof from "Animation Playback" Proof

Smoke tests prove the transport layer works (colors light, timing is stable). They do NOT prove animation playback, asset loading, or color dynamics. Document this boundary clearly.

| Proof Type | Example | When Ready |
|---|---|---|
| Smoke test | Four-phase color pattern at fixed intervals | Phase 2 (transport validation) |
| Asset playback | GIF file decoding + queue on device | Phase 3 (asset integration) |
| Color dynamics | Hue fades, brightness modulation | Phase 4 (animation polish) |

## Examples

### Example: WS2812B PIO Proof Gate (tasbot_eyes)

From board.h:
```c
#define TASBOT_WS2812B_DATA_PIN 0
#define TASBOT_WS2812B_PIXEL_COUNT 154
#define TASBOT_FRAME_RATE_HZ 1
#define TASBOT_LED_PROTOCOL PROTOCOL_WS2812B

#ifndef TASBOT_LED_PROTOCOL
#error "LED protocol not selected; define TASBOT_LED_PROTOCOL in board.h"
#endif
```

Build proof checklist:
- tasbot_eyes_pico.elf exists from two clean builds (byte-identical hashes)
- nm build/tasbot_eyes_pico.elf | grep pio returns PIO-related symbols
- No grep hits for "ws2811" or "pthread" in pico_build/src/

Hardware proof checklist:
- Serial log over 30 seconds shows:
  - Boot message within 3 seconds
  - Four-phase log entries (phase=0,1,2,3) repeating every 750ms
  - Identical checksum for each phase across 10+ cycles
- Physical observation:
  - Phase 0: Left eye RED (8 pixels)
  - Phase 1: Right eye GREEN (8 pixels)
  - Phase 2: Nose BLUE (20-30 pixels)
  - Phase 3: Alignment WHITE (6 pixels)
- Timing within +/-50ms of 750ms baseline

### Example: USB Serial Ready Gate (hypothetical)

If firmware must prove USB serial is working before animation code runs:

Build proof: pico_enable_stdio_usb(target 1) in CMakeLists.txt; symbol audit finds usb_out_* functions.

Hardware proof: Captured serial log shows boot banner within 3 seconds of power-on or button press. No manual REPL or interaction needed.

## Anti-Patterns

- **Conflating build and hardware proof:** Saying "the firmware compiles, so the LEDs must work" without physical evidence
- **Vague behavioral requirements:** "Colors should light" without specifying which colors, which regions, which timing
- **Ignoring checksum drift:** Saying "it looked pretty consistent" instead of measuring actual cycle-to-cycle variance
- **Stub acceptance as final proof:** Treating a stub implementation as equivalent to the real driver
- **Skipping fallback documentation:** Building a feature that cannot revert to stub if the real driver fails
- **Mixing timing concerns:** Validating 60 Hz smoothness in a 1 Hz smoke test (different gates, different criteria)