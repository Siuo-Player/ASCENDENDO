# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

O roadmap orienta a ordem do trabalho, mas uma propriedade só muda de estado quando existe evidência suficiente para a alegação correspondente.

## Gate 9.6 — Base Engineering Gate

O Gate está **CLOSED** a partir da integração da revisão final do Gate (PR #118) e da confirmação de fecho formal (PR #119).

## RenderSnapshot

**Issue:** #122  
**WP:** `docs/05-work-packages/WORK_PACKAGE_RENDERSNAPSHOT_BOUNDARY_2026-08-28.md`  
**Estado:** **FIRST TRANCHE CONCLUÍDA (PR #129) + editor boundary concluída (PR #132)**

A primeira tranche separou o world/player path do domínio através de `gfx::RenderSnapshot` e `RenderSnapshotBuilder`. O editor passou a materializar `EditorRenderSnapshot` na composição e `RendererFacade` deixou de depender diretamente de `EditorSession`.

## Shared Vulkan image upload

**Issue:** #23  
**Implementation:** PR #133  
**Merge:** `e3871bc935dfa52124ec5244ddbb04714caec161`  
**Estado:** **COMPLETED**

A duplicação de lifecycle Vulkan entre `FontRendererGpu` e `SpriteRendererGpu` foi centralizada em `Game/Graphics/VulkanImageUpload.h/.cpp`, preservando `VkFormat`, `VkFilter`, descriptor policy e ownership nos consumidores.

## EditorInteraction layer boundary

**Issue:** #135  
**WP:** `docs/05-work-packages/WORK_PACKAGE_EDITOR_INTERACTION_LAYER_BOUNDARY_2026-08-28.md`  
**Implementation:** PR #136  
**Merge:** `4d587af1f8d4633e47d4c86b51fb503493f27550`  
**Estado:** **COMPLETED**

### Descoberta

`Game/Logic/EditorInteraction.h` incluía `Graphics/Camera.h` e expunha `cursorFromLogical(..., const gfx::Camera&)`. A implementação usava somente `camera.position`, criando uma dependência de presentation desnecessária dentro da lógica do editor. Durante a validação também foi observada a mesma dependência concreta em `EditorSession.cpp`; no modelo atual de editor de tela única, a posição necessária é `{0,0}`.

### Decisão

Substituir `const gfx::Camera&` por `const Vec2& cameraPosition` e remover o uso de `gfx::Camera` de `EditorSession`. A `Camera` permanece propriedade da presentation; a lógica recebe somente os dados mínimos necessários.

```text
logical cursor + camera position
→ world cursor
```

No editor de tela única, `EditorSession` passa `Vec2{0.0f, 0.0f}`.

### Evidência

- `cursorFromLogical()` preserva exatamente a soma `logical + cameraPosition`;
- characterization test foi migrado para `Vec2`;
- Linux / Clang / C++20 / Headless Vulkan: **success**;
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**;
- Windows / Clang / C++20: **success**;
- source-size e campaign validation: **success**;
- nenhum consumidor do contrato antigo permanece no build validado.

Durante o primeiro ciclo de CI foi encontrado e corrigido o include concreto ausente de `Graphics/Camera.h` em `EditorRenderer.cpp`; a correção necessária ficou no mesmo ciclo de implementação antes do merge.

### Fora de escopo

- mudança de `Camera`;
- mudança do sistema de coordenadas;
- mudança da interação do editor;
- nova abstração genérica de transformação.

## GameState ownership boundary

**Issue:** #137  
**WP:** `docs/05-work-packages/WORK_PACKAGE_CORE_GAMESTATE_BOUNDARY_2026-08-28.md`  
**Implementation:** PR #137  
**Merge:** `b9f0d0021bef341327bfde1cdd02d2be8171e0ba`  
**Estado:** **COMPLETED**

### Descoberta

`Game/Core/GameStateMachine.h` dependia de `Graphics/GameState.h`, embora `GameState` seja apenas um enum de estado de runtime. Isso colocava um contrato de Core sob ownership de Graphics sem necessidade.

### Decisão

A definição canónica passou para `Game/Core/GameState.h`. `Game/Graphics/GameState.h` ficou como alias explícito de compatibilidade (`gfx::GameState = core::GameState`), enquanto `GameStateMachine`, `GameSession`, `main.cpp` e os testes passaram a usar diretamente `core::GameState`.

```text
Core state contract
        ↓
GameSession / state machine
        ↓
Presentation
```

Os estados e as transições não foram alterados.

### Evidência

- Linux / Clang / C++20 / Headless Vulkan: **success**;
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**;
- Windows / Clang / C++20: **success**;
- source-size e campaign validation: **success**;
- `static_assert` confirma a identidade de tipo entre `gfx::GameState` e `core::GameState`;
- issue #137 fechado como completed após integração.

## Próximo alvo — revisão final de ownership/arquitetura

Os dois findings concretos desta tranche foram resolvidos:

```text
EditorInteraction → Camera dependency       DONE (#136)
GameStateMachine → Graphics/GameState       DONE (#137)
```

A próxima etapa não deve ser mais uma movimentação de tipos por estética. Deve ser uma revisão global de ownership, especialmente em `main.cpp`, `GameSession`, RuntimeBootstrap e Presentation, procurando responsabilidades duplicadas ou fronteiras que possam ser testadas isoladamente.

Só abrir nova tranche quando existir:

1. finding concreto;
2. contrato mínimo claro;
3. mudança semântica explicitamente preservada;
4. evidência executável que possa validar a decisão.

Não reabrir o Gate 9.6 por propriedades futuras já adiadas, como replay persistence, terminal/result replay ou live-input frame-rate independence, sem novo requisito ou evidência.

## Fase 10

Semantic `LevelData` validation/schema/versioning permanece separado do bloco de presentation e só avança quando a evidência/requisito correspondente o justificar.

## Princípios de execução

```text
investigar
→ documentar
→ atualizar roadmap/architecture/tech-debt/WP
→ implementar
→ testar/validar
→ documentar resultado, falhas e próxima decisão
```

A decomposição de `main.cpp` é incremental e baseada em ownership/responsabilidade/testabilidade; não criar uma `Application` genérica apenas para reduzir linhas.