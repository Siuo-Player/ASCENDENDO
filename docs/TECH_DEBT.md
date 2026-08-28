# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável.

## Gate 9.6 — estado

**OPEN**

### Evidência integrada

- PR #76 — `RuntimeBootstrap` como composição de startup, sem `Application` genérica.
- PR #81 — rollback agregado de `GraphicsRuntime`.
- PR #87 — falhas de reconfiguração de `VulkanContext` tratadas como terminais; lower-level fault evidence continua parcial.
- PR #85 — Windows build/test evidence em runner real com Vulkan software driver.
- PR #88 — `TickInput` como unidade semântica da simulação.
- PR #90 — `ReplayManager` alinhado a `TickInput`.
- PR #92 — malformed syntax da gramática atual coberta por testes; PR #91 foi superseded.
- PR #94 — failure-path evidence específica para `vkDeviceWaitIdle()` em `RendererCore`.
- PR #95 — reconciliação do replay contra o `main` atual.
- PR #99 — licença MIT do projeto e fronteiras de licenciamento de terceiros.
- PR #100 — caracterização de collision-order por permutação, sem alteração da física de produção.
- PR #101 — documentação do resultado da caracterização.

### Estado residual de replay

A evidência atual demonstra replay tick-semantic e comparação de estado por tick sob agrupamento externo diferente da mesma sequência `TickInput`. Permanecem separadas e abertas:

- live-input frame-rate independence;
- terminal/result replay completo de `GameSession`;
- persistence/replay serialization.

### Estado residual de collision-order

O caso exercitado por #100 não apresentou divergência entre as duas ordens testadas. Isso é evidência limitada ao conjunto de contactos e estado inicial usados no teste; não deve ser transformado em uma alegação universal de permutation invariance.

### Gaps restantes

- capability/error evidence Vulkan além do happy path;
- paths/runtime roots independentes do current working directory;
- contrato world/chunk metadata de `Level`;
- eventual política global de collision-order apenas se surgir requisito ou counterexample adicional;
- semantic validation/schema/versioning de `LevelData` (Fase 10);
- revisão final de ownership/architecture.

## Regras

1. Renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Runtime não depende de current working directory.
5. `RuntimeBootstrap` é composição de startup, não `Application` genérica.
6. `LevelDataIO` é parser/serializer, não schema authority nem semantic validator.
7. Ordem de `Level::platforms()` não é assumida irrelevante para determinismo.
8. Input edge para replay pertence ao tempo de simulação.
9. `ReplayManager` usa `TickInput`; isto não prova live-input frame-rate independence.
10. Causas de falha CI exigem evidência observável.
11. Implementation semantics e executable evidence permanecem estados distintos.

## Fecho do Gate

Antes da migração geral de `RenderSnapshot`, deve existir evidência suficiente para:

```text
Vulkan failure/error semantics
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
