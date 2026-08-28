# Work Package — Replay tick evidence

## Identificação

**Roadmap:** 9.6 — Evidência transversal  
**Subsistema:** Runtime  
**Work Package:** 9.6 — Replay tick evidence  
**Branch:** `fix/9-6-replay-tick-evidence-20260827` (histórica)  
**PR:** #90

## Objetivo

Alinhar `ReplayManager` com `TickInput` e demonstrar que a mesma sequência semântica de comandos, partindo do mesmo estado, reproduz o mesmo estado de simulação por tick.

## Escopo

### Inclui

- replay baseado em `TickInput`;
- comparação de estado tick-by-tick;
- invariância da mesma sequência semântica sob agrupamento externo diferente;
- failure paths do replay em memória.

### Não inclui

- persistência de replay;
- live-input frame-rate independence;
- replay completo de terminal/result de `GameSession`;
- migração geral de `RenderSnapshot`.

## Dependências

### Depende de

- PR #88 / contrato `TickInput`;
- `SimulationOrchestrator` fixed-step;
- `ReplayManager` e testes unitários.

### Produz para

- Gate 9.6 — evidência de determinismo/replay.

### Consumidores afetados

- `ReplayManager`;
- `Tests/Unit/test_replay.cpp`;
- documentação do Gate.

### Dependências de validação

- Linux normal/headless Vulkan;
- Linux ASan/UBSan;
- Windows CI;
- execução sem dependência de GLFW real.

## Decisões arquiteturais

```text
Problema/contexto:
A representação anterior ligava replay a FrameInput/InputManager.

Decisão:
O replay armazena e reproduz TickInput. Edge events são dados explícitos do tick.

Alternativas consideradas:
- manter FrameInput: rejeitada;
- criar ReplaySystem novo: rejeitada;
- persistir imediatamente: adiada para a propriedade D7.

Condição de revisão:
Rever se TickInput mudar ou a futura persistência exigir outra representação.
```

## Validação

### Testes automatizados

- mesma sequência reproduz estado após cada tick;
- `jumpPressed`/`jumpReleased` são preservados;
- a mesma sequência semântica agrupada de forma diferente produz o mesmo estado;
- replay vazio não produz ticks;
- playback além do fim é rejeitado sem mutar estado.

### Failure paths

- sequência vazia;
- fim do playback;
- limpeza/reset do replay.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [x] implementação concluída dentro do escopo;
- [x] testes relevantes integrados na `main`;
- [x] failure paths em memória cobertos;
- [x] documentação sincronizada com a implementação atual;
- [x] dependências revistas;
- [x] dívida residual classificada;
- [x] critério de saída do WP demonstrado.

## Alterações durante a execução

Em 2026-08-28 a matriz de Studies de 2026-08-27 foi revalidada contra `main` após #88, #90 e #94. O estado original dos Studies era anterior a estas integrações e, por isso, D2/D3/D5 já não devem ser classificados como não demonstrados.

O teste de agrupamento prova apenas uma sequência **já reduzida a TickInput**. Não prova que a captura de input live através de frames GLFW seja independente da taxa de frames.

Também não existe neste WP replay do resultado terminal completo de `GameSession` nem persistence.

## Evidência / referências

- `docs/AUDITS/2026-08-28-replay-evidence-reconciliation.md`;
- `Game/Logic/ReplayManager.cpp`;
- `Tests/Unit/test_replay.cpp`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-deterministic-input-replay-evidence-matrix.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-deterministic-replay-input-contract-study.md`;
- PR #88;
- PR #90.

## Fecho

**Resultado:** `concluído — evidência de replay tick-semantic integrada`  
**Critério de saída:** reprodução tick-by-tick e invariância da mesma sequência semântica sob agrupamento externo diferente  
**Dívida residual:** terminal/result replay, persistence e live-input frame-rate independence permanecem propriedades separadas
