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

### 2026-04-16 (Session 12): Runtime Seam Slice Approved for Phase 2

- Mega Man approved Proto Man's runtime seam design as Phase 2 starting point.
- Isolation rule held: `pico_build\` stays clean of POSIX/RPi headers; seam boundary concrete and measurable.
- **Approved Seam:** `logical 28x8 frame → TASBot layout mapper → 154 RGB888 transport buffer → firmware hw_led sink`
- **Portable layer:** `pico_build\src\portable\tasbot_layout.c`, `smoke_patterns.c` — deterministic frame generation and geometry mapping
- **Firmware layer:** `pico_build\src\firmware\main.c`, `hw_led_stub.c`, `board.h` — startup, timing cadence, transport boundary
- Next step: Fix Pico host-tool build path to emit `.elf`/`.uf2`; capture USB-serial ready banner plus full smoke-pattern cycle from hardware.
- WS2812B-over-PIO implementation approved for Phase 2 work once hardware board wiring is confirmed.

### 2026-04-16 (Session 13): WS2812B PIO Transport Landed

- Replaced the Pico-side stub sink with `pico_build\src\firmware\hw_led_pio.c`, using a standard WS2812 PIO program and RGB888→GRB packing at the hardware seam.
- `pico_build\src\firmware\board.h` now resolves the LED data pin from `PICO_DEFAULT_WS2812_PIN`, then `PLASMA2350_DATA_PIN`, then a hard fallback to GPIO 15 so the Plasma board path stays explicit.
- `pico_build\CMakeLists.txt` now prefers `pimoroni_plasma2350` when the SDK has that board header, otherwise falls back to `pico2`.
- Clean Pico proof build succeeded at `pico_build\build\ws2812-proof`, producing `.elf`, `.bin`, `.hex`, `.uf2`, and embedding the updated boot/ready/runtime seam banners.
- Root baseline still fails in the expected legacy ways (`ws2811/ws2811.h`, `netinet/in.h`, and `dirent.d_type`/`DT_REG`), which confirms the new work stayed isolated under `pico_build\`.
- Checked in `pico_build\src\firmware\ws2812.pio.h` beside the source `.pio` file because building SDK `pioasm` from source in this Windows environment was less reliable than using the prebuilt tool once and committing the generated header.


📌 **2026-04-16 (Session 13 Complete):** WS2812B PIO firmware implementation delivered.
- Produced: ws2812.pio + generated header, hw_led_pio.c driver, reproducible UF2 proof
- Hardware path: GPIO from board.h with fallback to GPIO 15 (Plasma alignment)
- Seam validated: 154-entry RGB888 transport → hw_led → GRB repack → GPIO 800kHz
- Decision: Committed generated .pio.h to avoid pioasm rebuild friction on Windows
- Status: COMPLETE, handed to Mega Man for review gate (build proof, serial capture, physical hardware validation)
