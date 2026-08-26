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

## Source-size and modularity policy

```text
< 300 lines       normal
300–399           WARNING + subdivision plan
≥ 400             ERROR until subdivided or explicitly excepted
```

Thresholds trigger inspection, not arbitrary splitting. Prefer responsibility-cohesive boundaries and document justified exceptions.

## Architecture / WBS consistency

```text
Architecture ↔ Roadmap/WBS ↔ Branch/Work Package ↔ Implementation ↔ Tests/CI ↔ Documentation
```

Cross-boundary changes update the affected normative documents before or together with implementation.

## Branch / PR discipline

```text
main → new branch → document → implement → validate → PR → merge → delete branch → next branch
```

A new engineer or AI must be able to continue the project without the originating conversation.
