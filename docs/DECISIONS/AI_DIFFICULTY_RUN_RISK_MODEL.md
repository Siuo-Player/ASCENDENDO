# ASCENDENDO — Difficulty, Fall Risk and Run-Cost Model

## Status

**Research/decision document. Implementation follows only after the model is validated against the real game loop.**

This document supersedes the current assumption that a campaign level can be evaluated as an isolated single-screen gameplay episode.

The key correction is:

> An ASCENDENDO level is a spatial/content unit, but the gameplay experience is a continuous vertical run. Difficulty evaluation must therefore model the run state, not only the isolated level.

PR #343 remains useful as a **Challenge / Control Analyzer**. PR #344 is useful as an experiment layer, but its current isolated-level episode is insufficient for the final difficulty evaluator because it cannot reproduce cross-level progress loss.

---

## 1. What the game actually is

ASCENDENDO is built around the **Commitment Jump**:

- the player chooses jump strength;
- the jump is launched at a fixed 60 degree angle;
- there is no air steering;
- the player therefore commits to a trajectory before knowing its complete outcome.

The core design loop is therefore:

```text
observe
→ position
→ choose direction
→ choose charge
→ commit
→ land / miss
→ continue from the physical result
```

The campaign is vertically assembled. A campaign entry is a fixed-size content chunk, but runtime appends subsequent chunks upward into the same world coordinate system. Falling after a failure therefore does not necessarily remain inside the authored chunk in which the failure happened: a player can land on lower geometry belonging to earlier campaign content.

This distinction is essential for difficulty evaluation.

### 1.1 Single-screen is useful, but it does not make the run single-screen

The fact that campaign maps use the same fixed screen geometry is valuable because execution time and spatial distances are directly comparable.

It does **not** mean that a level can be simulated independently when evaluating its contribution to the real campaign experience.

The production runtime appends campaign chunks at increasing world `Y` offsets. The same physics world therefore contains the already-passed lower content when the player ascends. A high-level mistake can consequently move the player into a much lower part of the run.

The tester must preserve that topology.

---

## 2. What Jump King actually teaches us

The inspiration is not merely “hard jumps”. It is the coupling between:

1. execution difficulty;
2. the physical destination after a miss;
3. the amount of progress that remains lost;
4. the time required to regain that progress;
5. the psychological stakes created by knowing that a mistake can undo substantial progress.

Nexile itself describes *Jump King* as high-stakes platforming where a missed jump can cause a long fall and the player must climb back up again. The official description also says the player must think before jumping and cannot turn back while airborne. https://store.steampowered.com/app/1061090/Jump_King/

The official Workshop guidance reinforces that fall topology is deliberate level-design content. It warns level authors not to overuse teleport blocks, recommends allowing full falls to the start unless checkpoints are intended, and explicitly treats playtesting as essential. https://teamnexile.github.io/jk-workshop-docs/level-making/tips/

The Workshop documentation also defines an explicit screen coordinate system for vertically stacked screens. https://teamnexile.github.io/jk-workshop-docs/level-making/hitboxes/

Community analysis provides concrete examples where a jump's difficulty is inseparable from its failure consequence: some sections contain “safety nets”, while others can send the player several screens back. Those observations are not authoritative evidence for ASCENDENDO, but they are useful prior art for the design pattern. https://avidachievers.com/speedruns/jump-king-speedrun-guide/

### Consequence for ASCENDENDO

A miss must not be represented as:

```text
failure = +penalty
```

It must be represented as an actual **state transition produced by the physics**:

```text
jump attempt
    ↓
physics outcome
    ↓
new position / landed platform
    ↓
new available actions
    ↓
future recovery path
```

The fall is therefore part of the game state, not an external punishment variable.

---

## 3. The central model: run state, not isolated jump score

The evaluator should model the campaign as a stochastic gameplay process.

A useful abstract state is:

```text
S = {
    world position,
    current support/platform,
    progress high-water mark,
    player skill state,
    current route/knowledge state
}
```

An attempted action produces a transition:

```text
S_t
  │
  │ direction + launch position + charge
  ↓
physics
  │
  ├── success → S_{t+1} at higher progress
  │
  └── miss   → S_{t+1} at the actual lower landing position
```

Each transition carries **time** and **physical progress** as observations.

This is closer to the real game and to the useful part of the *Jump King* design pattern than treating every failed jump as an independent Bernoulli penalty.

---

## 4. What should be measured

There should not be one arbitrary 0–100 difficulty formula at this stage.

The evaluator should first produce distributions that describe the actual run.

### 4.1 Completion time

For any completed run:

```text
T_completion = elapsed time from run start to campaign completion
```

Because the authored chunks share the same screen dimensions, this is a strong first-order measure of practical challenge for comparable content.

For a single level/chunk, an isolated completion-time experiment remains useful as a **local challenge measurement**.

For campaign difficulty, however, the authoritative experiment is the **full vertical run** because setbacks can propagate across level boundaries.

Recommended summary statistics:

- median completion time;
- p75;
- p90;
- p95 where sample size supports it;
- variance/spread.

