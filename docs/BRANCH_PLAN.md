# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / editor boundaries`

**Work Package:** `EditorInteraction presentation-independent input boundary`

**Issue:** `#135`

**Branch de implementação:** `refactor/editor-interaction-layer-boundary-20260828`

## Contexto

`Game/Logic/EditorInteraction.h` dependia diretamente de `Graphics/Camera.h` através de `cursorFromLogical(..., const gfx::Camera&)`.

## Descoberta

A implementação utiliza apenas `camera.position.x` e `camera.position.y`. O tipo `gfx::Camera` não acrescenta informação necessária à política de interação.

Durante a validação do PR #136, o mesmo princípio foi encontrado em `EditorSession.cpp`: a sessão do editor criava `gfx::Camera` apenas para obter a posição zero de uma câmara fixa da tela lógica. `EditorSession` já documenta que não possui estado de câmera.

## Decisão

Usar `const Vec2& cameraPosition` como contrato mínimo em `EditorInteractionController` e passar `Vec2{0.0f, 0.0f}` desde `EditorSession`, mantendo a regra de que Camera pertence a presentation/composição.

```text
logical cursor + camera position
→ world cursor
```

No editor de tela única, a posição da câmara é `{0,0}`.

## Escopo

- remover `Graphics/Camera.h` de `EditorInteraction.h`;
- alterar implementação e consumidores;
- remover o uso desnecessário de `gfx::Camera` em `EditorSession.cpp`;
- preservar exatamente a transformação cursor→world;
- manter characterization tests;
- validar build, sanitizers e Windows.

## Fora de escopo

- `Camera`;
- sistema de coordenadas;
- comportamento STAMP/DRAG/move/delete;
- GLFW/Vulkan;
- novas abstrações genéricas.

## Validação

```text
header dependency removed
→ compile consumers
→ cursor→world characterization
→ full tests
→ ASan/UBSan
→ Windows
→ global dependency audit
```

## Critério de saída

```text
Game/Logic/EditorInteraction sem Graphics/Camera include
+ EditorSession sem uso de gfx::Camera
+ API usa somente Vec2 para camera position
+ comportamento preservado
+ nenhum consumidor antigo
+ CI obrigatório verde
+ documentação sincronizada
```

## Estado atual

`IMPLEMENTED — pending CI validation`

O PR #136 revelou ainda um include concreto ausente em `EditorRenderer.cpp`; a correção foi aplicada na mesma branch porque é necessária para restaurar a compilação dos três targets obrigatórios.

## Próxima decisão

Depois de integrar #135, auditar os restantes includes cruzados `Game/Logic ↔ Game/Graphics` antes de abrir outra tranche. Só remover dependências com evidência de coupling e contrato mínimo claro.