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

## Estado da auditoria arquitetural pós-#136

Foi encontrado um novo coupling concreto: `Game/Core/GameStateMachine.h` dependia de `Graphics/GameState.h`, embora `GameState` seja apenas um enum de estado de runtime sem dados de rendering. O finding foi formalizado no **Issue #137**.

### Próximo work package

**Issue:** #137 — `refactor: move GameState definition out of Graphics`

**Decisão:** canonicalizar o enum em `Game/Core/GameState.h`, fazer `Graphics/GameState.h` apenas reexportar um alias compatível `gfx::GameState`, e migrar `GameStateMachine` para o tipo `core::GameState`.

**Objetivo arquitetural:**

```text
Core state contract
        ↓
GameSession / state machine
        ↓
Presentation
```

em vez de:

```text
Core / Logic
        ↓
Graphics-owned state contract
```

A mudança não altera os estados nem o comportamento de transição e não introduz uma abstração genérica de state machine.

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