# F4: Validation and Release Readiness

**Status:** Planned  
**Scope:** Phase 4 of 4  
**Depends On:** F3

## Goal

Prove that the Pico path is buildable, repeatable, and behaviorally close enough to the legacy system to become the new happy path after the `colorful.gif` MVP demo already works.

## Validation Areas

### Build validation
- `pico_build\` configures and builds cleanly
- firmware artifacts are generated from the isolated Pico tree
- generated asset inputs are part of the normal build flow

### Playback validation
- `colorful.gif` continues to render reliably from embedded assets
- startup, base, blink, and any next-wave animations are added after the MVP demo
- LED mapping matches the expected 8x32 layout
- timing remains stable through longer animation runs

### Asset validation
- chosen canonical animations are traceable back to either `gifs\` or `external\TASBot-eye-animations\gifs\`
- the generated asset directory can be regenerated deterministically

### Repository validation
- root legacy sources remain untouched
- root `gifs\` stays ignored as a reference tree
- docs remain centralized under `docs\migration\`

## Acceptance Criteria

1. The Pico build story is clear and repeatable.
2. The offline asset pipeline can be rerun without manual repo surgery.
3. The firmware plays the MVP asset set from embedded data without relying on host filesystem paths.
4. Longer timing and stability runs are documented well enough to catch regressions after the first demo.
5. Documentation still points contributors to the correct source, submodule, and generated asset locations.

## Exit Condition

F4 is complete when a contributor can follow the documented Pico workflow, understand where assets come from, reproduce the firmware image, and extend the demo beyond `colorful.gif` without guessing which animation source tree is authoritative.
