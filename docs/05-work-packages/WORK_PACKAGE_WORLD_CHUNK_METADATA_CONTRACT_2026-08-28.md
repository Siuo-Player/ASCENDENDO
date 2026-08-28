# Work Package — world/chunk metadata contract

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsistema:** `Logic / Level + CampaignRuntime`
**Work Package:** `9.6 world/chunk metadata contract`
**Branch:** `docs/9-6-world-chunk-metadata-contract-20260828`

## Objetivo

Caracterizar a fronteira entre um ficheiro `.lvl` local (`LevelData`) e a composição desse documento no mundo (`Level`/`CampaignRuntime`), identificando quais metadados pertencem ao chunk e quais são derivados do stream global.

## Escopo

### Inclui
- estrutura declarativa de `LevelData`;
- transformação local→world em `Level::appendFromData()`;
- política de `offsetY` e altura lógica fixa;
- semântica de `spawnPosition`/`flag` e respetiva preservação;
- progressão de `CampaignRuntime::m_nextLevelIndex` e `m_spawnY`.

### Não inclui
- alteração do formato `.lvl`;
- schema/versioning completo (Fase 10);
- streaming assíncrono;
- gameplay changes;
- renderer changes.

## Observações

`LevelData` é explicitamente declarativo e contém plataformas, spawn opcional e flag opcional. Não contém estado de streaming, Vulkan, GLFW ou coordenadas de mundo globais.

`CampaignRuntime` mantém o estado global do stream:

```text
m_nextLevelIndex  → próximo ficheiro da campanha
m_spawnY          → limite vertical do próximo chunk
```

`Level::appendFromData()` recebe `offsetY`, desloca plataformas e flag para coordenadas de mundo e devolve `offsetY + LOGICAL_HEIGHT` para o chunk seguinte.

Assim, a composição atual é:

```text
LevelData(.lvl)
    │
    │ conteúdo local
    ▼
appendFromData(offsetY)
    │
    ├── plataformas → world Y = local Y + offsetY
    ├── flag        → world Y = local Y + offsetY
    └── retorno     → offsetY + LOGICAL_HEIGHT
```

## Contrato caracterizado

1. Um `.lvl` representa um chunk local, não um documento de mundo completo.
2. A altura lógica de um chunk é a unidade de composição atualmente usada para avançar `offsetY`.
3. O `Level` acumula plataformas dos chunks no mesmo `m_platforms` global.
4. O `flag` atual é um atributo singular de `Level`; `appendFromData()` redefine `hasFlag` antes de processar cada chunk.
5. Consequentemente, a existência de uma flag em chunks anteriores não é preservada como coleção de metadados de chunk.
6. `spawnPosition` existe em `LevelData`, mas `appendFromData()` não o materializa em estado explícito de `Level`; o stream usa `m_spawnY` como fronteira vertical, não como spawn semântico do jogador.

## Risco importante

A implementação atual suporta streaming de geometria, mas não possui uma abstração explícita de metadata por chunk. Isto não constitui automaticamente um bug porque a campanha atual parece usar a geometria acumulada como unidade principal. Contudo, qualquer requisito futuro de múltiplas flags, spawns por chunk, IDs, bounds, versioning ou unload/stream reversível exigirá distinguir metadados locais de estado global do mundo.

## Decisão

O contrato atual deve ser tratado como **chunk-local geometry + metadata parcial**, não como um modelo geral de world chunks.

Não alterar produção nesta tranche. O próximo passo deve ser um teste de characterization que torne explícitas as propriedades atuais de composição e detete regressões antes da Fase 10.

## Definition of Done

- [x] fronteira `LevelData` / `Level` caracterizada;
- [x] responsabilidade de `offsetY` identificada;
- [x] estado global do stream identificado;
- [x] limitações de metadata atual documentadas;
- [x] risco de assumir um contrato mais forte registado;
- [ ] teste executável de characterization;
- [ ] decisão sobre eventual modelo explícito de chunk metadata.

## Evidência

- `Game/Logic/LevelData.h`;
- `Game/Logic/LevelDataIO.cpp`;
- `Game/Logic/Level.h`;
- `Game/Logic/Level.cpp`;
- `Game/Logic/CampaignRuntime.h`;
- `Game/Logic/CampaignRuntime.cpp`;
- `Game/Assets/Levels/inicio.lvl`.

## Resultado

**Status:** `investigated / characterization pending`

**Residual:** characterization executável do contrato atual antes de qualquer redesign de metadata.