No artificial gameplay timeout is needed to define this metric. A simulation ends naturally at completion. A compute/event guard may still terminate pathological simulations, but such runs are censored/incomplete observations rather than assigned a fake completion time.

### 4.2 Progress loss in pixels

Use world-space pixels as the fundamental unit.

For a failure after a previous high-water progress `P_high` and settled landing progress `P_land`:

```text
progress_loss_px = max(0, P_high - P_land)
```

Do not convert this into “levels lost”. A continuous pixel measure preserves the actual geometry and remains meaningful even when authored chunks contain different platform arrangements.

Report:

- median progress loss;
- p90/p95/p99 progress loss;
- `P(loss >= x)` for useful pixel thresholds;
- largest observed loss;
- full failure-loss distribution when analysing a specific transition.

### 4.3 Fall destination distribution

The most important information is often not just the distance lost, but **where the player lands**.

For each risky transition, record:

```text
failure
→ landing world Y
→ landing support/platform
→ progress loss
→ immediately available recovery route
```

Two jumps with the same probability of failure can therefore have very different run consequences.

Example:

```text
Jump A
    failure probability = 0.30
    typical loss = 24 px

Jump B
    failure probability = 0.30
    typical loss = 420 px
```

They should not receive the same risk classification merely because their execution success probabilities are equal.

### 4.4 Recovery time

For a failure that drops the player below the previous high-water mark, measure:

```text
T_recovery = time from the fall landing
            until the previous high-water progress is restored
```

This is more informative than progress loss alone.

A 300 px loss can be cheap if the lower section is trivial and expensive if it contains several demanding transitions.

### 4.5 Incremental run cost

The most important combined quantity should be expressed in **time**, not an arbitrary weighted score.

For failures originating from a region/chunk `R`, measure:

```text
recovery_time_R
```

and its empirical expectation/quantiles over full runs.

This naturally combines:

```text
probability of failure
×
where the player lands
×
what must be replayed
×
time required to recover
```

without choosing arbitrary weights such as 40% robustness + 30% landing margin.

The practical result is:

> a difficult upper section can be expensive because it creates rare but very large recoveries, even when its local successful execution time is not extreme.

This is exactly the distinction the current isolated-level experiment cannot measure.

---

## 5. Difficulty and failure risk are related, but not the same variable

The evaluator should expose at least these three dimensions:

### Execution difficulty

How demanding it is for a player profile to make progress.

Observed through:

- completion time;
- action timing/precision;
- failed transition frequency;
- learning curve;
- variability across runs.

### Fall severity

What the physics does when the player fails.

Observed through:

- landing distribution;
- progress-loss distribution in pixels;
- probability of large setbacks.

### Recovery cost

How much future gameplay time is spent restoring lost progress.

Observed through:

- recovery time;
- number of recovery transitions;
- repeated exposure to previously completed challenges.

These dimensions should be reported separately before any scalar is considered.

---

## 6. The important campaign-level question

The key question is not:

> “How difficult is level 20 in isolation?”

It is:

> “What does reaching level 20 add to the difficulty and risk of completing the run?”

This suggests two complementary reports.

### Local challenge report

Run the chunk in isolation to measure:

```text
local completion time
local control demand
local transition difficulty
```

This is useful for authoring and comparing geometry.

### Campaign impact report

Run the complete stacked campaign and attribute observations back to the region where they originated:

```text
completion time contribution
failure frequency
progress-loss distribution
recovery-time distribution
run-time impact of failures from this region
```

The campaign impact report is the one that should drive final progression decisions.

---

## 7. Why completion-time-only is still insufficient

Completion time is the correct starting point for ASCENDENDO because maps share the same fixed geometry and there is no artificial timer.

But the tester must not interpret:

```text
Level A: median = 24 s
Level B: median = 31 s
```

as proof that B is “7 seconds harder”.

Suppose:

```text
A
    median clean execution = 24 s
    failures are small

B
    median clean execution = 31 s
    failures are rare
    but one failure commonly loses 400 px
```

A full-run experiment may reveal that B adds far more expected recovery time than its +7 seconds of clean execution suggest.

Conversely, a geometrically difficult level with excellent safety nets may have high local execution demand but low run-level punishment.

Therefore:

```text
local execution time
        ≠
run-level difficulty impact
```

The evaluator must preserve both.

---

## 8. Player simulation requirements

The player simulator should represent the **real decision problem** without smuggling the final answer into a hand-written scoring formula.

The current PR #344 contains an important experimental warning sign: target/action selection still uses a hand-weighted heuristic that combines vertical gap, landing margin, effort and action time. That is better than a static level score, but it reintroduces arbitrary weights inside the simulated player's behaviour.

The eventual evaluator should separate:

```text
Physics model
    ↓
Action options actually available
    ↓
Player policy / decision model
    ↓
Execution noise
    ↓
Physical outcome
```

The player policy must be treated as a behavioural hypothesis and calibrated against human telemetry, not silently treated as ground truth.

### Learning

Learning should be represented as a changing player state, but the update rule is also a hypothesis until human data supports it.

The useful observation is the empirical learning curve:

```text
attempt / exposure count
    ↓
execution error
    ↓
completion time
    ↓
failure / recovery behaviour
```

