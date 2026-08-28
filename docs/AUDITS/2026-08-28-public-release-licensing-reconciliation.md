# ASCENDENDO — Public Release Licensing Reconciliation

**Data:** 2026-08-28  
**Tipo:** auditoria documental / public-release  
**Estado:** implemented, validation pending

## Finding

A nova política dos `PROJECT-STUDIES` fixa MIT para o código próprio do ASCENDENDO, mas o repositório não possuía um `LICENSE` na raiz. O mesmo estudo exige preservar a separação entre código próprio e material de terceiros.

## Evidence

Current repository inspection confirmed:

- no root `LICENSE` file;
- vendored `doctest` at `external/doctest/doctest.h`;
- vendored `stb_image.h` and `stb_truetype.h` at `external/stb/`;
- vendored GLFW headers at `external/glfw/`;
- `Game/Assets/Fonts/UIFont.ttf` is present;
- README identifies doctest, stb, GLFW and Orbitron as third-party material.

Upstream license verification was performed against the public upstream repositories:

- doctest: MIT;
- stb: MIT or Public Domain, per upstream license file;
- GLFW: its own permissive GLFW license;
- Orbitron: SIL Open Font License 1.1.

## Decision

Add a root MIT license for ASCENDENDO-owned code and a dedicated third-party notice file documenting the known vendored code and font attribution boundaries.

Do not copy the root MIT license semantics onto third-party material.

Do not claim this audit is a complete historical provenance audit for every asset in the repository.

## Implementation

- add `LICENSE` for ASCENDENDO-owned source;
- add `docs/THIRD_PARTY_NOTICES.md` with current known third-party components, versions/source references, and license boundaries.

## Validation required before merge

- root license file exists and contains only the project license;
- third-party notice paths match files actually present;
- no existing third-party license notices are removed or modified;
- normal Linux/Clang, ASan/UBSan, and Windows checks remain green;
- PR remains separate from Gate 9.6 runtime/physics changes.

## Remaining limitations

`HISTORICAL_SECRET_AUDIT` and exhaustive asset provenance remain separate work. This change establishes the current public-release licensing boundary; it does not certify every historical repository object.
