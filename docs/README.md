# Documentation Index

This repository keeps Pico migration planning under `docs\migration\` instead of scattering planning markdown files at the root.

## What lives here

| Path | Purpose |
|---|---|
| `docs\migration\README.md` | High-level Pico migration plan, repo layout, and source-of-truth paths |
| `docs\migration\feature-breakdown.md` | Canonical phase order, critical path, and validation gates |
| `docs\migration\features\f1-foundation.md` | Foundation and isolated Pico scaffolding |
| `docs\migration\features\f2-core-runtime.md` | Runtime shell, hardware seam, and smoke-test playback |
| `docs\migration\features\f3-assets-playback.md` | Offline asset pipeline and `colorful.gif` demo gate |
| `docs\migration\features\f4-validation.md` | Broader validation, extra animations, and polish after MVP |
| `docs\PHASE_*.md` | Earlier planning snapshots retained for reference only; canonical decisions live under `docs\migration\` |

## Working rules captured by these docs

- Root application sources stay untouched.
- Pico SDK work is isolated in `pico_build\`.
- Legacy animation inputs remain in the root `gifs\` tree as local reference material only.
- The tracked comparison set lives in the submodule at `external\TASBot-eye-animations\`.
- The eventual offline asset pipeline compares both sources, picks canonical animations, and emits generated assets for `pico_build\assets\generated\` to compile into the UF2.
- The fastest demo path is: isolated Pico scaffold -> hardware smoke test -> embedded `colorful.gif` playback -> broader asset and timing polish later.
