# ASCENDENDO — world/chunk metadata contract audit

**Roadmap:** Gate 9.6 — Base Engineering Gate
**Status:** INVESTIGATED / CHARACTERIZATION PENDING
**Date:** 2026-08-28

## Question

What does the current `.lvl` → `LevelData` → `Level` → `CampaignRuntime` pipeline actually guarantee about world chunks and their metadata?

## Findings

`LevelData` is a local declarative document: name, platforms, optional spawn and optional flag. It contains no world-space offset or stream index.

`Level::appendFromData()` composes that local document into accumulated world geometry. Platform and flag Y coordinates receive `offsetY`; the next chunk boundary is advanced by `LOGICAL_HEIGHT`.

`CampaignRuntime` owns the stream cursor and the vertical boundary for the next chunk. It does not add a separate chunk identifier or metadata object.

## Metadata limitation

The current `Level` exposes a singular `hasFlag/flagBounds` pair. Each append resets `hasFlag` before processing the next chunk, so flag metadata is not retained as a per-chunk collection.

Likewise, `LevelData::spawnPosition` is parsed and stored locally but is not materialized into an explicit world-space spawn record by `appendFromData()`.

This means the current implementation is best described as:

```text
local chunk geometry
+
limited current-level metadata
+
stream cursor / vertical composition state
```

rather than a general-purpose chunk metadata model.

## Risk boundary

No production defect is asserted. The risk appears when a future requirement needs metadata to survive chunk composition independently of geometry: multiple flags, chunk IDs, per-chunk spawn points, unload/reload, world bounds, schema/version information, or editor/runtime provenance.

## Decision

Do not redesign `Level` or the file format inside Gate 9.6 without a concrete requirement. First add characterization tests for the existing local-to-world composition rules so that the current contract is explicit and protected against regression.

The stronger semantic schema/versioning question remains part of Fase 10.

## Conclusion

The residual gap is contract precision, not a demonstrated runtime failure. The correct next artifact is executable characterization of `appendFromData()` and `CampaignRuntime` progression, followed by a requirement-driven decision on explicit chunk metadata.
