---
name: "split-proof-gates"
description: "Separate software artifact proof from hardware-attached proof in embedded reviews"
domain: "testing"
confidence: "high"
source: "earned"
---

## Context
Use this when reviewing embedded or firmware migration work where part of the claim can be reproduced locally, but final behavior still depends on attached hardware.

## Patterns
- Reproduce the software-side build artifacts yourself from a fresh directory.
- Re-run the legacy or baseline build story to prove the new slice did not contaminate the old one.
- Treat source-visible timing or boot contracts as reviewable software claims, not as substitutes for on-device proof.
- Approve or reject with the scope named explicitly: software proof may pass while hardware proof remains open.
- Call out any path-sensitive artifacts if hashes depend on the exact build directory.
- If path-sensitive hashes are part of the published proof, make the proof tool's default build directory match the documented review path so the bare command and the explicit command reproduce the same package.

## Examples
- `pico_build\tools\collect-proof.ps1` reproduces Pico artifacts locally.
- `pico_build\proof\foundation-proof.md` documents software proof separately from serial capture.
- `pico_build\tools\collect-proof.ps1` can default to `pico_build\build\ws2812-proof` so the published `.elf`, `.dis`, and `.elf.map` hashes match the documented bare command.
- Root `CMakeLists.txt` build failure remains a regression checkpoint proving isolation.

## Anti-Patterns
- Accepting inherited proof notes without rerunning the build.
- Treating embedded strings in an ELF as equivalent to captured hardware serial output.
- Mixing software approval with hardware approval into one vague sign-off.
- Publishing hashes from one build directory while documenting a different default command path.
