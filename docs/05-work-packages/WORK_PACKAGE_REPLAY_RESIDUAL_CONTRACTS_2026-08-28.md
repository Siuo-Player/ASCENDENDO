# Work Package — residual replay contracts

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsistema:** `Logic / ReplayManager + GameSession`
**Work Package:** `9.6 residual replay contracts`
**Branch:** `docs/9-6-replay-contracts-20260828-v2`

## Objetivo

Separar claramente as propriedades de replay já demonstradas das três capacidades ainda não provadas: live-input frame-rate independence, terminal/result replay completo e persistence/serialization.

## Estado já provado

A arquitetura atual grava `TickInput` por tick e o playback devolve a mesma sequência sem depender do agrupamento externo de frames. Isto suporta replay semântico ao nível do tick.

Não usar essa evidência para inferir automaticamente que input ao vivo entre frames diferentes produz a mesma trajetória, que o resultado terminal completo de uma `GameSession` é reproduzível por replay, ou que um replay pode ser persistido e recarregado.

## Residuais

### 1. Live-input frame-rate independence

O replay trabalha sobre `TickInput`, mas o input normal é recolhido através de `InputManager` antes de `GameSession::update()`. Continua por provar uma propriedade mais forte sobre duas taxas de frame externas que produzam a mesma sequência semântica de ticks.

### 2. Terminal/result replay

`ReplayManager` guarda estado de `Player` e accumulator do mundo, além de `TickInput`. Não constitui ainda um recording explícito de todos os outputs/decisions necessários para afirmar que uma `GameSession` completa termina com o mesmo resultado observável, incluindo campaign completion/result metadata.

### 3. Persistence

O replay atual é memória-residente: `m_stateHistory`, `m_inputHistory`, `m_playbackIndex` e slots em estruturas C++. Não existe neste componente um formato persistente, schema ou round-trip de gravação/recarregamento.

## Decisão

Manter estas três questões separadas. Não expandir `ReplayManager` antes de existir um requisito concreto para cada propriedade.

A próxima evidência de maior valor é characterization de terminal/result replay e uma prova controlada de live-input frame-rate independence. Persistence deve seguir como capacidade própria e não como efeito colateral.

## Definition of Done

- [x] propriedades já demonstradas separadas das não demonstradas;
- [x] três gaps residuais identificados;
- [x] ausência de uma claim universal de replay registada;
- [x] ordem de evidência futura definida;
- [ ] teste de live-input frame-rate independence;
- [ ] teste de terminal/result replay;
- [ ] decisão específica sobre persistence format.

## Evidência

- `Game/Logic/ReplayManager.h`;
- `Game/Logic/ReplayManager.cpp`;
- `Game/Logic/GameSession.*`;
- PR #90 — TickInput alignment;
- PR #95 — replay reconciliation;
- `docs/ROADMAP.md`;
- `docs/TECH_DEBT.md`.

## Resultado

**Status:** `investigated / characterization pending`