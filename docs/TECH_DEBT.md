# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável.

## Gate 9.6 — estado

**CLOSED**

O fecho formal segue a integração da revisão final do Gate (PR #118) e a confirmação formal (PR #119); não existe blocker técnico restante dentro do escopo 9.6.

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
- PR #105 — evidência executável de independência do current working directory nos targets suportados.
- PR #108 — classificação dos failure/error paths Vulkan residuais.
- PR #109 — contrato pós-`vkQueueSubmit`: semântica terminal/fail-closed.
- PR #113 — caracterização documental do contrato `LevelData → Level → CampaignRuntime`.
- PR #114 — characterization tests desse contrato.
- PR #115 — isolamento dos residuais de replay/input.
- PR #116 — characterization executável da fronteira frame → `TickInput`.
- PR #117 — reconciliação do roadmap/technical debt com os Studies e a `main` real.
- PR #118 — revisão final do Gate.
- PR #119 — fecho formal do Gate 9.6.

## Replay

Provado:

- replay tick-semantic;
- comparação de estado por tick;
- fronteira frame → `TickInput`.

Não provado e explicitamente não necessário para este Gate:

- live-input frame-rate independence;
- terminal/result replay de uma sessão completa;
- replay persistence/serialization.

## RenderSnapshot — dívida arquitetural ativa

`RendererFacade` e `WorldRenderer` ainda atravessam a fronteira de presentation com `logic::Player`, `logic::Level` e `Camera`. Esta dívida passa agora a ser um work package arquitetural dedicado após o Gate 9.6.

**Issue:** #122  
**WP:** `docs/05-work-packages/WORK_PACKAGE_RENDERSNAPSHOT_BOUNDARY_2026-08-28.md`  
**Branch:** `docs/render-snapshot-wp-20260828`

### Contrato inicial

```text
RenderSnapshot
├── camera { x, y }
├── player { x, y, width, height, facingLeft }
├── platforms[] { x, y, width, height }
└── flag { visible, x, y, width, height }
```

O contrato deve permanecer mínimo: adicionar dados apenas quando um pass de presentation os consumir. Não transportar tipos de domínio, handles Vulkan, ownership, estado mutável ou lógica de gameplay.

### Exit criteria

```text
snapshot como value object de presentation
+ sem tipos de domínio
+ sem Vulkan ownership
+ builder explícito/determinístico
+ WorldRenderer sem Player/Level
+ comportamento preservado
+ testes/build/CI
```

## Outras dívidas explicitamente adiadas

- semantic validation/schema/versioning de `LevelData` — Fase 10;
- replay persistence;
- live-input frame-rate independence;
- terminal/result replay;
- decomposição de `FontRenderer` e `SpriteRenderer` em WPs separados quando a análise de coesão/coupling justificar a mudança.

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
11. Presentation deve receber dados necessários para renderização, não o modelo mutável de gameplay, quando a fronteira snapshot estiver disponível.

## Próximo bloco

```text
RenderSnapshot/domain-presentation boundary
→ dedicated work package
→ implementation
→ validation
→ decidir expansão para restantes presentation paths
```