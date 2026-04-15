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
