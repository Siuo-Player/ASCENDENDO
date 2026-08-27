# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável. Severidade:

- **P0** — impede evoluir com segurança ou pode causar comportamento incorreto em runtime/release.
- **P1** — deve ser tratado antes das fases seguintes que dependem dele.
- **P2** — melhoria importante de manutenção/performance.
- **P3** — limpeza posterior.

## Estado atual da migração do renderer

A implementação legada `Renderer.cpp/.h` já não está presente em `main`. A migração para `RendererCore` + passes + `RendererFacade` está integrada.

A próxima dívida arquitetural de presentation é a ausência de um `RenderSnapshot` geral: em `main`, `WorldRenderer` ainda recebe diretamente `Player` e `Level`.

## P0 — tratar antes de continuar a acumular complexidade

| Área | Problema | Ação | Critério de saída |
|---|---|---|---|
| Runtime | `main.cpp` acumula inicialização, bootstrap, frame coordination e apresentação | continuar extraindo responsabilidades incrementais por ownership real | nenhuma regra de runtime sem fronteira própria permanece concentrada sem rationale |
| Presentation | presentation ainda recebe modelos de domínio diretamente | introduzir `RenderSnapshot`/dados próprios de apresentação | presentation desacoplada dos modelos de domínio para os consumidores migrados |
| Paths | runtime usa paths relativos ao current working directory | criar resolução de `executable root`, `asset root` e `user data root` | executar o EXE a partir de qualquer diretório suportado |
| Levels / validation | grammar malformed atual é coberta; schema/versioning e semantic validation continuam ausentes | Fase 10: schema explícito, validação semântica, canonicalização | corpus malformed/unknown-version/invalid-domain com política determinística |
| Levels / domain model | `Level` mistura geometria acumulada com metadata do último chunk | definir owners/invariantes de world/chunk state | contrato formal + testes |
| Physics / determinism | múltiplos contactos podem depender da ordem das plataformas | decidir propriedade física e cobrir com teste adversarial | resultado determinístico segundo o contrato escolhido |
| Vulkan | capability assumptions requerem evidência além do happy path | ampliar capability/error-path evidence | matrix e failure policy demonstradas |
| Vulkan lifecycle | rollback e terminal-state implementados, mas lower-level fault injection continua parcial | criar apenas seams necessários para propriedades de risco elevado | failure policy observável ou limitação documentada |

## P1 — consolidação

| Área | Estado atual |
|---|---|
| Windows CI | evidência integrada no PR #85 |
| Linux ASan/UBSan | workflow independente no PR #70 |
| Replay representation | TickInput integrado no PR #90 |
| Malformed current grammar | evidência integrada no PR #91 |
| Level format versioning | aberto; Fase 10 |
| Semantic level validation | aberto; Fase 10 |
| Campaign document/identity | aberto |
| Vulkan RAII | aberto |
| Runtime/bootstrap decomposition | guardrails ativos |
| CI/tooling warnings | dívida separada |

## Regras de arquitetura

1. Renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Dados importáveis permanecem declarativos e sem execução de código.
5. Runtime não depende de current working directory.
6. CI testa o produto, não apenas unit tests.
7. Causas de falha CI exigem evidência observável.
8. `GameSession` é orchestration boundary.
9. `RuntimeBootstrap` é composição de startup, não service container nem `Application` genérica.
10. `LevelDataIO` é parser/serializer, não semantic validator ou schema authority.
11. Ordem de `Level::platforms()` não é assumida irrelevante para determinismo.
12. Input edge para replay pertence ao tempo de simulação.
13. `ReplayManager` usa `TickInput`; isso não prova live-input frame-rate independence.

## Gate 9.6

O Gate permanece aberto até haver evidência suficiente para:

```text
Vulkan failure semantics
+
queue/capability assumptions
+
deterministic simulation/replay
+
Windows build/test
+
malformed/error paths
+
architecture/ownership review
```

A migração geral de `RenderSnapshot` não deve avançar antes do fecho formal do Gate.

## Evidência recente

### Windows — PR #85

Build, testes, Vulkan software driver, campaign validation e artefacto foram demonstrados num runner Windows real. A evidência vale para o ambiente selecionado.

### Replay — PR #90

`ReplayManager` armazena e reproduz `TickInput`. Tests, ASan/UBSan e Windows passaram no head validado. A propriedade demonstrada é reprodução de uma sequência explícita de ticks; persistence e live sampling permanecem distintas.

### Malformed syntax — PR #91

A gramática atual tem testes para token desconhecido, número inválido, campo truncado, trailing tokens e caso válido. Tests, Sanitizers e Windows passaram no mesmo head. Semantic validation, schema/versioning e canonicalization continuam na Fase 10.
