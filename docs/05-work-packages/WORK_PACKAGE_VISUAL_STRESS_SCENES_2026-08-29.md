# Work Package — Visual Stress Scenes / Render Captures

## Context

The current presentation/camera validation has already established the numerical contracts needed for `Camera::follow()`, `worldToNDC()` and viewport letterboxing through #170 and #172. The fixed canonical levels were also aligned to the 16px visual module, and the procedural generator now emits the same module dimensions (#174).

The remaining uncertainty is visual rather than algebraic: the existing automated tests do not prove that the player, landing surfaces, route, hazards, goals and future camera geometry remain readable together at different viewport shapes.

## Objective

Create a small deterministic set of representative stress scenes and render captures that can be compared across implementation changes without introducing subjective gameplay claims into unit tests.

## Acceptance properties

```text
P0 player silhouette remains clearly identifiable
P1 platform / landing edge remains clearly readable
P2 hazard / checkpoint / goal remain distinguishable when present
reachable route remains visually legible
camera edge/future geometry remains visible enough for navigation
letterbox regions do not overlap the logical scene
large and small supported viewports preserve composition
background/props remain subordinate to gameplay-critical elements
```

These are visual acceptance properties, not claims that one art style or one camera feel is optimal.

## Stress scene set

### Scene A — baseline gameplay

Use the first canonical level with the player near a normal landing sequence.

Purpose:
- establish baseline player/platform contrast;
- verify platform contact-edge readability;
- provide a stable reference capture.

### Scene B — dense vertical composition

Use a section containing several vertically separated platforms with the camera following the player upward.

Purpose:
- check future geometry visibility;
- expose camera lag/offset composition problems;
- verify that repeated platforms do not merge perceptually.

### Scene C — edge-of-viewport

Place the player and the next reachable platform close to the logical viewport boundaries while preserving valid gameplay geometry.

Purpose:
- inspect crop/letterbox interaction;
- ensure the player or landing surface is not visually clipped by composition mistakes;
- verify lower-bound camera behavior visually.

### Scene D — hazard / goal salience

Use a scene containing a flag/goal or future hazard representation together with nearby platforms.

Purpose:
- verify hierarchy between gameplay-critical elements and decoration;
- verify that the goal is distinguishable without becoming visually louder than the player during ordinary play.

### Scene E — prop/background density

Use the curated visual vocabulary already researched under `Research/CC0_PROPS_REFERENCE_AND_TEST_PLAN.md` without making those external assets gameplay semantics.

Purpose:
- compare sparse versus denser presentation;
- verify background depth without reducing P0/P1 readability;
- validate that props remain presentation/content data.

## Viewport matrix

At minimum capture representative aspect ratios covering:

```text
16:9   normal/wide
4:3    narrower
21:9   ultrawide
```

For each viewport, record:

```text
window extent
logical viewport extent
letterbox offsets
camera position
player bounds
visible platform bounds
scene identifier
```

Do not compare raw pixel output between different viewport sizes as though they were identical scenes; compare the invariants and composition rules appropriate to each viewport.

## Capture requirements

The capture mechanism should be deterministic and should not require arbitrary timing or manual camera manipulation.

Each capture must be traceable to:

```text
scene id
source level/content revision
viewport size
camera state
capture frame/tick
commit SHA
```

Images are evidence for visual inspection. They are not unit-test oracles by themselves.

## Background / prop rule

The project visual hierarchy is:

```text
P0 PLAYER
P1 PLATFORM / LANDING EDGE
P2 HAZARD / CHECKPOINT / GOAL
P3 LANDMARK
P4 NEAR DECORATION
P5 MID/FAR ENVIRONMENT
P6 SKY / ATMOSPHERE
```

External props are presentation assets only. They must not mutate collision geometry or simulation values. The existing CC0 research remains a source-selection reference; it does not by itself approve an asset for final game use.

## Validation method

The first implementation tranche should prefer:

```text
existing renderer/composition path
+
small deterministic capture harness
+
fixed scene fixtures
+
recorded metadata
```

Avoid adding a generic screenshot framework or asset-management system solely for this work package.

## Exit criteria

```text
representative stress scenes defined
+ deterministic capture path
+ viewport matrix exercised
+ player/platform/route readability reviewed
+ background/prop hierarchy reviewed
+ evidence recorded with commit and viewport metadata
+ no gameplay/physics changes required
```

A failure in an objective presentation property may justify a focused renderer/camera fix in a separate implementation tranche. Subjective questions such as ideal camera lag, look-ahead, dead zones and difficulty remain outside this package and belong to later playtesting/product experiments.

## Next step

After the captures establish a stable visual baseline, proceed to the human playtesting protocol. Do not begin difficulty/progression calibration from visual captures alone.
