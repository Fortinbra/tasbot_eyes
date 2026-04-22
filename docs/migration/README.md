# Pico SDK Migration Plan

**Status:** Planning  
**Target:** Pimoroni Plasma 2350 W  
**Build Model:** Isolated Pico SDK project in `pico_build\`  
**Asset Model:** Offline preprocessing only; no runtime filesystem on device

## Intent

Migrate `tasbot_eyes` from the existing desktop or Raspberry Pi oriented codebase into a Pico SDK firmware project without modifying the original root source files.

## Source-of-Truth Layout

| Area | Path | Role |
|---|---|---|
| Original application code | root `*.c`, `*.h`, root `CMakeLists.txt` | Immutable reference implementation |
| Legacy animation dump | `gifs\` | Local, ignored reference set for later comparison |
| Tracked comparison asset set | `external\TASBot-eye-animations\` | Git submodule with maintained GIF and Aseprite sources |
| Pico firmware project | `pico_build\` | Future isolated SDK tree and build output |
| Generated embedded assets | `pico_build\assets\generated\` | Offline pipeline output compiled into firmware |
| Migration feature docs | `docs\migration\` | Planning, sequencing, and acceptance criteria |

## Architecture Direction

### 1. Preserve the original repo inputs

- Do not edit root application sources in place.
- Treat the root `gifs\` tree as a frozen legacy input set.
- Keep new platform glue, build logic, and generated assets outside the legacy runtime layout.

### 2. Keep Pico build glue isolated

The Pico port should live under `pico_build\` with its own CMake entry point, source tree, assets folder, and transient build directory. The happy path stays:

1. Prepare or refresh assets offline.
2. Build firmware from `pico_build\build\`.
3. Flash the resulting UF2.

### 3. Standardize asset intake before embedding

The project now has two animation source pools to compare:

- `gifs\` for the legacy in-repo GIF set
- `external\TASBot-eye-animations\gifs\` for the tracked submodule set

The offline pipeline should compare filenames, frame timing, and visual coverage before choosing which source becomes the generated firmware asset. That comparison is a planned part of Phase 3, not an ad hoc manual step.

For planning purposes, the default source-selection rule is:

1. Prefer `external\TASBot-eye-animations\gifs\` when the submodule is present and the requested animation exists there.
2. Fall back to the legacy `gifs\` tree when the submodule is absent or the specific animation is missing there.
3. Fail generation loudly when neither source contains the requested animation.

### 4. Compile assets into the UF2

The Pico target must not depend on filesystem discovery. The intended flow is:

1. Compare legacy and submodule assets.
2. Normalize selected source files.
3. Convert them into generated headers or binary blobs under `pico_build\assets\generated\`.
4. Link those generated assets into the firmware image.

## Phase Map

| Phase | Focus | Primary Doc |
|---|---|---|
| F1 | Foundation, isolated Pico scaffolding, and source policy | `docs\migration\features\f1-foundation.md` |
| F2 | Portable runtime shell, hardware seam, and playback smoke test | `docs\migration\features\f2-core-runtime.md` |
| F3 | Offline asset pipeline, source comparison, and `colorful.gif` demo | `docs\migration\features\f3-assets-playback.md` |
| F4 | Broader validation, extra animations, and deployment readiness | `docs\migration\features\f4-validation.md` |

## Fastest Honest Demo Path

The first hardware demo should be optimized around one artifact only: `colorful.gif`.

1. F1 proves the isolated Pico project exists and boots without touching legacy sources.
2. F2 proves the firmware shell and LED path can render deterministic frames on the real 8x32 array.
3. F3 converts and embeds `colorful.gif`, then loops it on hardware from UF2-resident data.
4. F4 comes back for timing polish, additional animations, and longer validation runs.

## Build and Workflow Notes

### Host baseline

The current root CMake project still describes the legacy host build. It remains as a reference only until `pico_build\` becomes the active firmware path.

### Pico happy path

The kickoff scaffold now lives in `pico_build\`. If the Pico SDK is checked out beside the repo at `C:\ws\pico-sdk`, the importer picks it up automatically. Otherwise pass an explicit SDK path:

```text
cmake -S pico_build -B pico_build\build -DPICO_SDK_PATH=C:\path\to\pico-sdk -DPICO_BOARD=pimoroni_plasma2350
cmake --build pico_build\build
```

For reviewable proof, keep the software build evidence and the hardware serial capture separate:

- `pico_build\proof\foundation-proof.md` records the reproduced local artifact evidence for the current scaffold
- `pico_build\tools\collect-proof.ps1` re-runs the proof build and can capture a live serial transcript when a board is attached

### Asset workflow

The expected asset workflow is:

```text
legacy gifs\ + external\TASBot-eye-animations\gifs\
    -> compare and select canonical animations
    -> preprocess offline
    -> emit generated assets in pico_build\assets\generated\
    -> link into UF2
```

The first landed Phase 3 slice uses `pico_build\tools\generate-gif-asset.ps1` to turn
`colorful.gif` into generated C data plus metadata under `pico_build\assets\generated\`, with
the canonical source selected by the documented submodule-first rule.

## Success Criteria

- Migration planning docs are centralized under `docs\`.
- Each migration phase has a dedicated feature doc.
- The repository clearly separates immutable inputs, tracked third-party asset sources, and future generated firmware assets.
- The asset strategy explicitly plans for comparing the legacy GIF tree against the submodule before UF2 embedding.
- The critical path to the first hardware demo is explicit, measurable, and centered on embedded `colorful.gif` playback.
