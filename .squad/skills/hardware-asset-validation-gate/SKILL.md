---
name: "hardware-asset-validation-gate"
description: "Design acceptance gates for embedded firmware assets (animations, graphics, sound). Separate offline reproducibility from hardware playback proof."
domain: "testing"
confidence: "high"
source: "earned"
extracted_from: "colorful.gif acceptance gate (F3 Pico migration)"
---

## Context

Use this skill when a firmware project embeds offline-generated assets (GIFs → frame arrays, audio files → sample buffers, etc.) and needs to prove:
1. The asset conversion pipeline is deterministic and reproducible
2. The generated binary is correctly linked into the firmware
3. The real hardware plays the asset with stable timing and fidelity

This skill separates concerns: build proof (offline), integration proof (linking), and hardware proof (playback).

## Pattern: Three-Layer Asset Proof

### Layer 1: Build Proof (Offline Reproducibility)

**Purpose:** Lock the asset conversion pipeline so developers trust the binary output.

**Approach:**
- Run the offline converter (GIF → C header, audio → blob, etc.) twice in **separate build directories** on the same machine.
- Compare **exact file checksums** (SHA256, not timestamps).
- If checksums don't match, the converter has hidden non-determinism:
  - Temporary file paths baked in
  - Uninitialized variables
  - Floating-point rounding without seeding
  - Timestamp metadata
  - Tool version mismatch
- **Gate:** Checksums must match exactly before integration proof starts.

**Example (GIF → Frame Array):**
`powershell
# Run 1
./convert-assets.ps1 -AnimationName colorful -OutputDir build1/assets/generated/
 = (Get-FileHash build1/assets/generated/colorful_frames.h -Algorithm SHA256).Hash

# Run 2
./convert-assets.ps1 -AnimationName colorful -OutputDir build2/assets/generated/
 = (Get-FileHash build2/assets/generated/colorful_frames.h -Algorithm SHA256).Hash

if ( -eq ) {
  Write-Host "✓ Reproducible: "
} else {
  Write-Host "✗ Non-deterministic:"
  Write-Host "  Run 1: "
  Write-Host "  Run 2: "
  exit 1
}
`

**Documentation Requirement:**
- Proof doc records the exact command, tool version, and SHA256 hash.
- Example: pico_build/proof/colorful-assets-proof.md contains:
  `
  ## Converter Reproducibility
  
  Tool: pico_build/tools/convert-assets.ps1 version X.Y.Z
  Source: xternal/TASBot-eye-animations/gifs/others/colorful.gif
  
  Build 1: SHA256(colorful_frames.h) = 0xABCD...
  Build 2: SHA256(colorful_frames.h) = 0xABCD...
  
  Result: ✓ Reproducible
  `

### Layer 2: Integration Proof (Linking & Code Hygiene)

**Purpose:** Verify the generated asset is correctly linked into the firmware and does not pollute the target namespace.

**Approach:**
1. **CMakeLists.txt Integration:**
   - Custom command auto-generates the asset header if source changes.
   - Dependency edge is explicit:
     `cmake
     add_custom_command(
       OUTPUT /assets/generated/colorful_frames.h
       COMMAND PowerShell -File /tools/convert-assets.ps1 ...
       DEPENDS /../external/TASBot-eye-animations/gifs/others/colorful.gif
       COMMENT "Generating colorful.gif → C header"
     )
     `
   - Firmware executable depends on the generated file (via 	arget_sources() or include).

2. **Symbol Audit:**
   - After linking, verify the asset is accessible to firmware:
     `ash
     objdump -t tasbot_eyes_pico.elf | grep colorful
     `
   - Expected symbols: colorful_frame_data, colorful_frame_count, colorful_delays[] (or equivalent).
   - Size: Should match rame_count × pixels_per_frame × bytes_per_pixel.

