# Work Package — EditorInteraction layer boundary

## Identificação

**Roadmap:** `Post-Gate 9.6 architecture / editor boundaries`  
**Work Package:** `EditorInteraction presentation-independent input boundary`  
**Issue:** `#135`  
**Implementation branch:** `refactor/editor-interaction-layer-boundary-20260828`

## Context

`Game/Logic/EditorInteraction.h` ainda inclui `Graphics/Camera.h` e expõe `cursorFromLogical(..., const gfx::Camera&)`. Isto cria uma dependência da camada de lógica do editor para um objeto de presentation, apesar de a própria arquitetura exigir que a política de interação permaneça testável sem dependência direta de rendering/Vulkan.

## Problema / evidência

A implementação de `cursorFromLogical()` usa apenas `camera.position.x` e `camera.position.y`. O tipo `gfx::Camera` não acrescenta informação necessária à lógica.

A dependência atual é, portanto:

```text
Game/Logic/EditorInteraction
        ↓
Game/Graphics/Camera
```

quando o contrato necessário é apenas:

```text
logical cursor + camera position
        ↓
world cursor
```

## Decisão

Substituir `const gfx::Camera&` por `const Vec2& cameraPosition`.

A `Camera` continua pertencente à presentation. A composição/presentation extrai a sua posição e entrega apenas o dado necessário ao controlador de interação.

## Alternativas consideradas

### A — manter `gfx::Camera`

Rejeitada: preserva uma dependência estrutural desnecessária entre Logic e Graphics.

### B — introduzir um novo tipo genérico para transformação

Rejeitada: a operação atual necessita apenas de uma posição 2D; outro wrapper acrescentaria abstração sem reduzir responsabilidade.

### C — usar `Vec2 cameraPosition`

Escolhida: remove coupling, preserva o comportamento e reduz a superfície da dependência.

## Escopo

- remover `Graphics/Camera.h` de `EditorInteraction.h`;
- alterar a assinatura de `cursorFromLogical()`;
- adaptar consumidores/tests;
- preservar exatamente a soma `logical + cameraPosition`;
- atualizar arquitetura/dívida/roadmap/WP.

## Fora de escopo

- alterar `Camera`;
- alterar o sistema de coordenadas;
- alterar interação STAMP/DRAG/move/delete;
- alterar GLFW/Vulkan;
- criar novos adapters genéricos.

## Dependências / consumidores

- `EditorInteractionController`;
- `Tests/Unit/test_editor_interaction.cpp`;
- consumidor que atualmente fornece `gfx::Camera` à conversão do cursor;
- `docs/ARCHITECTURE.md`;
- `docs/TECH_DEBT.md`;
- `docs/ROADMAP.md`.

## Riscos

| Risco | Mitigação |
|---|---|
| alterar a transformação cursor→world | characterization test com offsets positivos/zero/negativos |
| deixar algum consumidor com a API antiga | pesquisa global e build completo |
| introduzir nova abstração artificial | usar diretamente `Vec2` |

## Validação

- teste unitário da conversão logical→world;
- build/test suite;
- Linux ASan/UBSan;
- Windows/Clang;
- source-size;
- pesquisa global confirmando que `EditorInteraction.h` não inclui `Graphics/Camera.h`.

## Exit criteria

```text
Game/Logic/EditorInteraction sem dependência de Graphics/Camera
+ comportamento cursor→world preservado
+ consumidores migrados
+ testes/build/sanitizers/Windows verdes
+ documentação sincronizada
```

## Dívida criada

Nenhuma. A mudança reduz coupling e não adiciona uma nova camada de abstração.

## Próxima decisão

Após esta correção, manter `EditorInteraction` independente de tipos de presentation. Novas dependências entre Logic e Graphics devem ser tratadas como findings arquiteturais explícitos, não introduzidas por conveniência de chamada.
