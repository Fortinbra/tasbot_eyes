---
name: "offline-gif-asset-pipeline"
description: "Generate a reproducible embedded animation asset from GIF sources for Pico firmware"
domain: "build-systems"
confidence: "high"
source: "earned"
---

## Context

Use this when a Pico firmware migration must consume animation assets from flash instead of a runtime filesystem, especially when the repo already has both legacy GIFs and a tracked upstream animation source.

## Patterns

### Keep the asset lane inside the Pico project

- Put the converter under `pico_build\tools\`.
- Emit generated headers and metadata under `pico_build\assets\generated\`.
- Wire generation into `pico_build\CMakeLists.txt` with an explicit custom target so the build itself proves the pipeline works.

### Make source selection explicit

- Prefer the maintained submodule asset pool first.
- Fall back to the legacy `gifs\` tree only when the requested asset is missing upstream.
- Fail loudly when neither pool contains the requested asset, and write a metadata sidecar that records the chosen source plus the discovered candidates.

### Separate generated data from the playback contract

- Generate only raw frame arrays and metadata.
- Keep the stable playback struct/API in checked-in source (`embedded_animation.*`) so firmware code can depend on a normal interface instead of including tool-specific logic everywhere.
- Let a tiny wrapper translation unit (`colorful_asset.c`) bind the generated arrays into that contract.

### Reuse the existing transport seam

- Convert GIF frames into the logical frame dimensions the firmware already understands.
- Feed those frames through the current mapper and WS2812B PIO presenter rather than inventing a second rendering path.
- Preserve existing boot/proof banners so migration validation scripts stay useful while playback changes.

## Examples

- `pico_build\tools\generate-gif-asset.ps1`
- `pico_build\assets\generated\colorful_asset.metadata.txt`
- `pico_build\src\portable\embedded_animation.h`
- `pico_build\src\firmware\colorful_asset.c`
- `pico_build\src\firmware\main.c`

## Anti-Patterns

- **Runtime GIF loading on MCU** — do not keep filesystem discovery in the Pico firmware path.
- **Hidden source precedence** — do not silently pick whichever GIF appears first on disk.
- **Generated API sprawl** — do not make every firmware file understand the generator’s raw output format directly.
