---
name: "project-conventions"
description: "Core conventions and patterns for this codebase"
domain: "project-conventions"
confidence: "medium"
source: "repo-observed"
---

## Context

Conventions that matter for the ongoing Pico migration and repository hygiene work.

## Patterns

### Documentation Layout

- Keep migration planning docs under `docs\migration\`.
- Put one phase doc per file under `docs\migration\features\`.
- Root `README.md` can point at docs, but migration planning markdown should not accumulate at repo root.

### Platform Separation

- Treat root `*.c` and `*.h` files as immutable legacy references during migration planning.
- Keep Pico-specific build glue and future firmware work isolated in `pico_build\`.
- Do not mix platform setup decisions into the original host application files.

### Asset Source Ownership

- Root `gifs\` is legacy reference material and should remain untouched locally.
- The tracked comparison source for animations lives in `external\TASBot-eye-animations\`.
- Generated firmware-ready assets belong under `pico_build\assets\generated\`, not beside the legacy GIF dump.

### Build Story

- Prefer the standard Pico SDK layout and tooling when the firmware tree is created.
- Keep build outputs out of source directories; `pico_build\build\` is the planned transient output path.

## Examples

```
docs\migration\README.md
docs\migration\features\f3-assets-playback.md
external\TASBot-eye-animations\
pico_build\assets\generated\
```

## Anti-Patterns

- **Root planning doc sprawl** — Do not keep adding migration planning markdown files at the repo root.
- **Tracking both asset trees as maintained sources** — The root `gifs\` tree is for comparison only; use the submodule for tracked upstream animation sources.
- **Leaking platform work into legacy files** — Keep Pico build and firmware glue isolated from original application files.
