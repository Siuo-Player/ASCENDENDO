# ASCENDENDO — Windows Clang/MSVC STL compatibility finding

**Date:** 2026-08-27  
**Source:** Windows CI run for `fix/9-6-windows-portability-evidence`  
**Head:** `e21d15b64c84b39017dbdf3dce46e69f8dbcf1a6`  
**Role:** 9.6 Base Engineering Gate evidence

## Finding

The Windows runner successfully resolved the Vulkan SDK, GLFW 3.4, Visual Studio 18/2026 environment, source-size checks, and the GLFW dependency build. The first project compilation then failed before linking because the MSVC STL emitted `STL1000` and explicitly required Clang 20 or newer, while the workflow had installed Clang 19.1.7.

This is a **toolchain compatibility finding**, not evidence of an ASCENDENDO runtime defect.

## Evidence

Observed runner facts:

```text
Visual Studio: 18.0
MSVC tools:    14.51.36231
Clang:         19.1.7
CMake:         4.4.2
GNU Make:      4.4.1
```

Observed compiler diagnostic:

```text
STL1000: Unexpected compiler version, expected Clang 20 or newer.
```

## Correction

The Windows workflow is pinned to LLVM/Clang `20.1.8` for x64. The purpose is to match the compiler compatibility contract exposed by the observed MSVC STL on the current `windows-2025-vs2026` runner.

## Gate interpretation

The Windows gate remains open until the corrected toolchain demonstrates:

```text
make game      ✅ required
make tests     ✅ required
campaign       ✅ required
```

A green run establishes compatibility for the pinned runner/toolchain combination. It does not establish universal Windows compatibility.
