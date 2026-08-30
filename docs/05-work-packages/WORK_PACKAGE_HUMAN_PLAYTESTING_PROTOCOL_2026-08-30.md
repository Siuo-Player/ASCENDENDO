# ASCENDENDO — Work Package: Human Playtesting Protocol

**Date:** 2026-08-30  
**Phase:** Fase 10 — presentation/camera validation → human playtesting  
**Status:** PROTOCOL DEFINED — NO HUMAN EVIDENCE YET

## Why this exists

The deterministic capture work now proves that the renderer can produce real framebuffer captures at the three target viewport classes (16:9, 4:3 and 21:9). It does not prove that a human player can read the scene, understand the next action, control the character comfortably, or judge the camera as acceptable.

Those are empirical properties. They require observations from human sessions and must not be inferred from unit tests or screenshot existence checks.

## Evidence boundary

This work package does **not** claim:

- that the camera feels good;
- that all platforms are sufficiently legible;
- that the goal/flag is salient enough;
- that the difficulty progression is balanced;
- that the presentation works equally well for all players.

Those claims remain OPEN until human data exists.

## Protocol

Use the same three viewport classes already validated by the deterministic capture pipeline where feasible:

```text
16:9  — baseline
4:3   — reduced horizontal field
21:9  — increased horizontal field
```

Test the current playable campaign without changing level geometry between sessions.

Each session should record:

1. viewport class and effective framebuffer dimensions;
2. level reached/completed;
3. deaths/restarts;
4. time to first successful landing on representative jumps;
5. whether the player can identify the next intended platform before committing to a jump;
6. whether camera motion causes an observable loss of spatial orientation;
7. whether the goal/flag is noticed when it first becomes relevant;
8. explicit player-reported friction points immediately after the task.

## Task design

The initial protocol should use short, repeatable tasks rather than an unrestricted “play and tell us what you think” session.

### Task A — orientation

Start at the beginning of the campaign and ask the player to identify the intended route before making the first jump.

**Observation:** whether the next actionable geometry can be identified without prompting.

### Task B — camera tracking

Play through a section that requires vertical camera movement.

**Observation:** whether the player loses track of player/platform/world relationship during camera movement.

### Task C — viewport robustness

Repeat a representative section at 16:9, 4:3 and 21:9.

**Observation:** differences in route comprehension, platform visibility and camera-related confusion between viewport classes.

### Task D — goal salience

Reach the first state where the goal/flag is relevant.

**Observation:** whether the player notices the goal without explicit instruction.

## Data treatment

Do not treat a single participant or anecdote as proof of a general design property.

For the first pass, report:

- raw observations per session;
- task completion rates;
- median/dispersion for measurable timings when sample size permits;
- qualitative themes with the number of sessions in which each occurred;
- viewport-specific differences.

Keep observations and interpretations separate.

## Promotion rule

A human-playtesting finding may justify a production change only when the evidence identifies:

```text
repeatable observation
+
plausible affected property
+
clear consumer / player impact
+
change that can be isolated and retested
```

Do not modify camera speed, bounds, level geometry or art solely because a single player expresses a preference.

## Relationship to existing deterministic evidence

The deterministic capture pipeline remains useful as the controlled environment for the human protocol, but it is not a substitute for human evidence.

```text
Deterministic captures
        ↓
controlled visual stimulus
        ↓
Human playtesting
        ↓
empirical usability evidence
        ↓
only then: candidate design changes
```

## Next step

Collect a first small pilot set using the protocol above. Do not calibrate difficulty or procedural generation from the pilot until the observations have been reviewed for consistency and confounders.
