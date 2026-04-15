---
name: "pico-sdk-scaffold"
description: "Create an isolated Pico SDK firmware scaffold beside a legacy host build"
domain: "build-systems"
confidence: "high"
source: "earned"
---

## Context

Use this when a repository has an existing desktop or Linux-oriented C/CMake build that must stay intact while Pico firmware work begins in parallel.

## Patterns

### Isolate the firmware root

- Put the Pico entry point in `pico_build\CMakeLists.txt`.
- Keep `pico_sdk_import.cmake`, firmware sources, assets, and transient build output under `pico_build\`.
- Do not rewrite the root build just to start the migration.

### Prefer standard Pico SDK flow

- Use `include(pico_sdk_import.cmake)` followed by `pico_sdk_init()`.
- Start with a minimal executable that links only `pico_stdlib`.
- Turn on USB stdio, turn off UART stdio, and call `pico_add_extra_outputs()` early so UF2 generation is proven.

### Make local setup predictable

- Resolve `PICO_SDK_PATH` from cache or environment first.
- If the team has a common local checkout convention, add one explicit fallback path.
- Default `PICO_BOARD` to a stock board for the target MCU family until custom board config is ready.

### Keep generated lanes explicit

- Preserve `pico_build\assets\generated\` with a nested `.gitignore` that ignores generated payloads but keeps the directory tracked.
- Keep `pico_build\build\` ignored as transient output.

### Validate both stories

- Re-run the legacy host configure/build path to confirm it stayed untouched.
- Configure and build the Pico target far enough to prove `.elf`/`.uf2` outputs exist.

## Examples

- `pico_build\CMakeLists.txt`
- `pico_build\pico_sdk_import.cmake`
- `pico_build\src\firmware\main.c`
- `pico_build\assets\generated\.gitignore`

## Anti-Patterns

- **Root build takeover** — Do not repurpose the legacy root `CMakeLists.txt` as the first Pico milestone.
- **Pico target coupled to legacy libs** — Do not link `gif`, `pthread`, `ws2811`, or filesystem-driven runtime code into the kickoff firmware target.
- **Untracked generated path ambiguity** — Do not leave the future asset output location implicit.
