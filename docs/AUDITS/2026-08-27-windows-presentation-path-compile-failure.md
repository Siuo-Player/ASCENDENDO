# ASCENDENDO — Windows PresentationRuntime path compile failure

**Date:** 2026-08-27  
**Source:** Windows CI run #8 for `fix/9-6-windows-portability-evidence`  
**Head observed:** `e32fd4c8296af7cc994022f2f5349d8c5f5d08bc`  
**Role:** 9.6 Base Engineering Gate evidence

## Finding

After resolving the Windows runner toolchain to Clang 20.1.8, the game compiled far enough to expose a source-level interface mismatch in `PresentationRuntime`.

`PresentationRuntime::init()` receives `playerSpritePath` as `std::filesystem::path`, while `SpriteRenderer::init()` accepts the asset path as `const std::string&`. The Windows build therefore rejected the call without an implicit conversion at that boundary.

## Classification

This is a **source/API type mismatch exposed by Windows compilation**, not evidence of a runtime Vulkan/GLFW defect.

## Correction

Keep the existing `SpriteRenderer` string contract for this tranche and convert the already-resolved filesystem path explicitly at the boundary using `playerSpritePath.string()`.

No new abstraction, ownership change, or RoadMap change is required.

## Evidence boundary

The correction must be followed by a clean Windows build and then the full Windows test/campaign sequence. Only that execution can establish whether additional Windows source or linkage incompatibilities remain.

## Provenance

The finding was taken from the observable CI log and recorded before the source change was applied.
