# ASCENDENDO — Windows portability evidence gap

**Date:** 2026-08-27  
**Source:** `Siuo-Player/Siuo-Player-PROJECT-STUDIES` PR #10  
**Observed implementation baseline:** `main` @ `38f589e43600c79c9abf7a787f6b4bec59506f07`  
**Role:** 9.6 Base Engineering Gate evidence

## Finding

The Windows support path is documented in the build system, but the repository does not currently provide executable CI evidence that a clean Windows runner can build the game and run the full test suite.

The current `Makefile` selects a Windows path, expects `VULKAN_SDK`, uses `llvm-ar`, compiles shaders with `glslc`, and links GLFW from `external/glfw/lib-vc2022` when the GLFW headers are present. This demonstrates a documented build path; it does not prove that the required dependency artefacts are available in a clean runner.

`Development/Tools/run_tests_windows.cmd` exists, but it assumes `build\\tests.exe` already exists and therefore is a test runner, not build/link evidence.

## Classification

```text
Windows build path documented       ✅
Windows test runner present          ✅
Windows CI build/test evidence       ❌
Windows game build/link evidence     ❌
Fresh-machine dependency evidence    ❌
```

This is an **evidence gap**, not evidence that Windows is broken.

## Minimum gate evidence

The first useful experiment is a Windows GitHub Actions runner using the existing build contract where practical:

```text
fresh Windows runner
→ toolchain
→ Vulkan SDK
→ GLFW dependency
→ glslc
→ make game
→ make tests
→ preserve logs/artifacts
```

The full test mode is required; `tests-fast` deliberately excludes graphics-related suites.

## Dependency note

The repository does not currently vendor the prebuilt Windows GLFW library path expected by the Makefile. Therefore a workflow must make the GLFW dependency source/resolution explicit rather than assuming `external/glfw/lib-vc2022` exists on the runner.

The first implementation tranche should resolve dependency acquisition reproducibly without changing application/runtime code.

## Consequence

The 9.6 Gate remains open until Windows executable evidence exists. A successful Windows run proves compatibility for that toolchain/environment; it does not prove universal compatibility across Windows GPUs and drivers.

## Non-goals

- do not label Windows support broken without reproduction;
- do not modify runtime/application code;
- do not introduce generic build abstractions merely for CI;
- do not claim fresh-machine reproducibility from a runner with preinstalled project artefacts.

## Provenance

This audit revalidates PROJECT-STUDIES PR #10 against the newer ASCENDENDO baseline. The study layer remains read-only regarding implementation; this document makes the evidence gap explicit in the implementation repository.