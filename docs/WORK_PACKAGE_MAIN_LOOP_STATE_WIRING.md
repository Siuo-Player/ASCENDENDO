# Work Package — Main Loop State Wiring

## Objetivo

Substituir progressivamente o estado de aplicação mantido localmente em `main.cpp` por `core::GameStateMachine`, preservando exatamente as transições atuais antes de extrair a simulação e o bootstrap.

## Estado atual observado em `main.cpp`

`main.cpp` ainda mantém localmente:

- `GameState state`;
- `GameState editorReturnState`;
- `GameState creditsReturnState`;
- `int menuSel`.

A branch anterior criou `GameStateMachine`, mas deliberadamente não alterou o runtime.

## Contrato de transições

```text
MENU
  Start       -> PLAYING
  Editor      -> EDITOR -> MENU
  Credits     -> CREDITS -> MENU
  Quit        -> exit

PLAYING
  Pause       -> PAUSED
  Quit        -> MENU
  Editor      -> EDITOR -> PLAYING
  Flag        -> CREDITS -> MENU

PAUSED
  Resume      -> PLAYING
  Credits     -> CREDITS -> PAUSED
  Menu        -> MENU

EDITOR
  Return      -> explicit return state

CREDITS
  Confirm     -> explicit return state
```

## Invariantes

1. Entrar em `MENU`, `PLAYING`, `PAUSED`, `EDITOR` ou `CREDITS` através da máquina deve deixar `menuSelection == 0`, quando a transição atual assim o exige.
2. `EDITOR` e `CREDITS` mantêm explicitamente o estado de retorno.
3. Navegação relativa faz wrapping circular.
4. Seleção absoluta fora do intervalo faz wrapping circular.
5. `pause()` só altera `PLAYING`.
6. `resume()` só altera `PAUSED`.

## Sequência de implementação

```text
GameStateMachine ✅
      ↓
state/menuSel wiring      ← este work package
      ↓
frame orchestration
      ↓
Simulation / fixed timestep
      ↓
Application/bootstrap
```

A extração deve manter cada PR compilável e com CI verde. O `main.cpp` não deve ser reescrito de uma só vez.
