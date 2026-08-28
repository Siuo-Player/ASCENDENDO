# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`

**Work Package:** `Semantic TickInput contract`

**Issue:** `#138`

**Branch de implementação:** `refactor/semantic-tick-input-boundary-20260828`

## Contexto

A auditoria pós-#137 encontrou um acoplamento concreto: `Game/Logic/Player.h` incluía `Game/Logic/InputManager.h` apenas para conhecer `TickInput`.

`Player` não usa callbacks, key codes, mouse state, `GLFWwindow` ou `KeyBindings`; recebe somente o estado semântico já preparado para um tick de simulação.

## Decisão

Mover `TickInput` para `Game/Logic/TickInput.h`.

```text
InputManager
    ↓ produz
TickInput
    ↓ consome
Player
```

`TickInput` continua em Logic porque é um contrato específico da simulação do jogo. Não há evidência que justifique movê-lo para Core.

## Escopo

- adicionar `Logic/TickInput.h`;
- remover a definição de `TickInput` de `InputManager.h`;
- remover a inclusão de `InputManager.h` de `Player.h`;
- manter a construção agregada e os cinco campos;
- explicitar o include do contrato no teste de Player;
- atualizar documentação.

## Fora de escopo

- redesign do `InputManager`;
- alteração de bindings ou teclas;
- alteração de física/gameplay;
- generalização de input;
- mover `TickInput` para Core.

## Validação

```text
header dependency removed
→ aggregate construction preserved
→ Player characterization
→ Linux normal
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
Player.h sem InputManager.h
+ TickInput canónico em Logic/TickInput.h
+ InputManager continua produtor
+ comportamento preservado
+ CI obrigatório verde
+ documentação sincronizada
```

## Estado atual

`IMPLEMENTED — pending PR/CI validation`

## Próxima decisão

Depois do #138, voltar à revisão de ownership/arquitetura e abrir nova tranche apenas para crossings concretos com contrato mínimo verificável.
