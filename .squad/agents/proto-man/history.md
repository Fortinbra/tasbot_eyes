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

**Team Alignment (2026-04-15):**
- Portable core identified: minimal API reshaping required
- Platform shell wholesale replacement confirmed
- Architectural blockers escalated for team/hardware clarification
- Asset handling confirmed as most visible breaking change for users
