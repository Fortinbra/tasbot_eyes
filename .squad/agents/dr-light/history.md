# Project Context

- **Project:** tasbot_eyes
- **Owner:** Fortinbra
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and is being migrated from Raspberry Pi oriented code to Pico SDK firmware.
- **Created:** 2026-04-15

## Core Context

Dr. Light leads the Pico SDK migration architecture.

### Historical Summary (Sessions 2–6)
- Established the four-phase migration strategy around the three critical seams: LED driver, threading/timing, and assets/filesystem.
- Broke the work into executable features with checkpoints, then inserted **Phase 0** after Fortinbra clarified hard constraints: root sources stay read-only, Pico work lives in an isolated subdirectory, offline GIF preprocessing is mandatory, and `colorful.gif` is the first validation target.
- Validated Pimoroni Plasma 2350 W assumptions: RP2350A, ample flash/RAM headroom, standard WS2812B or APA102-style LED protocols, optional wireless out of scope for v1.
- Approved the documentation/submodule architecture: migration planning under `docs\migration\`, maintained animation sources in `external\TASBot-eye-animations\`, graceful fallback to local `gifs\`, and generated firmware assets under `pico_build\assets\generated\`.
- Locked the sequencing principle that Phase 0 precedes hardware work, hardware proof precedes timing refactors, and later asset/cleanup work can parallelize only after runtime stability is proven.

## Recent Updates

- 📌 Team roster finalized on 2026-04-15.
- 📌 **2026-04-15 (Session 2):** Four-phase migration strategy captured and merged to decisions.md. Team consensus on phased abstraction, conditional CMake, embedded ROM assets, and serial injection.

## Learnings

- Migration success depends more on seam control than on superficial build progress.
- The architecture should preserve the legacy host build, isolate Pico work, and make every phase exit criterion observable.
- `colorful.gif` on real hardware is the earliest meaningful proof point; smooth playback and fuller feature parity come later.