3. **Code Hygiene:**
   - Scan generated files for forbidden headers (platform-specific legacy code):
     `powershell
     grep -r "gif_lib|dirent|unistd|pthread|ws2811" pico_build/assets/generated/
     `
   - Expected: 0 matches.
   - Any match = generated code pulled in POSIX-only or legacy headers = gate fails.

4. **Flash Size Tracking:**
   - Log the size of the generated asset in the proof doc.
   - Warn if > 50 KB (typical flash budget constraint for embedded firmware).
   - Example: "colorful.gif → 28 KB of frame data (acceptable for 252 KB budget)".

5. **Stale Artifact Audit:**
   - If multiple build directories exist, inspect the built ELF/UF2 itself for representative asset constants rather than trusting path names or agent notes.
   - Use little-endian RGB888 constants that only the fixed build should contain (for example `0x00FFAA00`, `0x00AAFF00`, `0x0000FFAA`, `0x0000AAFF`) as fingerprints.
   - If the generated header is multicolor but the flashed artifact lacks those fingerprints, the bug is stale firmware selection, not the mapper or transport-buffer path.

**Documentation Requirement:**
- Proof doc (pico_build/proof/colorful-integration-proof.md) records:
  - Generated file paths and sizes
  - Symbol audit output (objdump)
  - Code hygiene scan (grep) result
  - CMakeLists.txt command (for reviewability)

### Layer 3: Hardware Proof (Playback Fidelity)

**Purpose:** Verify the asset actually plays on the real target hardware with stable timing and no corruption.

**Approach:**

1. **Serial Instrumentation in Firmware:**
    - Playback loop logs per-frame information:
      `c
      [playback] anim=colorful frame=0 delay=100ms checksum=0xABCD1234 ts=0ms
     [playback] anim=colorful frame=1 delay=100ms checksum=0xDEF56789 ts=100ms
     ...
      [playback] cycle_count=1 total_ms=6000 expected_ms=6000 drift_ms=0 PASS
      `
    - Checksum = CRC32 or simple sum of all pixel values in the frame.
    - Timestamp = milliseconds since boot.
    - **USB-CDC caveat:** host-side COM capture may begin only after the port is opened. If the firmware emits one-shot boot banners before the host attaches, flash first, attach immediately when the COM port enumerates, and treat missing early banners as a capture limitation unless you actually observed them.

2. **10+ Cycle Checksum Stability:**
   - Capture serial output for 5–10 minutes (10+ full animation loops).
   - Extract frame checksums from serial log.
   - For each frame index (0, 1, 2, ..., N), verify checksum is identical across all cycles.
   - Variance = frame corruption or memory corruption; gate fails.

3. **Timing Accuracy:**
   - Total cycle time = sum of all per-frame delays (e.g., 60 frames × 100 ms = 6000 ms).
   - Measured cycle time (from serial timestamps):
     `
     measured = frame[N].ts - frame[0].ts + last_frame_delay
     tolerance = ±50 ms (typical hardware jitter)
     
     if |measured - expected| <= tolerance:
       PASS
     else:
       FAIL (timing issue)
     `

4. **Visual Observation:**
   - Record a short video (3–5 seconds) of the LED array during playback.
   - Expected: Smooth animation, recognizable pattern, no flickering or dead pixels.
   - If asset is a color gradient or animation, it should be visibly recognizable.

**Documentation Requirement:**
- Proof doc (pico_build/proof/colorful-playback-proof.md) records:
  - Date and hardware setup (board model, LED array type, serial port)
  - Serial capture log (raw or excerpt showing 10+ cycles)
  - Checksum analysis table (frame index → all cycle checksums)
  - Timing analysis (expected duration, measured duration, drift)
  - Video file reference or description

