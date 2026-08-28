# CI workflow duplicate-trigger reduction

## Finding

`tests.yml` and `windows.yml` previously ran full validation on both:

- every branch `push` (`'**'`); and
- `pull_request` targeting `main`.

For a branch with an open PR, every push could therefore schedule a second full run in addition to the PR validation.

## Decision

Keep full validation on:

- `pull_request` → `main`; and
- direct `push` → `main`.

Remove the all-branches `push` trigger from these required validation workflows.

This does not change:

- required check names;
- test contents;
- sanitizer coverage;
- Windows coverage;
- manual smoke workflow;
- permissions;
- branch protection policy.

## Rationale

The PR event is the authoritative validation path for branch changes proposed for `main`. A branch push is still useful for direct updates to `main`, but running the same full validation independently for every feature-branch push duplicates work and runner consumption without adding distinct evidence.

The optimization is therefore trigger-level deduplication rather than weakening validation.

## Boundary

This decision intentionally does not add caching of build outputs or dependencies yet. Such caching should only be introduced after measuring whether it reduces wall time enough to justify cache invalidation and correctness complexity.
