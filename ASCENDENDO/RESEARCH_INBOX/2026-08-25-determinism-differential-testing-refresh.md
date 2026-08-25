# ASCENDENDO — Determinism / differential-testing evidence refresh

Status: NEW SUPPORT / REFINEMENT

## Current upstream state checked

- `main` has the base hardening and renderer/adapter consolidation integrated.
- Historical PR #20 (`feat/9-10-render-snapshot-migration`) is **closed, not merged, superseded**.
- Therefore `RenderSnapshot` general migration must not be treated as complete.

## Relevant evidence

### Metamorphic testing of chess engines (2023)
Mendez et al. demonstrate that metamorphic relations can address the oracle problem in complex engines by checking semantically equivalent transformed inputs.

### Replication study (2025)
Martin et al. re-evaluated the chess-engine study using realistic positions, deeper searches and different versions. They found metamorphic relations substantially less effective under realistic conditions and showed that dataset, depth and implementation details affect conclusions.

## ASCENDENDO implication

Do **not** make metamorphic testing the primary correctness oracle.

Use the following evidence ladder:

```text
known deterministic replay
→ C++ ↔ Python differential trace
→ invariants / property tests
→ selected domain-specific metamorphic relations
→ human/playtest evidence
```

## First useful differential contract

Compare C++ and Python simulator traces from the same:

- initial level/model;
- player state;
- physics configuration/version;
- fixed timestep;
- action sequence;
- random seed, if any.

Compare at least:

- tick;
- action;
- position;
- velocity;
- grounded/collision flags;
- terminal/result state;
- campaign/level identity.

Report the **first divergent tick** and the first differing field.

## Critical caveat

Python agreement with C++ is not proof of physical correctness. Both implementations can share a conceptual mistake. Differential testing must therefore complement independent invariants and analytic/reference tests.

## Candidate metamorphic relations

Only introduce relations with explicit domain justification, for example:

- deterministic replay of the same action stream produces the same trace;
- replay with semantically irrelevant metadata changes leaves physics unchanged;
- equivalent serialization/deserialization preserves the playable state;
- transformations known to preserve game semantics preserve the selected result metric.

Do not invent geometric transformations that alter ASCENDENDO's one-screen/physics semantics merely to increase test count.

## Sources

- Mendez et al. (2023), *Metamorphic testing of chess engines*, Information and Software Technology 162, 107263. DOI: 10.1016/j.infsof.2023.107263.
- Martin et al. (2025), *Re-evaluating metamorphic testing of chess engines: A replication study*, Information and Software Technology 181, 107679. DOI: 10.1016/j.infsof.2025.107679.

## Evidence class

E4 for metamorphic testing as a general testing technique.
E4 for the warning that effectiveness is domain-, dataset-, depth- and implementation-dependent.
E1 for ASCENDENDO-specific effectiveness until local experiments are run.
