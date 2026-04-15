# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Mega Man joined as Tester and reviewer for migration work.

## Recent Updates

📌 Team roster finalized on 2026-04-15.
📌 **2026-04-15 (Session 2):** Baseline confirmed; regression gates defined and approved. Six checkpoints, five riskiest regressions documented.

## Learnings

Migration success needs clear behavior checkpoints because the runtime platform is changing drastically.
- Current baseline is a host-side Visual Studio CMake build with no CTest coverage; in this environment the build stops immediately on missing POSIX/RPi headers (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`, `netinet/in.h`).
- The highest-risk migration seams are not cosmetic build edits: they are the LED driver contract, runtime asset loading from `./gifs`, POSIX threading/timing, and UDP control paths that may not exist on the Pico target.
- Migration gates need proof at each seam: firmware target generation, preserved animation asset access, verified LED index mapping, timing stability, and an explicit decision on whether network-driven features survive on Pico or are replaced/stubbed.

**Team Alignment (2026-04-15):**
- Six regression checkpoints approved as merge gates (not aspirations)
- Five riskiest regressions identified for monitoring
- Tester mandate clear: firmware artifacts + core behavior + hardware proof required
- Feature-scope and asset-source decisions escalated to team
