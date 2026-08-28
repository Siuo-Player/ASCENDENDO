# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável.

## Gate 9.6 — estado

**CLOSED**

O fecho formal segue a integração da revisão final do Gate (PR #118) e confirma que não existe blocker técnico restante dentro do escopo 9.6.

### Evidência integrada

- PR #76 — `RuntimeBootstrap` como composição de startup, sem `Application` genérica.
- PR #81 — rollback agregado de `GraphicsRuntime`.
- PR #87 — falhas de reconfiguração de `VulkanContext` tratadas como terminais.
- PR #85 — Windows build/test evidence em runner real com Vulkan software driver.
- PR #88 — `TickInput` como unidade semântica da simulação.
- PR #90 — `ReplayManager` alinhado a `TickInput`.
- PR #92 — malformed syntax da gramática atual coberta por testes; PR #91 foi superseded.
- PR #94 — failure-path evidence específica para `vkDeviceWaitIdle()` em `RendererCore`.
- PR #95 — reconciliação do replay contra o `main` atual.
- PR #99 — licença MIT do projeto e fronteiras de licenciamento de terceiros.
- PR #100 — caracterização de collision-order por permutação.
- PR #101 — documentação do resultado de collision-order.
- PR #102 — reconciliação canónica do Gate após #100/#101.
- PR #105 — evidência executável de independência do current working directory para `RuntimePaths::fromProcess(nullptr)` nos ambientes suportados.
- PR #108 — classificação dos failure/error paths Vulkan residuais.
- PR #109 — contrato pós-`vkQueueSubmit`: semântica terminal/fail-closed no design atual.
- PR #113 — caracterização documental do contrato `LevelData → Level → CampaignRuntime`.
- PR #114 — characterization tests desse contrato.
- PR #115 — isolamento dos residuais de replay/input.
- PR #116 — characterization executável da fronteira frame → `TickInput`.
- PR #117 — reconciliação do roadmap/technical debt com os Studies e a `main` real.
- PR #118 — revisão final de Vulkan capability/queue/synchronization, architecture/ownership e disposição dos claims residuais de replay.

### Replay

Provado:

- replay tick-semantic;
- comparação de estado por tick;
- fronteira frame → `TickInput`.

Não provado e explicitamente não necessário para este Gate segundo a revisão final:

- live-input frame-rate independence;
- terminal/result replay de uma sessão completa;
- replay persistence/serialization.

### Collision-order

A evidência permanece limitada ao cenário exercitado. Universal permutation invariance não é uma claim do projeto neste momento.

### Runtime-root

A independência de CWD está evidenciada para `RuntimePaths::fromProcess(nullptr)` nos targets exercitados. Packaging/deployment e fallbacks não exercitados continuam fora desta claim.

### World/chunk

O comportamento atual está caracterizado por #113/#114. O modelo mais forte de metadata e schema/versioning permanece Fase 10.

### Vulkan

A revisão final confirmou que:

- queue selection distingue graphics e present capabilities;
- `VK_KHR_swapchain` é requisito do logical device;
- surface support/capabilities/formats/present modes são verificadas pelo caminho de swapchain;
- `vkDeviceWaitIdle()` failure e `vkQueueSubmit()` post-failure têm contratos explícitos;
- não existe requisito atual que justifique fault injection genérica de todas as branches.

### Architecture / ownership

`GameSession` mantém estado de sessão sem ownership gráfico. `GraphicsRuntime` mantém o ownership agregado do stack gráfico e o cleanup em ordem inversa. `PresentationRuntime` mantém os recursos de presentation e attachments não-owning.

`RendererFacade` ainda recebe `logic::Player`/`logic::Level` diretamente em parte do caminho. A migração geral para `RenderSnapshot` permanece dívida arquitetural futura, não blocker do Gate 9.6.

Não foi encontrada responsabilidade adicional que justifique uma `Application` genérica nesta revisão.

## Gaps restantes após o fecho

Nenhum blocker técnico de 9.6 permanece.

Fora do Gate / próximos blocos:

- generalização de `RenderSnapshot` e separação domain/presentation;
- semantic validation/schema/versioning de `LevelData` (Fase 10);
- replay persistence;
- live-input frame-rate independence;
- terminal/result replay de uma sessão completa.

Estas capacidades futuras só devem ser promovidas a requisitos quando existir uma decisão de produto explícita.

## Regras preservadas

1. Renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Runtime não depende de CWD na claim exercitada.
5. `RuntimeBootstrap` é composição de startup, não `Application` genérica.
6. `LevelDataIO` continua parser/serializer, não schema authority.
7. Ordem de `Level::platforms()` não é assumida irrelevante sem evidência.
8. `ReplayManager` usa `TickInput`.
9. CI failure causes exigem evidência observável.
10. Implementation semantics e executable evidence continuam estados distintos.

## Próximo bloco

O próximo trabalho arquitetural pode iniciar independentemente do fecho de 9.6:

```text
RenderSnapshot/domain-presentation boundary
→ dedicated work package
→ implementation
→ validation
```
