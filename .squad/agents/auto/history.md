# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Auto joined as Platform Dev for build and SDK integration work.

## Recent Updates

📌 Team roster finalized on 2026-04-15.
📌 **2026-04-15 (Session 2):** Build system audit completed. CMake rewrite recommendations merged to decisions.md.

## Learnings

The migration will likely hinge on separating portable application code from Raspberry Pi specific setup and runtime assumptions.
The current build is a host Visual Studio CMake project that links desktop-era dependencies (`gif`, `ws2811`, `pthread`, `m`) directly, so the Pico SDK conversion needs both a top-level CMake rewrite and a platform seam for LED, timing, filesystem, and UDP features.
The existing `build\` directory is useful only as a baseline signal: it proves the repo is still configured as a desktop executable and immediately fails on missing POSIX and Raspberry Pi Linux headers under Windows, which is consistent with a non-Pico layout.

**Team Alignment (2026-04-15):**
- Build system rewrite decision merged: standard Pico SDK pattern with conditional networking
- Source layout separation (app vs. platform-specific) approved
- Asset handling abstraction strategy captured
- Ready for Phase 1 implementation (hw_led abstraction)
