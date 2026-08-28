# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`

**Work Package concluído:** `Core-owned GameState contract`

**Issue:** `#137`

**Branch de implementação:** `refactor/core-gamestate-boundary-20260828`

## Resultado

O finding foi resolvido no PR #137. A definição canónica de `GameState` passou para `Game/Core/GameState.h`; `Game/Graphics/GameState.h` permanece apenas como alias compatível `gfx::GameState = core::GameState`, e os consumidores internos foram migrados para `core::GameState`.

A direção resultante é:

```text
Core state contract
        ↓
GameSession / state machine
        ↓
Presentation
```

## Validação

- Linux / Clang / C++20 / Headless Vulkan: **success**
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**
- Windows / Clang / C++20: **success**
- source-size: **success**
- campaign validation: **success**

**Merge:** `b9f0d0021bef341327bfde1cdd02d2be8171e0ba`

## Próxima decisão

Voltar à revisão final de ownership/arquitetura. Só abrir nova tranche quando existir um finding concreto com coupling observável e um contrato mínimo verificável. A próxima auditoria deve começar por `main.cpp`, `GameSession` e composição/startup, mas não deve presumir que uma extração de `Application` seja necessária.

```text
investigar
→ documentar
→ confirmar finding
→ implementar menor mudança suficiente
→ validar
→ reconciliar documentação
```
