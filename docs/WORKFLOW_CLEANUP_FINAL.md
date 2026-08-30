# ASCENDENDO — Workflow cleanup finalization — 2026-08-31

The obsolete manual `actions-smoke.yml` diagnostic is being removed in a separate CI-only tranche. Production gates remain: Linux normal + ASan/UBSan in `tests.yml`, Windows in `windows.yml`, and deterministic capture in `deterministic-capture.yml`.