**Example Proof Template:**
`markdown
## Hardware Playback Proof: colorful.gif

**Setup:**
- Board: Pimoroni Plasma 2350 W
- LED Array: 8×32 WS2812B
- Serial Port: COM3 (115200 baud)
- Test Date: 2026-04-16

**Checksum Stability (10 cycles):**
| Frame | Cycle 1 | Cycle 2 | Cycle 3 | ... | Cycle 10 | Status |
|-------|---------|---------|---------|-----|----------|--------|
| 0     | 0xABCD  | 0xABCD  | 0xABCD  | ... | 0xABCD   | ✓      |
| 1     | 0xDEF5  | 0xDEF5  | 0xDEF5  | ... | 0xDEF5   | ✓      |
| ...   | ...     | ...     | ...     | ... | ...      | ✓      |
| 59    | 0x1122  | 0x1122  | 0x1122  | ... | 0x1122   | ✓      |

**Timing Analysis:**
- Expected duration: 6000 ms (60 frames × 100 ms)
- Measured duration (cycles 1–10): 6000 ± 30 ms
- Tolerance: ±50 ms
- Status: ✓ PASS

**Visual Observation:**
- ✓ Smooth animation, recognizable pattern
- ✓ No flickering or dead pixels
- ✓ Colors stable across cycles

**Conclusion:** colorful.gif playback PASSED all hardware acceptance criteria.
`

## Checklist Pattern

Create a 14-point reviewer checklist:

| # | Layer | Criterion | Evidence | ☐ |
|---|-------|-----------|----------|---|
| 1–5 | Build | Source selected, frame count documented, converter reproducible, size acceptable, code hygiene | pico_build/proof/colorful-assets-proof.md | ☐ |
| 6–9 | Integration | CMakeLists.txt updated, symbols present, code hygiene (no forbidden includes), size logged | pico_build/proof/colorful-integration-proof.md | ☐ |
| 10–14 | Hardware | UF2 flashes, playback executes, 10+ cycle checksums stable, timing accurate, visual proof | pico_build/proof/colorful-playback-proof.md + video | ☐ |

Collapse any layer, gate does not pass.

## Risks & Mitigations

| Risk | Layer | Mitigation |
|------|-------|-----------|
| Non-deterministic converter | 1 | Seed RNG, avoid temp paths, pin tool versions |
| Hidden platform dependency in generated code | 2 | Automated grep scan for forbidden headers before merge |
| Frame corruption on real hardware | 3 | Per-frame checksum logging + 10-cycle validation |
| Flash size blowup | 1 | Pre-scan asset size before conversion; warn early |
| Timing drift | 3 | Per-frame timestamp logging (not cycle-level summary) |

## Examples

- **GIF → Frame Array (Pico firmware):** Converter produces C header with frame data + delay table; firmware loops playback and logs checksums.
- **Stale UF2 triage (Pico firmware):** `build\ws2812-proof\tasbot_eyes_pico.elf` containing orange/green/cyan RGB888 constants while `build\review-colorful-proof\tasbot_eyes_pico.elf` lacks them is decisive evidence that the wrong UF2 can preserve a blue-only symptom after the generator was fixed.
- **Audio → Sample Blob (RTOS):** Converter encodes WAV as binary blob; firmware DMA-streams to DAC and logs frame checksums.
- **Font → Bitmap Table (LCD):** Converter renders glyphs to bitmap; firmware renders text and validates glyph checksums.

## Anti-Patterns

- **Skipping Layer 1:** "The converter is simple, reproducibility doesn't matter." — Non-determinism always hides until it doesn't; require proof.
- **Skipping Layer 2:** "The asset is embedded; we'll test it on hardware." — Linking errors and code hygiene issues are cheapest to catch early.
- **Trusting the build directory name:** assuming any artifact called `tasbot_eyes_pico.uf2` contains the latest generated asset payload.
- **Skipping Layer 3:** "The hardware test is optional if the build proof passes." — Real hardware exposes timing, memory, and electrical issues offline tests miss.
- **Collapsing Proof Artifacts:** Presenting converter output as equivalent to hardware playback proof.

