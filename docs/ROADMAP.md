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
**Estado:** **READY FOR IMPLEMENTATION**

### Descoberta

`Game/Logic/EditorInteraction.h` ainda inclui `Graphics/Camera.h` e expõe `cursorFromLogical(..., const gfx::Camera&)`. A implementação usa somente `camera.position`, criando uma dependência de presentation desnecessária dentro da lógica do editor.

### Decisão

Substituir `const gfx::Camera&` por `const Vec2& cameraPosition`. A `Camera` permanece propriedade da presentation; a composição fornece apenas o dado necessário para converter coordenadas lógicas em coordenadas de mundo.

```text
logical cursor + camera position
→ world cursor
```

### Escopo

- remover a dependência `Game/Logic → Game/Graphics/Camera`;
- preservar exatamente a transformação cursor→world;
- adaptar consumers/tests;
- validar build, sanitizers e Windows;
- sincronizar documentação.

### Fora de escopo

- mudança de `Camera`;
- mudança do sistema de coordenadas;
- mudança da interação do editor;
- nova abstração genérica de transformação.

## Próximo alvo após #135

Reavaliar as restantes dependências entre `Game/Logic` e `Game/Graphics` com o mesmo critério: só remover coupling quando existir dependência concreta e o contrato mínimo puder ser expresso sem introduzir abstração artificial.

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