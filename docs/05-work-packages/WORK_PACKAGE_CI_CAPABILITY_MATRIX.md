# Work Package — CI Capability Matrix

## Status

Planned after the core graphics ownership hardening.

## Problem

The project currently has strong local Linux/headless-Vulkan evidence, but that does not establish equivalent robustness on Windows or on different graphics/API capabilities.

## Invariant

Every CI environment must report separately:

- operating system;
- compiler/toolchain;
- Vulkan capability/driver mode;
- build result;
- unit/integration result;
- active campaign validation result;
- known environment limitations.

A failure in one capability must not be conflated with a gameplay or logic failure in another environment.

## Initial matrix

| Environment | Compiler | Graphics mode | Build | Tests | Campaign | Status |
|---|---|---|---|---|---|---|
| Linux | Clang C++20 | Headless Vulkan software driver | ✅ | ✅ | ✅ | Established |
| Windows | MSVC/MinGW | Native Vulkan | ⏳ | ⏳ | ⏳ | Required |
| Linux | Clang/GCC | Alternate Vulkan capability | ⏳ | ⏳ | ⏳ | Optional evidence |

## Rule

Do not treat the Linux headless run as proof of Windows correctness. Keep platform failures attributable to the environment while preserving the same logical test contract.

## Next implementation step

Add a Windows CI job using the repository's existing Windows test launcher. Keep it independent from the Linux/headless job so platform-specific failures remain visible.

## Out of scope

- gameplay changes;
- renderer redesign;
- performance optimisation;
- player-difficulty modelling;
- PCG evaluation metrics.
