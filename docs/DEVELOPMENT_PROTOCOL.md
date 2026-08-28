# Development Continuity Protocol

> Repository knowledge is operational state. Any human or AI must be able to continue the project without access to the originating conversation.

## Mandatory sequence

For any non-trivial discovery, decision, design choice, refactoring choice, workflow change, dependency change, or validation finding:

```text
INVESTIGATE
    ↓
DOCUMENT THE FINDING IMMEDIATELY
    ↓
UPDATE ARCHITECTURE / ROADMAP / DEBT / WORK PACKAGE
    ↓
IMPLEMENT
    ↓
TEST / VALIDATE
    ↓
DOCUMENT RESULT + FAILURES + NEXT DECISION
```

This is a process requirement, not a style preference. Private chat memory is never the authoritative record of an architectural decision.

## Documentation hierarchy

The repository is the source of truth in this order:

```text
PROJECT_MANAGEMENT / DEVELOPMENT_PROTOCOL
            ↓
      PRODUCT_DECISIONS
            ↓
          ROADMAP
            ↓
 ARCHITECTURE / TECH_DEBT
            ↓
       BRANCH_PLAN
            ↓
      WORK PACKAGE / PR
            ↓
 implementation / tests / validation
```

When a lower document contradicts a higher one, resolve the documentation contradiction before implementing new work.

## Discovery-before-change

When investigation reveals a problem, do not patch first and document afterwards. First record the evidence and the chosen response in the appropriate project document.

For every material decision:

```text
Context:
Problem:
Observation / evidence:
Decision:
Alternatives considered:
Why this option:
Dependencies affected:
Consumers affected:
Risks / trade-offs:
Validation required:
Revisit / removal condition:
```

This applies when a finding changes architecture/module boundaries, interfaces/data ownership, roadmap order, work-package scope, dependencies/consumers, validation/CI, technical or architectural debt, maintainability thresholds, compatibility layers, tooling/workflows, or a materially rejected alternative.

## Work package contract

Every work package must be traceable to a subsystem and roadmap block and answer:

```text
Objective:
Scope:
Out of scope:
Depends on:
Produces for:
Consumers:
Tests:
Tools:
Documentation:
Risks:
Decisions:
Alternatives:
Validation:
Exit criteria:
Debt created:
Next dependent work package:
```

A PR integrates a work package; it does not replace the planning record.

## Dependency protocol

Dependencies are treated as explicit engineering objects. For every relevant change inspect:

```text
technical dependency
        +
consumers
        +
tests
        +
tools
        +
documentation
        +
coordination needs
```

Reducing code coupling can still leave coordination requirements between components, branches, tests, tools and documents. A dependency is therefore not considered managed merely because it compiles.

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

The distinction between **likely** and **confirmed** is mandatory. Never infer a compiler/test failure from a generic failed step when the diagnostic is unavailable. In that case the cause remains **unknown**.

### Historical CI evidence — 2026-08-25

Run #281 (`32879936455`) is known to have failed in the aggregate job/step named **Build and run tests in virtual X display**. The source-size gate did not cause this run to fail.

The detailed failure diagnostic was not recovered through the available GitHub interface at the time of that historical update. Therefore its confirmed cause remains:

```text
Classification: CI build/test failure
Confirmed cause: UNKNOWN
```

No code change may be attributed to a more specific cause without diagnostic evidence.

### Current sanitizer evidence — 2026-08-27

PR #70 (`ci(9.6): add production ASan/UBSan coverage`) added an independent Linux/Clang sanitizer workflow. Its implementation commit `c450f62cd5f3bac4f37e768a34ec17bbcb4a08cd` produced successful `Tests` run `33025107053` and successful `Sanitizers` run `33025107141`.

The later documentation-only follow-up was integrated without changing runtime/build source. Windows CI and hardware/capability coverage remain unvalidated.

### Historical source-size incident

Run #251 reported:

```text
ERROR   20.21 KiB   430 lines  Game/Graphics/FontRenderer.cpp
WARNING 14.36 KiB   332 lines  Game/Graphics/SpriteRenderer.cpp
WARNING 11.37 KiB   305 lines  Tests/Unit/test_keybindings.cpp
WARNING 12.61 KiB   326 lines  Tests/Unit/test_level.cpp
WARNING 13.83 KiB   330 lines  main.cpp
```

That result is evidence of a maintainability problem in the affected files; it is not evidence that GitHub Actions infrastructure was broken.

