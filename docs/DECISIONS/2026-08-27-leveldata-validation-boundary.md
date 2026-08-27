# Decision — LevelData parser versus validation boundary

**Date:** 2026-08-27  
**Status:** accepted

## Context

The current `LevelData` boundary correctly centralizes declarative `.lvl` parsing/serialization, but the parser must not accidentally become the final UGC/schema contract.

The adversarial study review confirmed a concrete weakness in the previous parser: unknown directives and trailing tokens were silently accepted. That makes malformed input capable of being partially interpreted as a valid level document.

## Decision

`LevelDataIO` is responsible for **strict syntax parsing** of the currently supported textual grammar:

```text
NAME
PLATFORM x y w h
SPAWN x y
FLAG x y w h
comments / blank lines
```

Unknown directives and trailing tokens are rejected.

`LevelDataIO` is **not yet** the complete semantic/schema validator. Version envelopes, domain/range validation, migration, canonicalization and future UGC policy remain the explicit Phase 10 validation boundary.

## Why

This separates three concerns without prematurely introducing a schema system:

```text
text syntax
    ↓
LevelDataIO
    ↓
LevelData
    ↓
semantic/schema validator (Phase 10)
    ↓
canonical / importable content
```

The current parser therefore has a useful fail-closed syntax boundary while preserving the planned later validation architecture.

## Evidence

The parser now rejects:

- an unknown record type;
- a recognized record containing additional tokens.

Existing historical `NAME/PLATFORM/FLAG` files and optional `SPAWN` remain supported.

## Non-goals

- no `VERSION` field yet;
- no migration layer yet;
- no UGC import/export implementation;
- no Level/Chunk/World semantic rewrite;
- no change to campaign streaming semantics.

## Follow-up

Phase 10 must add an explicit schema envelope and semantic validation contract before external/imported `.lvl` content is treated as trusted playable data. The validator must distinguish malformed syntax, unknown schema versions, missing mandatory fields and semantically invalid geometry.
