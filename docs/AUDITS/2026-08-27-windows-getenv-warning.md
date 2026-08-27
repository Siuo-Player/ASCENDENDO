# 2026-08-27 — Windows `getenv` warning

## Finding

The Windows portability validation for PR #88 completed the build and test suite successfully, but Clang/MSVC emitted a deprecation warning for `std::getenv("LOCALAPPDATA")` in `Game/Core/RuntimePaths.cpp`.

This is a project-owned warning, not third-party/vendor output, and the Windows CRT recommends `_dupenv_s` for this API boundary.

## Decision

Treat the warning as a compatibility-hygiene issue within the existing Windows portability work package rather than a separate cleanup project.

The Windows implementation now uses `_dupenv_s` for `LOCALAPPDATA`, preserves the existing `ASCENDENDO` suffix and fallback to the temporary directory, and leaves the POSIX environment-variable paths unchanged.

## Scope

Included:
- Windows environment-variable lookup in `RuntimePaths.cpp`.
- Preservation of existing path semantics and fallback behavior.

Not included:
- Global warning policy changes.
- Linux/macOS environment-variable rewrites.
- CI action upgrades.

## Validation

The change must be revalidated by the existing PR #88 matrix:
- Linux Tests
- Linux ASan/UBSan
- Windows build/tests/campaign

The pre-fix Windows warning is historical evidence; successful compilation alone is not considered sufficient until the new head has been checked.
