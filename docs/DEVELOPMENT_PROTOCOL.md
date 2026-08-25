# Development Continuity Protocol

> Repository knowledge is operational state. Any human or AI must be able to continue the project without access to the originating conversation.

## Mandatory sequence

For any non-trivial discovery, decision, design choice, refactoring choice, workflow change, or validation finding:

```text
OBSERVE / INVESTIGATE
        ↓
DOCUMENT THE FINDING IMMEDIATELY
        ↓
UPDATE ARCHITECTURE / ROADMAP / DEBT / WORK PACKAGE
        ↓
IMPLEMENT
        ↓
VALIDATE
        ↓
DOCUMENT RESULT + FAILURES + FOLLOW-UP
```

Do not rely on private memory or chat history to explain the current implementation.

## Discovery-before-change

When investigation reveals a problem, do not patch first and document afterwards. The repository must first contain the observation/evidence and the chosen response.

For every material decision record:

```text
Context:
Problem:
Observation / evidence:
Decision:
Alternatives considered:
Why this option:
Dependencies affected:
Risks / trade-offs:
Validation required:
Revisit / removal condition:
```

Document before implementation when a finding changes architecture/module boundaries, interfaces/data ownership, roadmap order, work-package scope, dependencies/consumers, validation/CI, technical or architectural debt, maintainability thresholds, compatibility layers, tooling/workflows, or a materially rejected alternative.

## CI failure protocol

Before changing code because CI failed, record:

```text
Run:
Commit / PR:
Failing job:
Failing step:
Observed evidence:
Classification:
Likely cause:
Confirmed cause:
Fix:
Validation after fix:
```

Never infer a compiler/test failure from a generic failed step when the diagnostic is unavailable. Mark the cause unknown until evidence is obtained.

## Source-size and modularity

File size is a maintainability signal, not an architectural objective. Thresholds trigger inspection; they do not justify arbitrary splitting.

| Lines | Status | Action |
|---:|---|---|
| < 300 | normal | inspect cohesion/coupling as usual |
| 300–399 | warning | create/update a split/refactoring plan |
| ≥ 400 | error | CI blocks until subdivided or an explicit exception is documented |

Exceptions must explain why the responsibility remains cohesive, relevant coupling/dependencies, alternatives considered, why splitting is not appropriate now, and a revisit condition.

Prefer splitting by cohesive responsibility, not arbitrary line ranges.

## Knowledge continuity contract

A new engineer or AI must be able to answer from repository documents alone:

1. What are the current architectural boundaries?
2. Why do they exist?
3. What is the current roadmap and next work package?
4. What dependencies must be respected?
5. Which temporary decisions/debts remain?
6. How is the project actually validated?
7. Which recent failures/discoveries changed the plan?

If answering requires an old conversation, documentation is incomplete.

## End-of-work record

```text
Changed:
Decisions added/updated:
Discoveries:
Tests run:
CI result:
Failures and disposition:
Remaining debt:
Next dependent work package:
```

Update architecture, roadmap, debt, branch and validation documents whenever their truth changes.

## Current CI incident — 2026-08-25

The source-size gate correctly reported:

```text
ERROR   20.21 KiB / 430 lines  Game/Graphics/FontRenderer.cpp
WARNING 14.36 KiB / 332 lines  Game/Graphics/SpriteRenderer.cpp
WARNING 11.37 KiB / 305 lines  Tests/Unit/test_keybindings.cpp
WARNING 12.61 KiB / 326 lines  Tests/Unit/test_level.cpp
WARNING 13.83 KiB / 330 lines  main.cpp
```

This is a real maintainability gate failure, not evidence that GitHub Actions infrastructure is broken. The response is to document responsibility boundaries and subdivide affected files by cohesion.

## Research basis

This protocol is informed by the project's research repository `Siuo-Player/Siuo-Player-PROJECT-STUDIES`, especially its AI continuation contract, evidence/provenance policy and document subdivision rules.

Key references:

- Tausworthe, *The Work Breakdown Structure in Software Project Management* (1979), DOI `10.1016/0164-1212(79)90018-9`.
- van Heesch et al., *A documentation framework for architecture decisions* (2012), DOI `10.1016/j.jss.2011.10.017`.
- Tofan et al., *An expert survey on kinds, influence factors and documentation of design decisions in practice* (2014).
- Capilla et al., *10 years of software architecture knowledge management: Practice and future* (2016), DOI `10.1016/j.jss.2015.08.054`.
- Cataldo, Herbsleb & Carley, *Socio-technical congruence* (ESEM 2008), DOI `10.1145/1414004.1414008`.
- Cataldo & Herbsleb, *Coordination Breakdowns and Their Impact on Development Productivity and Software Failures* (IEEE TSE, 2013), DOI `10.1109/TSE.2012.32`.
- Abbes et al., *Code smells and their collocations: A large-scale experiment on open-source systems* (2018), DOI `10.1016/j.jss.2018.05.057`.

Reference repository: `https://github.com/Siuo-Player/Siuo-Player-PROJECT-STUDIES`
