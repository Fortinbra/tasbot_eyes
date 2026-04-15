# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Roll joined as Frontend/UI Dev, focused on developer-facing flows and clarity.

## Recent Updates

📌 Team roster finalized on 2026-04-15.

## Learnings

Even firmware work here still needs clear build and usage flows because the project already exposes CLI-oriented behavior.

### 2026-04-15: Feature Documentation Spike

**Key Insights:**
- Original `tasbot_eyes` already has a clean separation between animation timing logic and hardware control (thanks to existing rpi_ws281x abstraction). This makes the Pico port less invasive than typical firmware rewrites.
- The project's animation model is inherently event-driven (blink cycles, random queuing) — translates well to a Pico tick loop without forced threading.
- Asset pipeline is the biggest unknown. GIF decoding at runtime won't work on Pico flash constraints; recommend offline conversion (GIF → frame buffer) or embedding small GIFs.
- Plasma 2350 W electrical spec (protocol, pins, timing) is a critical blocker for Phase 3. Needs Pimoroni docs early.

**Patterns Observed:**
- The portable/firmware split mirrors the existing "abstract hardware output" design — team already thinks this way.
- Build validation (no Linux libs in binary) is a cheap early checkpoint that prevents hidden dependencies.
- Four-phase structure (Foundation → Runtime → Assets → Validation) matches the actual dependency chain, not wishful thinking.

**File Paths to Remember:**
- `src/portable/` — animation, color, palette, stack logic (all copied from existing C)
- `src/firmware/` — Pico SDK glue, main loop, LED driver
- `PICO_MIGRATION.md` — living guide for phases, risks, and acceptance criteria
- `.squad/decisions/` — document Plasma 2350 W protocol choice and asset format decision once made