---

## 9. The simulator must use the campaign world for risk evaluation

For final difficulty/risk experiments:

```text
campaign.txt
    ↓
append levels in runtime order
    ↓
single continuous world coordinates
    ↓
full physics simulation
    ↓
player can fall onto previously passed geometry
```

This is non-negotiable for the Jump King-inspired risk model.

An isolated level simulator can still exist, but its reports must be explicitly labelled **local/chunk difficulty**.

It must not be used to infer the campaign's full fall-risk structure.

---

## 10. Failure transition data model

Each attempted jump should ultimately produce a record conceptually similar to:

```text
JumpEvent
    source_world_y
    source_platform
    intended_target
    launch_x
    direction
    charge

    success

    if success:
        landing_world_y
        landing_platform
        execution_time

    if failure:
        landing_world_y
        landing_platform
        progress_loss_px
        recovery_target_y
        recovery_time_s
```

The evaluator can then reconstruct both:

```text
local challenge
```

and:

```text
run-level risk graph
```

from the same telemetry.

---

## 11. Natural stopping and censoring

ASCENDENDO does not need a gameplay time limit merely to make simulations measurable.

A completed simulation provides a natural completion-time observation.

A computational safeguard is still required to prevent a pathological stochastic policy from running forever. That safeguard must be treated as **censoring**, not as a gameplay outcome.

Therefore:

```text
completed
    → valid completion-time observation

compute guard reached
    → censored / incomplete observation

player abandoned
    → only applicable to human telemetry or an explicitly modelled abandonment policy
```

Do not convert the compute guard into “completion probability before N minutes”.

---

## 12. Recommended campaign reports

The campaign tester should eventually produce at least four aligned curves:

```text
1. Completion-time distribution by level/region
2. Failure probability by transition/region
3. Progress-loss distribution by transition/region
4. Recovery-time distribution by transition/region
```

And one combined, non-arbitrary quantity:

```text
expected recovery-time contribution to the run
```

A campaign may intentionally rise and fall in local difficulty. The evaluator should reveal those changes rather than force monotonicity.

---

## 13. Relation to existing work

### PR #343 — Challenge / Control Analyzer

Keep it as the geometric/control layer.

Useful outputs include:

- reachable transitions;
- launch-position coverage;
- charge-window coverage;
- control-space width;
- alternative route structure.

These are explanatory features, not the final difficulty score.

### PR #344 — Player simulation experiment

Keep the player-profile/full-episode approach, but change the semantic target of the experiment:

- isolated single-level episodes are **local experiments**;
- final risk/difficulty evaluation must use the vertically stacked campaign world;
- progress loss must be measured against the run high-water mark in world pixels;
- fall outcomes must remain attached to their actual landing states;
- completion time must be a natural run observation;
- current hand-weighted player policy is experimental and requires calibration.

---

## 14. Research evidence and boundaries

Aramini, Lanzi & Loiacono developed a platformer level-analysis framework that models player error and jump success and validated predictions against human players. This supports using physics-backed player-error experiments, but it does not justify transplanting its exact parameters into ASCENDENDO. https://doi.org/10.1109/GEM.2018.8516490

Mourato, Birra & Próspero dos Santos studied success prediction and player profiling in action-based challenges using more than 10,000 trials from 40 users. This supports the idea that difficulty modelling should include player behaviour rather than only geometry. https://doi.org/10.1145/2663806.2663832

Pedersen, Togelius & Yannakakis linked level features in *Super Mario Bros.* to player-experience measures such as challenge and frustration, demonstrating that objective level properties and perceived experience are related but not identical. https://doi.org/10.1109/CIG.2009.5286482

LADDER (2025) combines level characteristics, objective performance and perceived difficulty across multiple platformer datasets, reinforcing the separation between game-centric and player-centric evidence. https://doi.org/10.1109/GEM66882.2025.11155795

The ASCENDENDO Studies difficulty synthesis already requires separation between physical difficulty, perceived difficulty, player-specific performance and section-vs-level difficulty. This document extends that distinction to **chunk-vs-run difficulty and physical fall consequences**. fileciteturn27file0L2-L2

Repository evidence establishes runtime and deterministic physics behaviour, not human enjoyment or perceived frustration. Human calibration remains necessary before simulated player profiles can be called representative. fileciteturn7file0L2-L2

---

## 15. Decision

The ASCENDENDO difficulty tester should **not** define difficulty as a hand-weighted geometric score and should **not** treat an authored single-screen level as an isolated gameplay episode for final campaign evaluation.

The authoritative experimental object should be the **continuous vertical run**.

The primary observed outcome is **natural completion time**.

Failure is represented by its actual physical consequence:

```text
failure
→ landing position
→ pixel progress loss
→ recovery path
→ recovery time
→ effect on total run completion time
```

The final system should first report this multidimensional evidence and only later derive a scalar difficulty measure if empirical calibration shows that one is justified.

Until then, the goal is not to manufacture a number that says “level 20 = 73”. The goal is to build a reproducible experiment that explains **why** level 20 is demanding, **how dangerous its failures are**, **how far the player falls**, and **how much those failures cost in actual run time**.
