# ASCENDENDO — Work Package: remove obsolete Actions smoke diagnostic

## Property

`actions-smoke.yml` is a temporary manual diagnostic, not a production gate.

## Evidence

The workflow-governance study classifies it as temporary diagnostic. The repository currently has no open issue for the earlier runner-startup incident, while Linux normal/ASan, Windows, and deterministic capture have dedicated validation paths.

## Scope

- remove `.github/workflows/actions-smoke.yml`;
- preserve all required production checks and their names;
- preserve Windows and deterministic capture workflows;
- make no runtime changes.

## Validation

The PR must show Linux normal, ASan/UBSan, Windows, and deterministic capture still execute successfully after removal.
