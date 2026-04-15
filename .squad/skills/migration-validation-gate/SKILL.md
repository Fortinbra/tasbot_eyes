---
name: "migration-validation-gate"
description: "Review an in-flight migration phase against concrete regression gates and reject build theater"
domain: "testing"
confidence: "high"
source: "earned"
---

## Context

Use this when a platform migration claims progress because files exist or CMake configures, but the real risk is hidden coupling, missing artifact proof, or unverified hardware behavior.

## Patterns

### Start from the preserved baseline

- Re-run the legacy configure/build story in a fresh build directory.
- Treat unchanged failure modes as evidence the migration did not contaminate the old path.
- Do not silently upgrade the gate from "unchanged baseline" to "successful host port" unless the feature explicitly requires that.

### Separate file inspection from behavior proof

- Files can prove layout, source-policy, and symbol hygiene.
- Files cannot prove emitted artifacts or visible boot behavior; require fresh build logs, artifacts, or hardware captures.
- Treat inherited notes in agent history as claims to verify, not proof.

### Audit the seam explicitly

- Search the new migration tree for forbidden legacy/platform headers and APIs.
- Confirm the new target links only the expected minimal dependencies for its current phase.
- Verify generated-output lanes and ignored transient paths are explicit.

### Reject on missing evidence, not just broken code

- If a gate says `.uf2` exists, reproduce or inspect the artifact directly.
- If a gate says "serial-ready within 3 seconds," require a captured serial log or equivalent hardware evidence.
- A plausible stub is not the same as a passed gate.

## Examples

- `pico_build\CMakeLists.txt` imports the Pico SDK and enables extra outputs, but acceptance still waits on actual `.elf`/`.uf2` generation.
- `pico_build\src\firmware\main.c` prints a banner after `sleep_ms(2000)`, but the gate stays open until flashing/serial evidence exists.
- Root host build failing on the same old missing headers (`gif_lib.h`, `ws2811/ws2811.h`, `unistd.h`, `dirent.h`, `pthread.h`) is useful regression evidence.

## Anti-Patterns

- **Accepting build theater** — approving a migration because the directory structure looks right.
- **Trusting prior notes blindly** — treating another agent's history entry as substitute for current proof.
- **Collapsing file proof and runtime proof** — inferring hardware success from a `puts()` call in source.
