# ASCENDENDO — Workflow topology consolidation — 2026-08-31

## Basis

The live workflow-governance study in `Siuo-Player-PROJECT-STUDIES/ASCENDENDO/2026-08-30-workflow-cleanup-live-reconciliation.md` recommends:

```text
tests.yml + sanitizers.yml
→ one CI entrypoint with two jobs

windows.yml
→ remain independent

deterministic-capture.yml
→ remain independent evidence workflow

actions-smoke.yml
→ temporary diagnostic only
```

The current ASCENDENDO state was checked against `main` before this change.

## Change

`sanitizers.yml` is being consolidated into `tests.yml` as the `asan-ubsan` job.

The required job name remains exactly:

`Linux / Clang / ASan + UBSan / Headless Vulkan`

This preserves the existing required-check identity while removing a second Linux workflow entrypoint.

The sanitizer job keeps its previous execution semantics: it runs for pull requests and manual dispatch, but is skipped on pushes to `main`, matching the previous `sanitizers.yml` trigger topology.

## Intentionally unchanged

- Linux normal test job name and behaviour;
- Windows workflow;
- deterministic capture workflow;
- manual runner-smoke diagnostic;
- sanitizer flags and validation commands;
- campaign validation.

## Acceptance criteria

1. The pull-request check identity for Linux normal remains available.
2. The pull-request check identity for ASan/UBSan remains available.
3. Windows remains an independent workflow.
4. Deterministic capture remains an independent evidence workflow.
5. No required check disappears because of a workflow-file rename/deletion.
6. The consolidated workflow passes Linux normal, ASan/UBSan and campaign validation on the PR head.

## Follow-up

After this PR is validated, evaluate `actions-smoke.yml` independently. It is a manual diagnostic created during an earlier runner-startup incident and should only be removed once that diagnostic need is no longer useful.
