# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Proto Man joined as Backend Dev for low-level firmware work.

## Recent Updates

📌 Team roster finalized on 2026-04-15.
📌 **2026-04-15 (Session 2):** Runtime seams analysis completed. Portable core and platform shell separation scoped. Decision merged with blockers for team clarification.

## Learnings

This project's core risk is translating Raspberry Pi oriented runtime assumptions into microcontroller-friendly firmware behavior.
- 2026-04-15: The current runtime is split between portable animation logic (frame composition, TASBot index mapping, blink scheduling) and Linux/Raspberry-Pi-specific services (rpi_ws281x, pthread threads, POSIX signals, UDP sockets, filesystem-driven GIF loading).
- 2026-04-15: Biggest Pico blockers are unresolved hardware/service seams: which Pico variant is targeted, how animation assets will be stored and decoded without host filesystem/GIFLIB assumptions, and whether realtime control survives via Wi-Fi, USB, UART, or gets dropped.
- 2026-04-16: `pico_build\src\portable\` can safely start with TASBot geometry plus deterministic smoke-pattern code; `color.c/.h` is the next low-risk legacy copy when HSV fade or gamma correction is needed.
- 2026-04-16: The clean Phase 2 firmware seam is `logical 28x8 frame -> TASBot layout mapper -> 154-pixel RGB888 transport buffer -> hw_led driver`, with the firmware loop owning all cadence.
- 2026-04-16: `led.h`, `gif.h`, `filesystem.h`, and `network.h` are the hard compatibility blockers because they leak `ws2811`, GIFLIB, POSIX directory APIs, sockets, and pthreads across otherwise portable code.

**Team Alignment (2026-04-15):**
- Portable core identified: minimal API reshaping required
- Platform shell wholesale replacement confirmed
- Architectural blockers escalated for team/hardware clarification
- Asset handling confirmed as most visible breaking change for users
