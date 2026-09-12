# ASCENDENDO difficulty model

## Status

The authoritative mechanical validator answers **whether a level is mechanically reachable**. It is a PASS/FAIL concern and is not a player-difficulty metric.

PR #343 is now explicitly the **Challenge / Control Analyzer**. Its physics-backed control-space measurements are diagnostic features: launch-position coverage, charge-window coverage, transition feasibility and reachable-route structure. They are not an authoritative human difficulty score.

PR #344 is the player-simulation layer. It now runs full single-level episodes with the existing fixed-step physics replica rather than a hand-written stochastic difficulty proxy.

## Primary output: gameplay observations

ASCENDENDO levels use the same fixed single-screen geometry, so completion time is directly comparable across levels. The first metric to investigate is therefore the empirical distribution of

`T_completion`

rather than an arbitrary 0-100 score.

Reports should expose at least:

- median completion time;
- p75 and p90 completion time;
- completion-rate coverage as a secondary diagnostic;
- progress lost in pixels after failures;
- largest setback;
- setback count;
- recovery time until the previous progress high-water mark is restored.

A logarithm of median completion time may later be useful for comparisons, but it is deliberately **not fixed as the production difficulty metric before we inspect simulated and human data**.

## Failure severity is separate from execution difficulty

A failed transition is not represented as a death. The simulator records where the physics actually leaves the player.

For a failure at progress `P_before` that lands at `P_after`:

`progress_loss_px = max(0, P_before - P_after)`

The level therefore preserves the full spatial risk distribution instead of converting setbacks into an arbitrary number of "lives" or "levels lost".

Useful diagnostics include:

- `E[progress_loss_px | failure]`;
- p50/p90/p99 progress loss;
- `P(loss >= x)` for chosen pixel thresholds;
- largest observed setback.

This keeps **execution difficulty** distinct from **failure severity**. Two levels can have similar completion times but radically different punishment profiles.

## Recovery cost

A setback is not fully described by its size. Recovery is the elapsed gameplay time from the failure landing until the previous progress high-water mark is restored.

This gives a third dimension:

- execution difficulty: how demanding the transitions are for a player profile;
- failure severity: how much spatial progress is lost when execution fails;
- recovery cost: how much gameplay time is spent regaining the lost progress.

These dimensions must remain separate in diagnostics and campaign review.

## Player model

Profiles are hypotheses, not human facts. They currently vary:

- reaction time and reaction jitter;
- horizontal execution error;
- charge error;
- braking error;
- aim error;
- route-choice noise;
- consistency;
- learning rate after failure.

Learning changes the error state between failed transitions, so repeated sessions can produce skill-improvement curves. Human calibration remains a later research step.

## Episode simulator

`Development/AI_Validation/sim/player_episode.py` uses the existing fixed-step physics replica to:

1. choose a target from the currently reached geometry;
2. find a nominal action through actual physics simulation;
3. perturb the launch and charge according to the player profile;
4. execute the whole jump, including accidental lower-platform landings;
5. continue from the resulting physical position;
6. accumulate completion time, spatial setbacks and recovery windows;
7. finish naturally when the derived goal is reached.

Nominal action planning is cached per level/source geometry and reused across all agents and seeds. The expensive physics search is therefore not repeated independently for every population member.

`max_events` is only a computational safety guard for pathological experiments. It is not a gameplay timeout and an incomplete run is not assigned an artificial completion time.

## Population experiments

Population size is tiered by purpose:

- one level: default `1000` agents for a stable distribution;
- full campaign exploration: default `100` agents per profile and level;
- targeted follow-up: increase the population only for selected levels/profiles after the exploration identifies interesting regions or campaign transitions.

Examples:

```powershell
# broad campaign exploration; 100 agents/profile/level
python3 Development/AI_Validation/difficulty_report.py --campaign

# high-resolution study of one level; 1000 agents/profile
python3 Development/AI_Validation/difficulty_report.py --level Game/Assets/Levels/nivel_18.lvl
```

The result is a distribution, for example:

```text
Level 18
median completion: 38.4 s
p90 completion:    51.7 s
p90 progress loss: 118 px
median recovery:    4.1 s
```

These values are descriptive experimental observations until the profile parameters are calibrated against human telemetry.

## Control analyzer → player simulation

The intended architecture is now:

```text
Mechanical Validator
        │
        └── PASS/FAIL

Challenge / Control Analyzer (#343)
        │
        └── geometry/control features
                │
                ↓
          Player Profiles
                │
                ↓
        Full Episode Simulation
                │
        ┌───────┼────────┐
        ↓       ↓        ↓
      Time    Loss    Recovery
        │       │        │
        └───────┼────────┘
                ↓
       Difficulty / Risk Profile
```

The simulator is therefore an experiment that **observes gameplay**, rather than a formula that declares gameplay difficulty from geometric weights.

## Human calibration path

1. Keep mechanical validation separate.
2. Keep control-space features as diagnostics.
3. Run full-episode simulations over multiple profiles and seeds.
4. Collect completion-time, progress-loss and recovery distributions.
5. Collect human telemetry using the same schema.
6. Fit/calibrate player profiles against observed human behavior.
7. Test whether simulated distributions reproduce human rank/order and variance.
8. Only then consider a scalar difficulty summary, if the evidence justifies one.

Until human telemetry exists, reports must explicitly distinguish **simulated observations** from **human-calibrated difficulty**.

## Campaign use

Campaign progression should be inspected as multiple curves:

- completion-time progression;
- execution-risk progression;
- failure-severity progression;
- recovery-cost progression.

The campaign does not need to be strictly monotonic. Deliberate recovery levels and challenge peaks are valid design choices; the tester should reveal them rather than force every level into an arbitrary score ladder.
