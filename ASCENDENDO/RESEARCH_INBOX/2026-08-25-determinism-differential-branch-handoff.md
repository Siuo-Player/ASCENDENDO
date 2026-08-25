# Branch handoff — determinism / differential testing

## Scope
This is an evidence/dependency note for the next ASCENDENDO branch that continues the current 9.6 P1.9 / RenderSnapshot work. It is **not** an instruction to reopen superseded PR #20.

## Upstream state

- PR #20 (`feat/9-10-render-snapshot-migration`) is closed and not merged.
- `main` must be re-read before implementation because the active branch/PR may have changed.

## Required investigation before implementation

1. Confirm current `main` and active branch.
2. Read `docs/DEVELOPMENT_PROTOCOL.md`, `docs/PROJECT_MANAGEMENT.md`, `docs/ROADMAP.md`, `docs/BASE_ARCHITECTURE_AUDIT.md`.
3. Verify whether `RenderSnapshot` has advanced since PR #20.
4. Do not infer completion from the existence of old PRs.

## Differential-testing target

Establish a reproducible trace contract for the C++ runtime and Python simulator.

```text
same initial state
+ same level/model version
+ same physics configuration
+ same fixed timestep
+ same action sequence
(+ same seed when relevant)
        ↓
 C++ trace  ↔  Python trace
        ↓
 first divergent tick + first divergent field
```

## Minimum trace fields

- tick;
- input/action;
- position;
- velocity;
- grounded/collision state;
- terminal/result state;
- level/campaign identity.

## Testing hierarchy

```text
replay determinism
→ differential C++/Python
→ invariants/property tests
→ domain-specific metamorphic tests
→ human validation
```

Metamorphic testing must remain a supplementary oracle strategy. The 2025 replication of chess-engine metamorphic testing shows that effectiveness can fall on realistic datasets and at different depths; relations must be validated against the domain rather than assumed useful.

## Exit evidence

A branch implementing this should produce:

- machine-readable traces;
- a deterministic comparator;
- first-divergence diagnostics;
- fixed fixtures covering nominal, boundary and malformed cases;
- documented environment/toolchain;
- CI artifact or reproducible local command.

## Important negative result

If C++ and Python disagree, do not immediately patch whichever implementation is easier. Determine which model is justified by the project contract and independent physics invariants.

If they agree, do not treat agreement as proof of correctness; shared conceptual bugs remain possible.
