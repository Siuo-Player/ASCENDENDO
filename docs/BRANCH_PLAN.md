# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / editor boundaries`

**Work Package:** `EditorInteraction presentation-independent input boundary`

**Issue:** `#135`

**Branch de implementação:** `refactor/editor-interaction-layer-boundary-20260828`

## Contexto

`Game/Logic/EditorInteraction.h` dependia diretamente de `Graphics/Camera.h` através de `cursorFromLogical(..., const gfx::Camera&)`.

## Descoberta

A implementação utiliza apenas `camera.position.x` e `camera.position.y`. O tipo `gfx::Camera` não acrescenta informação necessária à política de interação.

## Decisão

Usar `const Vec2& cameraPosition` como contrato mínimo.

```text
logical cursor + camera position
→ world cursor
```

A `Camera` permanece em presentation/composição. `Game/Logic` não deve depender de um tipo concreto de `Game/Graphics` quando um valor de domínio simples é suficiente.

## Escopo

- remover `Graphics/Camera.h` de `EditorInteraction.h`;
- alterar implementação e consumidores;
- preservar exatamente a transformação cursor→world;
- manter teste de characterization;
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
+ API usa somente Vec2 para camera position
+ comportamento preservado
+ nenhum consumidor antigo
+ CI obrigatório verde
+ documentação sincronizada
```

## Resultado

Ainda não implementado nesta branch; documentação/decisão concluída antes da alteração de código, conforme `DEVELOPMENT_PROTOCOL.md`.

## Próxima decisão

Depois de integrar #135, auditar os restantes includes cruzados `Game/Logic ↔ Game/Graphics` antes de abrir outra tranche. Só remover dependências com evidência de coupling e contrato mínimo claro.