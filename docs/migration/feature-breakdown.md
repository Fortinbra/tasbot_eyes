# Pico Migration Feature Breakdown

## Summary

The migration is organized as four feature phases. Each phase has its own document so build, runtime, and asset decisions stay scoped and reviewable.

## Phase Breakdown

| Phase | Goal | Key Outputs | Depends On |
|---|---|---|---|
| F1 | Establish the migration foundation | isolated `pico_build\` scaffold, source policy, bootable stub | None |
| F2 | Port runtime behavior into a Pico-safe model | portable core copies, event loop, LED seam, hardware smoke test | F1 |
| F3 | Build the offline asset and playback path | asset comparison workflow, converter, generated `colorful.gif`, first real demo | F2 |
| F4 | Validate and harden the firmware path | longer runs, broader asset set, mapping and timing polish, release readiness | F3 |

## Cross-Phase Rules

1. **Original files stay untouched.** Root application sources remain reference-only.
2. **Pico work stays isolated.** New build logic, generated assets, and platform glue live under `pico_build\`.
3. **Asset intake is explicit.** Compare the ignored legacy `gifs\` tree against `external\TASBot-eye-animations\gifs\` before generating embedded assets.
4. **No runtime filesystem on device.** The UF2 contains the chosen animation data.

## Critical Path for the First Demo

The project should optimize for the earliest honest proof that the board can play a real animation, not for immediate full parity with the Raspberry Pi version.

1. **F1 exit:** `pico_build\` exists, configures as an isolated Pico target, and boots a serial-ready stub without touching root sources.
2. **F2 exit:** the Pico shell can drive the LED transport and render deterministic frames on the actual 8x32 array.
3. **F3 exit:** `colorful.gif` is selected, converted offline, compiled into the UF2, and loops on hardware.
4. **F4 exit:** extra animations, timing polish, source comparison completeness, and release confidence are added after the MVP demo already works.

## Validation Gates

### Gate 1: Layout clarity
- Docs live under `docs\`.
- The firmware plan points at `pico_build\`.
- Asset source locations are documented unambiguously.
- The submodule-preferred, legacy-fallback source rule is documented.

### Gate 2: Platform seam isolation
- Portable logic can be copied into `pico_build\src\portable\`.
- Platform glue stays in `pico_build\src\firmware\`.
- Root sources remain unchanged.
- The firmware shell can render at least one deterministic frame pattern on hardware.

### Gate 3: Asset pipeline readiness
- Submodule assets are available at `external\TASBot-eye-animations\`.
- Legacy assets remain available locally in `gifs\`.
- The plan defines how both sets will be compared and converted.
- `colorful.gif` has a chosen canonical source and a defined generated output contract.

### Gate 4: Playback readiness
- `colorful.gif` remains the first end-to-end playback target and the Phase 3 exit gate.
- Generated assets have a defined landing zone in `pico_build\assets\generated\`.
- The demo requirement is explicit: a flashed UF2 loops `colorful.gif` on the physical array without filesystem access.

## Doc Map

- Overview: `docs\migration\README.md`
- F1: `docs\migration\features\f1-foundation.md`
- F2: `docs\migration\features\f2-core-runtime.md`
- F3: `docs\migration\features\f3-assets-playback.md`
- F4: `docs\migration\features\f4-validation.md`
