# Windows Evidence Scope Reconciliation — 2026-08-27

## Decision

The previous PR #83 mixed three classes of work: Windows evidence, Windows-specific compatibility fixes, and unrelated cross-platform CI hardening. This tranche separates them.

```text
Windows portability evidence
    = Windows workflow
    + Windows dependency setup
    + Windows-specific compatibility fixes proven by Windows failures
    + evidence required to classify the Gate

CI/toolchain hardening
    = separate follow-up work package
```

## Windows tranche

The scoped tranche keeps only:

- `.github/workflows/windows.yml`;
- Windows GLFW acquisition/build required by the existing Makefile;
- Windows Vulkan software-driver provisioning and verification;
- `Development/Tools/run_tests_windows.cmd` path correction;
- `Game/Graphics/PresentationRuntime.cpp` path conversion required by the observed Windows build failure;
- Windows-specific Clang/MSVC CRT configuration required by the observed link failures.

## Excluded from this tranche

- Linux workflow modifications;
- Linux sanitizer workflow modifications;
- global warning policy changes;
- global third-party include policy changes;
- general Actions modernization unrelated to the Windows path;
- warning cleanup not required for Windows execution.

## Gate interpretation

A green run proves observed compatibility for the selected Windows runner and toolchain. It does not prove universal Windows compatibility and does not close unrelated Gate 9.6 properties.
