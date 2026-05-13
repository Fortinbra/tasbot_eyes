# F04: -c Random Monochrome Recolor

## Status

- Done.
- Implemented with runtime monochrome detection fallback, random palette selection, and non-black pixel recolor stage.

## Legacy Intent

`-c` enables random color application for monochrome animations. Colored animations keep their original palette.

## Firmware Scope

Add a recolor stage that applies a random selected color only when the current animation/frame is monochrome according to chosen detection strategy.

## Static Firmware Constants (Current)

Define these as constants (for example in `board.h` or a dedicated firmware feature header):

- `randomize_monochrome` (bool)
- `recolor_palette` (array of RGB colors)

## Future Config Surface

- compile-time override macros
- optional runtime config channel

Metadata strategy (choose one):

- generation-time `is_monochrome` flag per animation (preferred)
- runtime frame scan fallback (higher CPU cost)

## Implementation Checklist

1. Define monochrome detection source (metadata first, runtime fallback optional).
2. Add random palette picker and seed strategy.
3. Apply recolor only to non-black pixels in monochrome content.
4. Keep colored animations unchanged.
5. Add diagnostics: recolor applied count, skipped count.

## Independent Test Plan

### Unit Tests

1. Monochrome detector identifies monochrome test frame correctly.
2. Recolor pass leaves black pixels black.
3. Colored frame bypasses recolor when feature enabled.

### On-Board Validation (Firmware)

1. `randomize_monochrome=true` with monochrome animation:
- verify non-black pixels adopt selected random color

2. `randomize_monochrome=true` with colored animation:
- verify original colors preserved

3. `randomize_monochrome=false`:
- verify no recolor on either asset type

### Pass Criteria

- Recolor only occurs for monochrome content.
- Output remains stable per frame (no accidental per-pixel random flicker unless explicitly desired).
- No noticeable frame-time regression from detection/recolor path.

## Out of Scope

- `-a` behavior (randomize blinks/base too)
- artist-defined per-region recolor rules
