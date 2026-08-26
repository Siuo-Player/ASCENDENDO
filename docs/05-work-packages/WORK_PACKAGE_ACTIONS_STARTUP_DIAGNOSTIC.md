# Work Package — GitHub Actions startup diagnostic

## Scope

Provide a minimal workflow that distinguishes pre-job Actions startup failures from project build/test failures without modifying the canonical `Tests` workflow.

## Observation

Recent ASCENDENDO runs have shown `startup_failure` while the only job remained `queued` with no steps. Other runs have executed normally. This indicates that the canonical test workflow can be affected by a failure before project commands run.

## Decision

Keep `.github/workflows/tests.yml` unchanged. Add a separate smoke workflow with trivial jobs on GitHub-hosted runners. The smoke workflow performs no project build and no repository mutation.

Interpretation:

```text
smoke succeeds
  -> runner dispatch/control-plane path works for the repository
  -> investigate Tests workflow specifically

smoke startup-fails
  -> problem is independent of C++/Vulkan/test content
```

## Non-goals

- no replacement of the canonical Tests workflow;
- no CI concurrency changes;
- no build matrix expansion;
- no project-code changes.
