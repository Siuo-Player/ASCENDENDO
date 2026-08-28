# ASCENDENDO — residual replay contracts audit

**Roadmap:** Gate 9.6 — Base Engineering Gate
**Status:** INVESTIGATED / THREE RESIDUALS ISOLATED
**Date:** 2026-08-28

## Current evidence

`ReplayManager::recordTick()` stores the pre-tick `Player`/world accumulator state and the corresponding `TickInput`. `preparePlaybackTick()` reproduces the stored `TickInput` sequence in order. `startPlayback()` resets the playback cursor.

That is evidence for a tick-semantic replay mechanism. It is not evidence for every externally visible notion of replay fidelity.

## Residual 1 — live input and frame-rate independence

Normal input is sampled by `InputManager` before the `GameSession` update. A stronger claim would require showing that different external render-frame groupings, producing the same semantic tick inputs, lead to the same simulation outcome.

## Residual 2 — terminal/result replay

The current replay history stores player/world pre-tick state and inputs, but does not define a complete recording of campaign completion/result metadata. Therefore terminal replay equivalence of an entire `GameSession` remains unproven.

## Residual 3 — persistence

Replay state is held in memory. `ReplayManager` has no persistence format, schema, serializer, or reload operation. Persistence is a separate capability and should not be inferred from in-memory playback.

## Decision

Keep the three properties independent. The next executable tranche should target terminal/result replay and controlled live-input frame-rate independence. Persistence should receive its own format/compatibility WP if it becomes a requirement.

No production change is justified by this audit alone.

## Conclusion

The replay design is further along than a frame recorder but narrower than a complete replay system. Gate 9.6 should preserve that distinction instead of turning tick-level evidence into an unsupported universal claim.