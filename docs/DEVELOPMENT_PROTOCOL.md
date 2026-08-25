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

### Current CI evidence — 2026-08-25

Run #281 (`32879936455`) is known to have failed in the aggregate job/step named **Build and run tests in virtual X display**. The source-size gate did not cause this run to fail.

The later build failure received during the FontRenderer decomposition work package contained this concrete compiler diagnostic:

```text
Game/Graphics/SpriteRenderer.cpp:52:21: error: member access into incomplete type 'VulkanContext'
SpriteRenderer.h:20:7: note: forward declaration of 'gfx::VulkanContext'
make: *** [Makefile:229: build/game/Game/Graphics/SpriteRenderer.o] Error 1
```

**Classification:** compiler build failure.

**Likely cause:** `SpriteRenderer.cpp` was compiled in a state where the complete `VulkanContext` definition was not visible at the point of `ctx->isInitialized()`.

**Current source evidence:** the current branch version of `SpriteRenderer.cpp` already includes `Graphics/VulkanContext.h` directly, so the corrective include is present in the source being validated now. The exact reason the failing CI state did not observe the complete type cannot be reconstructed solely from the supplied tail; the current commit therefore still requires CI validation rather than assuming success.

**Validation required:** a fresh CI build of the current branch must compile `SpriteRenderer.cpp` and complete the full test/campaign pipeline.

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

The project uses **encoded file size** as the primary source-size gate because the historical CI signal already measures KiB and because the repository wants a stable physical-size limit independent of comments/blank-line formatting. File size is still a heuristic: it triggers architectural/cohesion analysis and never justifies arbitrary splitting.

For C/C++ files:

```text
< 30 KiB
    normal

30–36 KiB
    WARNING
    explicit subdivision plan required

> 36 KiB
    ERROR
    CI blocks until subdivided or an explicit exception is documented
```

A file may have many lines without being a problem, and a compact file may still have poor cohesion. The byte/size gate is therefore a guardrail, not a substitute for architectural review.

An exception above the hard limit must document:

```text
why the file remains cohesive
coupling/dependencies relevant to the decision
alternatives considered
cost/risk of splitting now
condition for reconsideration
```

The exception belongs to the work package/PR that keeps the file large.

### Historical wording correction

An earlier draft of this protocol used a 300/400-line policy. That wording is superseded by the encoded-size policy above. The active checker is the authority for enforcement, and the documentation must remain aligned with it.

## Source-size work packages

The current engineering order remains:

```text
FontRenderer.cpp
    → decompose by cohesive responsibility

SpriteRenderer.cpp
    → investigate cohesion after FontRenderer

main.cpp
    → continue architectural extraction, not artificial splitting

large test files
    → split by behavior/domain only where cohesion supports it
```

Consult `docs/CODE_SIZE.md` and `docs/PROJECT_MANAGEMENT.md` before beginning each split.