## Source-size and modularity policy

The current source-size policy is the one documented by `docs/CODE_SIZE.md` and implemented by `Development/Tools/check_source_sizes.py`:

```text
< 40 KiB
    normal

40–47.99 KiB
    WARNING
    explicit assessment of partitioning before accumulating new responsibilities

≥ 48 KiB
    ERROR
    CI blocks until coherent partitioning or an explicit documented exception
```

The metric is physical UTF-8 file size in KiB. LOC is diagnostic only. Size is a decision trigger for maintainability review, not a demand for arbitrary splitting.

An exception must document:

```text
why the file remains cohesive
important coupling/dependencies
alternatives considered
why splitting is not appropriate now
when to reconsider
```

Prefer responsibility-cohesive boundaries. Never create artificial files such as `main1.cpp`, `main2.cpp`, `main3.cpp` solely to reduce metrics.

`Development/Tools/check_source_sizes.py` is the canonical checker. The repository must not introduce a second implementation of the same policy.

## Architecture / WBS consistency

Architecture and work breakdown must evolve together:

```text
Architecture
    ↕
Roadmap / WBS
    ↕
Branch / Work Package
    ↕
Implementation
    ↕
Tests / CI
    ↕
Documentation result
```

A change that crosses architectural boundaries must update the affected architecture, roadmap, debt and work-package records before or together with implementation.

## Current renderer boundary

The legacy `Renderer.cpp/.h` implementation is absent from `main`. The implemented presentation direction is:

```text
Runtime / domain
      ↓
application composition
      ↓
render data / RenderSnapshot
      ↓
RendererFacade
      ↓
World / UI / Editor passes
```

On current `main`, the world/player path receives `gfx::RenderSnapshot`, and the editor path receives `logic::EditorRenderSnapshot` materialized by composition. `WorldRenderer` does not receive `logic::Player`/`logic::Level` directly, and `RendererFacade` does not retain or query `logic::EditorSession`.

PR #129 established the first world/player snapshot boundary. PR #132 removed the remaining `RendererFacade → EditorSession` dependency. These are integrated implementation records and should be used with the current architecture/work-package documents rather than the historical PR #20.

## Branch / PR discipline

Normal development order:

```text
main
 ↓
new branch
 ↓
document / update work package
 ↓
implement
 ↓
validate
 ↓
document result
 ↓
PR
 ↓
merge
 ↓
close branch
 ↓
new branch from updated main
```

Documentation committed directly to `main` by mistake is treated as a process defect. Do not rewrite history merely to hide it; track and correct the process through normal repository changes unless a deliberate history rewrite is explicitly authorized.

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

## Knowledge continuity contract

A new engineer or AI must be able to answer from repository documents alone:

1. What are the current architectural boundaries?
2. Why do they exist?
3. What is the current roadmap and next work package?
4. What dependencies must be respected?
5. Which temporary decisions/debts remain?
6. How is the project actually validated?
7. Which recent failures/discoveries changed the plan?

If answering requires an old conversation, the documentation is incomplete.

## Research basis

This protocol follows the project's research-repository principle that project knowledge must remain reproducible, traceable and sufficient for continuation without the originating conversation. Relevant foundations include WBS, architectural decision/rationale documentation, socio-technical congruence, coordination breakdowns, technical/architectural debt and code-smell research.

Key references already catalogued by the project include:

- Tausworthe, *The Work Breakdown Structure in Software Project Management* (1979), DOI `10.1016/0164-1212(79)90018-9`.
- van Heesch et al., *A documentation framework for architecture decisions* (2012), DOI `10.1016/j.jss.2011.10.017`.
- Tofan et al., *An expert survey on kinds, influence factors and documentation of design decisions in practice* (2014).
- Capilla et al., *10 years of software architecture knowledge management: Practice and future* (2016), DOI `10.1016/j.jss.2015.08.054`.
- Cataldo, Herbsleb & Carley, *Socio-technical congruence* (ESEM 2008), DOI `10.1145/1414004.1414008`.
- Cataldo & Herbsleb, *Coordination Breakdowns and Their Impact on Development Productivity and Software Failures* (IEEE TSE, 2013), DOI `10.1109/TSE.2012.32`.
- Abbes et al., *Code smells and their collocations* (2018), DOI `10.1016/j.jss.2018.05.057`.

For provenance and paper-level synthesis, use the project's research repository rather than copying conclusions into this protocol without traceability.
