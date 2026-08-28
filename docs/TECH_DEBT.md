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

## RenderSnapshot — primeira tranche concluída

A primeira tranche da fronteira `RenderSnapshot` foi integrada no PR #129 e completada com a remoção do acoplamento `RendererFacade → EditorSession` no PR #132.

### Contrato atual

```text
RenderSnapshot
├── player { bounds, facingDirection }
├── platforms[] { x, y, width, height }
└── flag { visible, x, y, width, height }
```

O snapshot reutiliza o contrato existente em `Game/Graphics/RenderSnapshot.h` e permanece um value object de presentation, sem `logic::Player`, `logic::Level`, `logic::Vec2`, `Camera`, recursos Vulkan ou ownership.

### Fronteira implementada

```text
logic::Player + logic::Level
            ↓
RenderSnapshotBuilder
            ↓
     gfx::RenderSnapshot
            ↓
RendererFacade / WorldRenderer
```

Para o editor:

```text
EditorSession
      ↓
EditorRenderSnapshot
      ↓
RendererFacade / EditorRenderer
```

`RendererFacade` não mantém nem consulta `EditorSession` mutável durante `drawFrame()`.

### Regra de custo

O world snapshot é construído apenas em `PLAYING`/`PAUSED`. O editor fornece o seu snapshot apenas em `EDITOR`.

### Validação

- PR #129 integrado com workflows obrigatórios verdes;
- PR #132 integrado com `Tests`, `Sanitizers` e `Windows` verdes;
- testes caracterizam cópia independente e API da facade sem `EditorSession`.

### Estado

`DONE — world/player + editor presentation boundaries`

## Shared Vulkan image upload — concluído

PR #133 integrou `Game/Graphics/VulkanImageUpload.h/.cpp` como primitive estreito para o lifecycle comum de criação/upload de imagens Vulkan de `FontRendererGpu` e `SpriteRendererGpu`.

### Centralizado

- staging buffer e memória host-visible;
- criação/binding de `VkImage` e memória device-local;
- command pool/buffer one-shot;
- image layout transitions;
- `vkCmdCopyBufferToImage`;
- `VkImageView`;
- `VkSampler`;
- cleanup de recursos parcialmente criados.

### Mantido nos consumidores

- `VkFormat`;
- `VkFilter`;
- dados de origem;
- descriptor pools/sets;
- descriptor policy;
- ownership dos handles devolvidos.

Comportamento preservado:

- Font: `R8_UNORM` + `LINEAR`;
- Sprite: `R8G8B8A8_UNORM` + `NEAREST`.

PR #133 foi integrado como `e3871bc935dfa52124ec5244ddbb04714caec161`. Os workflows obrigatórios Linux normal, Linux ASan/UBSan e Windows passaram, incluindo source-size, full tests, headless Vulkan e campaign validation.

### Estado

`DONE`

## EditorInteraction layer boundary — próxima tranche

Issue #135 identificou uma dependência residual de camada: `Game/Logic/EditorInteraction.h` inclui `Graphics/Camera.h` e expõe `cursorFromLogical(..., const gfx::Camera&)`, embora a operação utilize apenas `camera.position`.

### Decisão

Substituir a dependência da classe `Camera` por `const Vec2& cameraPosition`.

```text
logical cursor + camera position
→ world cursor
```

A `Camera` continua uma responsabilidade de presentation/composição. `EditorInteractionController` permanece em `Game/Logic` e não dependerá de tipos concretos de `Game/Graphics`.

### Validação requerida

- caracterização cursor→world preservando exatamente a soma;
- build/testes;
- ASan/UBSan;
- Windows/Clang;
- source-size;
- pesquisa de consumidores da API antiga.

### Estado

`READY FOR IMPLEMENTATION — branch refactor/editor-interaction-layer-boundary-20260828`

### Condição de revisão

Não introduzir um novo tipo de transformação. `Vec2` é o contrato mínimo; voltar a aumentar a dependência só se existir uma operação real que necessite de mais estado de `Camera`.

## Outras dívidas explicitamente adiadas

- semantic validation/schema/versioning de `LevelData` — Fase 10;
- replay persistence;
- live-input frame-rate independence;
- terminal/result replay;
- future presentation snapshots apenas quando houver benefício verificável.

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
11. Presentation recebe dados necessários para rendering, não o modelo mutável de gameplay/editor.
12. Shared Vulkan primitives devem permanecer estreitos e não absorver políticas específicas sem nova evidência.
13. `Game/Logic` não deve depender de tipos concretos de `Game/Graphics` para operações de interação.

## Próximo passo

```text
Issue #135
→ remove Logic → Graphics/Camera dependency
→ validate
→ update architecture/roadmap/debt/WP
→ re-audit remaining Logic ↔ Graphics includes
```