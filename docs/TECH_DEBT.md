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
- PR #102 — reconciliação canónica do Gate após #100/#101.
- PR #105 — evidência executável de independência do current working directory para `RuntimePaths::fromProcess(nullptr)` nos ambientes suportados.
- PR #108 — classificação dos failure/error paths Vulkan residuais.
- PR #109 — contrato pós-`vkQueueSubmit`: semântica terminal/fail-closed no design atual.
- PR #113 — caracterização documental do contrato `LevelData → Level → CampaignRuntime`.
- PR #114 — characterization tests desse contrato, sem alterar semântica de produção.
- PR #115 — isolamento dos residuais de replay/input.
- PR #116 — characterization executável da fronteira frame → `TickInput`.

### Estado residual de replay

A evidência atual demonstra:

- replay tick-semantic;
- comparação de estado por tick;
- fronteira frame → `TickInput` com edges restritas ao primeiro tick do frame e ações held disponíveis nos ticks seguintes do mesmo frame.

Permanecem separadas e abertas:

- live-input frame-rate independence;
- terminal/result replay completo de `GameSession`;
- persistence/replay serialization.

Nenhuma destas propriedades deve ser inferida apenas da passagem dos testes existentes.

### Estado de collision-order

O caso exercitado por #100 não apresentou divergência entre as duas ordens testadas. Isso é evidência limitada ao conjunto de contactos e estado inicial usados no teste; não deve ser transformado em uma alegação universal de permutation invariance.

### Estado runtime-root

PR #105 demonstrou por teste que `RuntimePaths::fromProcess(nullptr)` produz o mesmo executable root e paths derivados de assets/levels/sprite quando a chamada ocorre sob dois current working directories distintos nos ambientes da matriz CI. Esta evidência não define uma política de packaging/deployment futura nem elimina limites não exercitados do fallback de `argv0`.

### Estado world/chunk

PRs #113/#114 já caracterizaram e protegeram o comportamento atual de composição. Portanto este assunto deixa de ser um finding não caracterizado do Gate e passa a ser um contrato documentado.

O modelo atual continua deliberadamente limitado: `flag` é metadata singular e `spawnPosition` não é materializado como metadata persistente de `Level`. Um modelo semântico geral de chunks e schema/versioning pertencem à Fase 10, salvo requisito novo.

### Estado Vulkan

- #94 fornece evidência executável para `vkDeviceWaitIdle()`.
- #108 classifica os restantes paths por contrato, evitando uma matriz artificial de fault injection.
- #109 estabelece que falha de `vkQueueSubmit` depois de reset do fence é terminal/fail-closed no design atual; não existe promessa de recuperação em processo da frame falhada.

O gap restante é uma revisão de capability/queue/synchronization assumptions e da suficiência da evidência atual, não a obrigação de testar cada `VkResult` isoladamente.

## Gaps restantes

1. revisão final das assumptions de capability/queue/synchronization de Vulkan;
2. revisão final de ownership/architecture;
3. disposição explícita sobre se terminal/result replay e live-input frame-rate independence são requisitos de Gate;
4. decisão formal de fecho do Gate 9.6.

Ficam fora do Gate, salvo requisito novo:

- semantic validation/schema/versioning de `LevelData` (Fase 10);
- replay persistence/serialization;
- generalização de `RenderSnapshot` antes do fecho do Gate.

## Regras

1. Renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Runtime não depende de current working directory para a resolução de paths de processo atualmente exercitada.
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
+
explicit disposition of remaining replay claims
```
