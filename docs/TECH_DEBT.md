# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável. Severidade:

- **P0** — impede evoluir com segurança ou pode causar comportamento incorreto em runtime/release.
- **P1** — deve ser tratado antes das fases seguintes que dependem dele.
- **P2** — melhoria importante de manutenção/performance.
- **P3** — limpeza posterior.

## Estado atual da migração do renderer

A implementação legada `Renderer.cpp/.h` já não está presente em `main`. A migração para `RendererCore` + passes + `RendererFacade` está integrada.

A próxima dívida arquitetural de presentation é a ausência de um `RenderSnapshot` geral: em `main`, `WorldRenderer` ainda recebe diretamente `Player` e `Level`. PR #20 foi encerrado/superseded; a migração continua uma responsabilidade do roadmap, mas não existe uma branch histórica ativa a manter.

## P0 — tratar antes de continuar a acumular complexidade

| Área | Problema | Ação | Critério de saída |
|---|---|---|---|
| Runtime | `main.cpp` acumula inicialização, bootstrap, frame coordination e apresentação | continuar extraindo responsabilidades incrementais por ownership real | `main.cpp` deixa de concentrar regras de runtime que já têm fronteira própria |
| Presentation | `RendererFacade`/passes ainda recebem modelos de domínio diretamente | introduzir `RenderSnapshot`/dados de apresentação | presentation recebe dados próprios de apresentação |
| Paths | runtime usa paths relativos ao current working directory | criar resolução de `executable root`, `asset root` e `user data root` | executar o EXE a partir de qualquer diretório suportado |
| Levels / validation | sintaxe desconhecida, números inválidos, campos truncados e trailing tokens da gramática atual são rejeitados; schema/versioning e validação semântica ainda não existem como contrato único | na Fase 10, introduzir envelope/schema explícito, validação semântica e canonicalização | corpus malformed/unknown-version/invalid-domain rejeitado de forma determinística; representação canónica documentada |
| Levels / domain model | `Level` acumula geometria mundial, mas `name`/`hasFlag`/`flagBounds` representam apenas o chunk mais recentemente anexado | definir formalmente o escopo de world/chunk metadata antes de alterar campos individualmente | estado de mundo e metadata de chunk/entry têm owners e invariantes explícitos |
| Physics / determinism | `Level::resolveCollision()` pode produzir estados diferentes quando a ordem das plataformas é permutada em situações de múltiplo contacto | decidir contrato de ordem e cobrir com teste adversarial | propriedade física escolhida e evidência determinística |
| Physics / input | `TickInput` é agora a unidade explícita da simulação e do replay | manter semântica por tick e separar replay de live input sampling | mesma sequência de `TickInput` reproduz o mesmo estado tick-a-tick |
| Vulkan | seleção de queue/device exige evidência separada de graphics/present/extension/capabilities | ampliar matriz e manter assumptions explícitas | capability matrix mínima demonstrada |
| Vulkan lifecycle | rollback e terminal-state foram implementados, mas lower-level fault injection continua parcial | aumentar cobertura onde o ambiente permitir; não assumir happy-path como prova de todos os error paths | política pós-falha explicitamente testada ou limitação documentada |

## P1 — consolidação

| Área | Estado atual |
|---|---|
| Windows CI | evidência integrada no PR #85 para runner Windows selecionado |
| Linux ASan/UBSan | workflow independente no PR #70 |
| Level format versioning | aberto; Fase 10 |
| Semantic level validation | aberto; Fase 10 |
| Campaign document/identity | aberto |
| Vulkan RAII | aberto |
| Runtime/bootstrap decomposition | guardrails ativos |
| CI/tooling warnings | dívida separada |

## Regras arquiteturais

1. O renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Dados importáveis continuam declarativos e sem execução de código.
5. O current working directory não é uma dependência do runtime.
6. O CI testa o produto, não apenas testes unitários.
7. Causas de falha CI exigem evidência observável.
8. `GameSession` é orchestration boundary, não owner de platform/Vulkan/presentation.
9. `RuntimeBootstrap` é composição de startup, não service container nem `Application` nominal.
10. `LevelDataIO` é parser/serializer; não é ainda schema authority nem validator semântico.
11. A ordem de `Level::platforms()` não deve ser tratada como irrelevante para determinismo enquanto a política de múltiplos contactos não estiver definida.
12. A semântica de input edge para replay pertence ao tempo de simulação.
13. `ReplayManager` usa `TickInput`; isso não prova independência do live input face ao frame rate.

## Portões do roadmap

Antes de avançar para `RenderSnapshot` geral, o Gate 9.6 deve ter:

```text
Vulkan failure semantics com evidência suficiente
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

## Evidência recente

### Windows — PR #85

Build, testes, Vulkan software driver, campaign validation e artefacto foram demonstrados num runner Windows real. Esta evidência é válida para o ambiente selecionado e não implica compatibilidade universal de hardware/driver.

### Replay — PR #90

`ReplayManager` passou a armazenar/reproduzir `TickInput`. Tests, ASan/UBSan e Windows passaram no mesmo head. A propriedade demonstrada é reprodução para uma sequência explícita de ticks; persistence e live sampling continuam distintas.

### Malformed syntax — PR #91

A gramática atual é testada para rejeição de token desconhecido, número inválido, campo truncado e trailing tokens, além de um caso válido. Tests, Sanitizers e Windows passaram no mesmo head. Semantic validation, schema/versioning e canonicalization permanecem fora desta tranche e seguem a Fase 10.
