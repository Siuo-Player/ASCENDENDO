# ASCENDENDO — Sprint 21 — Campaign Editor integration

## Status

**OPEN — initiated 2026-09-09**

Tracking issue: **#263**  
Foundation predecessor: **Sprint 20 — CLOSED**  
Current baseline: `2b6c1e8a1f8152e224b6bf806a398bd91d0db1af`

## Why Sprint 21 exists

Sprint 20 established that the engineering foundation is sufficiently mature to stop treating every remaining property as a foundation blocker. The product now needs to cross the boundary from isolated, tested authoring models into the actual user-facing editor flow.

The immediate product requirement is the Campaign Editor: authors need to be able to see, select and reorder the campaign playlist through the real editor UI while retaining the already-established persistence, validation and failure semantics.

This is deliberately an integration sprint, not a new architecture sprint.

## Verified Foundation closure

Sprint 20 Foundation was formally closed by merged PR #256. Its closure recorded:

- semantic editor-input routing through `GameAction`/`KeyBindings`;
- preservation of existing `GameAction` numeric values;
- Linux normal tests;
- Linux ASan + UBSan;
- Windows tests;
- deterministic capture evidence;
- camera/viewport audit completed without a new contract change.

The closure explicitly states that already-deferred capabilities are not to be reintroduced as artificial Foundation blockers.

## Pre-sprint integration evidence

PR #261 was merged into `main` as commit `2b6c1e8a1f8152e224b6bf806a398bd91d0db1af` after its validation completed successfully.

It introduced the minimal null-safe `CampaignEditorDocument::selectedLevel()` seam and regression coverage for selection after load and reorder. This seam is now available to the real UI integration rather than remaining an orphaned model-only capability.

## Sprint objective

Turn the existing 9.6 campaign-editor model into a real editor-flow capability without duplicating campaign semantics or weakening established contracts.

## Work packages

### 21.1 — UI consumption boundary

Integrate the canonical `CampaignEditorDocument` into the editor's actual visual/session path.

The UI must consume the existing model rather than maintain a second playlist representation.

**Exit evidence:** the real editor flow can display the canonical playlist and the explicitly selected level.

### 21.2 — Selection and reorder interaction

Expose campaign-level selection and reorder operations through the real editor interaction boundary.

Selection identity must remain attached to the logical level, including when another level is reordered around it.

**Exit evidence:** executable regression tests cover the UI/session boundary rather than only the document object.

### 21.3 — Load/save and failure preservation

Wire existing campaign load/save semantics into the visual flow without changing `campaign.txt` authority.

A failed load must not silently destroy the currently valid playlist or selection.

**Exit evidence:** valid save/load and failed-load preservation are observable from the integrated flow and remain protected by tests.

### 21.4 — Deterministic evidence

Extend deterministic evidence only where the new UI path is meaningfully observable. Do not create captures merely to inflate coverage counts.

**Exit evidence:** applicable Linux, ASan/UBSan, Windows and deterministic-capture gates are green for the final sprint head.

### 21.5 — CI hardening disposition

Keep sanitizer-on-`main` as a separate CI workstream unless a minimal, non-duplicative change is justified by the actual current workflows.

Do not mix unrelated CI policy changes into the editor implementation.

## Product boundaries

Do not introduce in this sprint:

- new gameplay mechanics;
- physics tuning;
- free-form sprite/art editing;
- speculative campaign-format redesign;
- automatic content routing between `Levels/`, `Unused/` and `NaoValidados/`;
- online/community sharing;
- adaptive difficulty;
- deferred replay properties as Foundation work.

## Definition of done

Sprint 21 is complete when:

```text
campaign model
    ↓
real editor/session boundary
    ↓
visible selection + reorder
    ↓
canonical load/save
    ↓
failure-safe preservation
    ↓
regression coverage
    ↓
CI evidence
    ↓
canonical documentation update
```

The next sprint must then be selected from the first concrete product gap exposed by this integrated flow, not from the old Foundation backlog.
