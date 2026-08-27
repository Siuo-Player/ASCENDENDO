# Decision — Windows CI dependency resolution

**Date:** 2026-08-27  
**Status:** accepted  
**Scope:** 9.6 Windows executable evidence

## Context

The ASCENDENDO repository contains GLFW 3.4 headers but does not contain the prebuilt Windows `glfw3.lib` expected by the existing Makefile at `external/glfw/lib-vc2022`.

The Windows test runner also assumes that `build\\tests.exe` already exists. Therefore a Windows CI job must make dependency acquisition and game/test build steps explicit.

## Decision

Keep the existing Makefile unchanged for this first evidence tranche. The GitHub Actions Windows job will:

1. use a Windows x64 runner;
2. provision a pinned Vulkan SDK version;
3. provision Clang/LLVM and a native Windows build environment;
4. fetch the upstream GLFW 3.4 source explicitly;
5. build GLFW Release with the Windows toolchain;
6. stage the resulting `glfw3.lib` under the exact path expected by the existing Makefile;
7. run `make clean`, `make game`, and `make tests`;
8. preserve environment and test logs as CI artifacts.

This makes dependency resolution reproducible without pretending that the repository vendors the binary library.

## Why

The goal of this tranche is evidence, not a build-system rewrite. Introducing CMake or changing the application build contract would conflate portability validation with tooling migration.

## Scope boundary

A successful run establishes observed Windows toolchain/build/test compatibility for the pinned environment. It does not establish compatibility with every Windows GPU, driver or machine configuration.

Windows ASan/UBSan is outside this tranche; Linux sanitizer coverage remains the mandatory sanitizer evidence.
