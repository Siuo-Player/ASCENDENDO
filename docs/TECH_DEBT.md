# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável. Severidade:

- **P0** — impede evoluir com segurança ou pode causar comportamento incorreto em runtime/release.
- **P1** — deve ser tratado antes das fases seguintes que dependem dele.
- **P2** — melhoria importante de manutenção/performance.
- **P3** — limpeza posterior.

## Estado atual da migração do renderer

A implementação legada `Renderer.cpp/.h` já não está presente em `main`. A migração para `RendererCore` + passes + `RendererFacade` está integrada.

A próxima dívida arquitetural de presentation é a ausência de um `RenderSnapshot` geral: em `main`, `WorldRenderer` ainda recebe diretamente `Player` e `Level`.

## P0 — estado atual do Gate 9.6

| Área | Estado | Saída |
|---|---|---|
| GraphicsRuntime rollback | **proven** | PR #81 |
| VulkanContext terminal failure state | **implemented / partial evidence** | PR #87; fault injection inferior continua limitado |
| Windows compatibility | **proven for selected runner** | PR #85 |
| Tick-scoped simulation input | **implemented** | PR #88 |
| Replay tick representation | **proven for explicit TickInput sequence** | PR #90; não inclui persistence/live sampling |
| Malformed current grammar | **proven** | PR #91; semantic validation/schema continuam fora |
| Runtime/main decomposition | partial | continuar por responsabilidade real |
| Presentation/domain boundary | partial | RenderSnapshot continua bloqueado |
| Paths/CWD independence | open | resolver executable/asset/user-data roots |
| Level world/chunk metadata | open | contrato formal e testes |
| Collision-order determinism | open | propriedade física + teste adversarial |
| Vulkan queue/capability/error evidence | partial | ampliar evidence matrix |
| Architecture/ownership final review | open | revisão final antes do Gate |

## Regras arquiteturais

1. O renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Dados importáveis permanecem declarativos e sem execução de código.
5. O current working directory não é dependência do runtime.
6. CI testa o produto, não apenas unit tests.
7. Causas de falhas CI exigem evidência observável.
8. `GameSession` é orchestration boundary.
9. `RuntimeBootstrap` é composição de startup, não `Application` genérica.
10. `LevelDataIO` é parser/serializer, não semantic validator nem schema authority.
11. Ordem de `Level::platforms()` não é assumida irrelevante para determinismo.
12. Input edge para replay pertence ao tempo de simulação.
13. `ReplayManager` usa `TickInput`; isso não prova live-input frame-rate independence.

## Gate 9.6 — critérios de fecho

```text
Vulkan failure/error semantics com evidência suficiente
+
queue/capability assumptions evidenciados
+
deterministic simulation/replay evidence
+
Windows build/test evidence real
+
malformed/error-path evidence
+
architecture/ownership final review
```

A migração geral de `RenderSnapshot` não deve avançar antes do fecho formal do Gate.

## Evidência recente

### Windows — PR #85

Build, testes, Vulkan software driver, campaign validation e artefacto foram demonstrados num runner Windows real. A evidência aplica-se ao ambiente selecionado; não prova compatibilidade universal.

### Replay — PR #90

`ReplayManager` passou a armazenar/reproduzir `TickInput`. Tests, ASan/UBSan e Windows passaram no mesmo head. A propriedade demonstrada é reprodução de uma sequência explícita de ticks; persistence e live sampling continuam distintas.

### Malformed syntax — PR #91

A gramática atual é testada para rejeição de token desconhecido, número inválido, campo truncado e trailing tokens, além de um caso válido. Tests, Sanitizers e Windows passaram no mesmo head. Semantic validation, schema/versioning e canonicalization seguem para a Fase 10.

### Vulkan — PR #87

`VulkanContext::reconfigureForSurface()` trata falhas como terminais e normaliza device/queue handles e `isInitialized()`. A evidência experimental de falhas inferiores continua limitada porque o projeto não introduziu um framework genérico de fault injection Vulkan.
