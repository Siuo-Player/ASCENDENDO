# Work Package — Core-owned GameState boundary

## Identificação

**Roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`  
**Work Package:** `Core-owned GameState contract`  
**Issue:** `#137`  
**Implementation branch:** `refactor/core-gamestate-boundary-20260828`

## Context

`Game/Core/GameStateMachine.h` dependia diretamente de `Game/Graphics/GameState.h` e expunha `gfx::GameState` como o contrato de estado da máquina. O enum não contém dados de rendering e é usado para representar estado do runtime.

## Problema / evidência

A dependência observada era:

```text
Game/Core/GameStateMachine
        ↓
Game/Graphics/GameState
```

Isto faz Core depender estruturalmente de um tipo possuído por Graphics, apesar de a informação ser um contrato de estado do runtime.

## Decisão

Canonicalizar `GameState` em `Game/Core/GameState.h`.

`Game/Graphics/GameState.h` permanece temporariamente como uma compatibilidade explícita:

```cpp
using GameState = core::GameState;
```

Assim, consumidores gráficos existentes podem continuar a usar `gfx::GameState` sem manter a definição em Graphics.

`GameStateMachine` passa a depender diretamente de `core::GameState`.

## Invariante

A mudança deve preservar exatamente o conjunto e a semântica dos estados:

```text
PLAYING
PAUSED
CREDITS
MENU
EDITOR
```

Nenhuma transição deve mudar como consequência da alteração de ownership do tipo.

## Escopo

- criar `Game/Core/GameState.h`;
- migrar `Game/Core/GameStateMachine.h/.cpp`;
- tornar `Game/Graphics/GameState.h` um alias compatível;
- caracterizar a equivalência dos tipos;
- sincronizar documentação.

## Fora de escopo

- alterar nomes ou valores dos estados;
- alterar a política de transição;
- alterar `GameSession` além do necessário para compatibilidade;
- introduzir uma abstração genérica de state machine;
- alterar rendering/Vulkan;
- redesenhar `main.cpp`.

## Validação

```text
Core GameState compiles independently
→ compatibility alias remains type-identical
→ existing state-machine characterization remains green
→ full Linux tests/headless Vulkan
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
Game/Core/GameStateMachine não inclui Graphics/GameState
+ definição canónica vive em Core
+ gfx::GameState continua type-identical por alias
+ sem alteração de semântica
+ três workflows obrigatórios verdes
+ documentação sincronizada
```

## Próxima decisão

Após esta tranche, repetir a auditoria apenas para dependências concretas entre Core/Logic e Graphics. Não transformar namespace hygiene em refatoração ampla: cada mudança deve ter coupling demonstrável e contrato mínimo verificável.
