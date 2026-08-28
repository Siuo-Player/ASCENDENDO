# Level clear finding

`Level::clear()` deve limpar geometria e metadata do modelo de mundo.

Current production behavior clears only `m_platforms`, leaving `name`, `hasFlag` and `flagBounds` stale after a failed/reset load.
