# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`

**Work Package:** `Core-owned GameState contract`

**Issue:** `#137`

**Branch de implementação:** `refactor/core-gamestate-boundary-20260828`

## Contexto

A auditoria pós-PR #136 encontrou uma dependência estrutural concreta: `Game/Core/GameStateMachine.h` incluía `Graphics/GameState.h` e expunha `gfx::GameState` como contrato da máquina de estados.

`GameState` é apenas um enum de estado de runtime. Não contém dados de rendering nem exige ownership de Graphics.

## Decisão

Mover a definição canónica para `Game/Core/GameState.h`.

`Game/Graphics/GameState.h` continua disponível como alias explícito:

```cpp
using GameState = core::GameState;
```

Isto preserva consumidores existentes enquanto inverte a dependência estrutural para:

```text
Core state contract
        ↓
GameSession / state machine
        ↓
Presentation
```

## Escopo

- adicionar `Core/GameState.h`;
- migrar `GameStateMachine.h/.cpp` para `core::GameState`;
- manter `gfx::GameState` como alias compatível;
- adicionar characterization compile-time;
- atualizar arquitetura, roadmap e dívida técnica;
- validar todos os workflows obrigatórios.

## Fora de escopo

- alterar os cinco estados existentes;
- alterar transições;
- alterar rendering/Vulkan;
- criar abstrações genéricas;
- redesenhar `main.cpp`.

## Validação

```text
Core header ownership
→ type-identity compatibility
→ state-machine characterization
→ Linux normal
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
Game/Core/GameStateMachine sem include de Graphics/GameState
+ definição canónica em Core
+ gfx::GameState type-identical
+ comportamento preservado
+ CI obrigatório verde
+ documentação sincronizada
```

## Estado atual

`IMPLEMENTED — pending PR/CI validation`

## Próxima decisão

Depois do #137, voltar à revisão final de ownership/arquitetura. Só abrir nova tranche de desacoplamento quando houver um finding concreto e um contrato mínimo comprovável.
