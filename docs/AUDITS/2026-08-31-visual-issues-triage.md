# ASCENDENDO — Visual issues triage — 2026-08-31

## Current state

The current production main includes the deterministic renderer/capture and viewport evidence tranches through PR #196.

The open visual issues were checked against the live implementation:

- #166 — gameplay-first visual direction + safe UGC licensing: policy/acceptance intent; no direct runtime change justified by the issue alone.
- #167 — creator artwork/import rights: policy constraints; issue explicitly says not to implement solely from the issue.
- #168 — gameplay-first visual foreground priority: visual acceptance criterion; implementation requires golden-scene evidence.
- #169 — background remains visually secondary: permanent visual rule; implementation requires visual evidence at supported viewport sizes.

## Evidence boundary

The repository now proves technical viewport capture at 16:9, 4:3 and 21:9, but that evidence does not establish human readability, salience or movement feel.

Therefore:

```text
technical capture evidence
    !=
human visual/usability evidence
```

## Decision

Do not alter camera, background contrast, parallax, asset policy or UGC runtime behaviour solely from #166–#169.

The first appropriate implementation trigger is a reproducible visual finding from the existing playtest protocol or a deterministic golden-scene failure against an explicitly defined visual property.

## Next action

Proceed with the first real human playtest using the already-integrated session sheet. Keep #166/#167 as policy references and #168/#169 as acceptance criteria for subsequent presentation work.
