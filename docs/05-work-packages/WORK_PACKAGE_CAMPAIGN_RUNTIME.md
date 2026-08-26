# Work Package — Campaign runtime progress boundary

## Scope

Extract campaign progression state from `main.cpp` without introducing a monolithic application object.

## Observation

`main.cpp` still owns the ordered campaign paths, current level index, current streaming Y position, initial level loading, and next-level streaming. `CampaignLoader` already owns syntax-level loading, but not runtime progression.

## Boundary

`CampaignRuntime` owns:

- ordered campaign level paths;
- current level index;
- current streaming/spawn Y position;
- reset of campaign progress into an existing `Level` object;
- loading the next level when the streaming threshold is reached.

It does not own:

- `InputManager`;
- `PhysicsWorld`;
- `Player`;
- `GameStateMachine`;
- `RunHistory`;
- `CampaignID`;
- rendering or Vulkan resources.

## Required invariants

After reset:

```text
currentLevelIndex == 1 when a first level exists
currentSpawnY    == first level extent / returned spawn value
```

For each successful streaming step:

```text
one campaign entry consumed
currentLevelIndex increments exactly once
next level is appended at currentSpawnY
```

If no next entry exists, streaming is a no-op.

## Design choice

Keep `CampaignRuntime` as a small stateful gameplay-domain component. It consumes the paths already produced by `CampaignLoader` rather than reparsing `campaign.txt`. It operates on a caller-owned `Level`, preserving ownership of world simulation in the existing `main.cpp` composition.

This is an incremental boundary toward the later `CampaignDocument` design; it does not redefine `CampaignID`.

## Non-goals

- no `CampaignDocument` or canonical identity implementation;
- no UGC/web validation;
- no rendering changes;
- no CI workflow changes;
- no `Application` class.
