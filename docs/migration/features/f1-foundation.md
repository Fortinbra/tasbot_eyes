# F1: Foundation, Layout, and Pico Project Isolation

**Status:** Planned  
**Scope:** Phase 1 of 4  
**Owner:** Platform / firmware setup

## Goal

Create a boring, reviewable foundation for the Pico migration:

- organize docs under `docs\migration\`
- create the isolated Pico firmware home in `pico_build\`
- preserve root source files as immutable reference inputs
- lock in the asset-source policy before any conversion work starts
- end with a bootable firmware stub so the next phase starts from real hardware footing

## Deliverables

- `docs\migration\` becomes the home for migration planning docs
- `pico_build\` exists as the isolated Pico SDK project root
- `pico_build\CMakeLists.txt` builds a minimal firmware target without pulling in legacy host dependencies
- `pico_build\src\firmware\main.c` brings up stdio and emits an explicit boot/ready window
- `pico_build\proof\foundation-proof.md` captures reproduced Pico artifact evidence and the remaining hardware-proof status honestly
- `external\TASBot-eye-animations\` is added as the tracked comparison asset submodule
- root `gifs\` is explicitly treated as a legacy local reference set and ignored going forward
- the repository documents `pico_build\assets\generated\` as the future output path for preprocessed assets

## Acceptance Criteria

1. Migration docs are no longer spread across root planning markdown files.
2. `pico_build\` configures as a standalone Pico SDK tree and a fresh proof build reproduces firmware artifacts (`.elf`, `.bin`, `.hex`, `.uf2`, `.dis`) without modifying the root build.
3. The stub's boot-visible contract is explicit and reviewable: first ready emission occurs at 2 seconds after boot and remains visible through the repeated ready window, while the hardware gate stays open until a captured serial log proves the board actually emits that ready line within 3 seconds.
4. The docs call out `pico_build\` as the isolated firmware build location.
5. The docs distinguish:
   - root `gifs\` for legacy comparison only
   - `external\TASBot-eye-animations\gifs\` for tracked animation sources
   - `pico_build\assets\generated\` for generated firmware assets
6. Repository tracking policy no longer treats the root `gifs\` folder as a maintained tracked asset source.
7. The source-selection rule is explicit: prefer the submodule when available, fall back to legacy `gifs\` otherwise, and fail loudly when neither source contains the requested animation.

## Notes for Implementation

- Favor official Pico SDK layout when `pico_build\` is created.
- Keep platform glue out of root application files.
- Treat documentation cleanup as part of the foundation, because unclear paths create fragile build stories later.
- Do not treat source inspection as a substitute for hardware proof; keep local build evidence and on-device serial evidence as separate claims.

## Exit State

F1 is done when a contributor can point at one isolated Pico tree, one documented asset policy, one reproduced Pico artifact bundle, and one boot-ready stub contract without guessing which files are safe to edit. The on-device serial-ready gate is only considered passed once a captured hardware transcript is attached.

## Risks

| Risk | Mitigation |
|---|---|
| Mixed documentation paths cause confusion | Keep all migration planning docs under `docs\migration\` |
| Asset sources drift without clear ownership | Track the submodule and ignore the legacy root GIF dump |
| Future firmware work leaks into legacy files | Keep `pico_build\` isolated and document that boundary early |

## Handoff

Once F1 is accepted, F2 can start porting runtime logic into a Pico-safe structure without reopening repository layout, bootstrapping, or asset-source ownership questions.
