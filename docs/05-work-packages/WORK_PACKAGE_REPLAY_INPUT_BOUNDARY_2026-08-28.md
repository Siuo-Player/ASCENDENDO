# Work Package — replay/input frame boundary characterization

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsistema:** `Logic / InputManager + ReplayManager`
**Work Package:** `9.6 replay/input frame boundary characterization`
**Branch:** `test/9-6-replay-terminal-live-20260828`

## Objetivo

Caracterizar o contrato atual entre sampling de input por frame e `TickInput` por tick, sem transformar essa evidência numa afirmação universal de independência do frame-rate do input ao vivo.

## Inclui

- edge events (`jumpPressed`/`jumpReleased`) no primeiro tick de cada frame;
- ações contínuas (`left`, `right`, `jumpHeld`) em todos os ticks derivados do mesmo frame;
- documentação da fronteira entre sampling externo e semântica da simulação.

## Não inclui

- alteração do `InputManager`;
- alteração do `ReplayManager`;
- simulação GLFW real com múltiplas cadências de render;
- persistência de replay;
- terminal/result replay completo.

## Decisão arquitetural

`InputManager::tickInput()` já contém um contrato explícito: callbacks/edges são amostrados uma vez por frame, edges são emitidos apenas em `tickInFrame == 0`, enquanto ações held permanecem disponíveis para cada fixed tick. O teste deve tornar esse contrato executável.

Este contrato não prova que duas cadências reais de frame produzem sempre a mesma sequência semântica de ticks. Essa propriedade mais forte exige controlar a relação temporal entre eventos GLFW e ticks e permanece fora desta tranche.

## Validação

O teste `Tests/Unit/test_input_tick_boundary.cpp` verifica:

1. press edge apenas no primeiro tick;
2. release edge apenas no primeiro tick;
3. ações held em múltiplos ticks do mesmo frame.

Validação obrigatória: Linux normal, Linux ASan/UBSan e Windows.

## Definition of Ready

- [x] contrato atual lido no código;
- [x] limite da claim explicitado;
- [x] teste mínimo definido;
- [x] sem alteração de produção necessária.

## Definition of Done

- [x] teste executável criado;
- [ ] CI 3/3 verde;
- [ ] resultado documentado;
- [ ] decisão sobre live-input frame-rate independence mantida explícita.

## Evidência

- `Game/Logic/InputManager.cpp`;
- `Game/Logic/InputManager.h`;
- `Tests/Unit/test_replay.cpp`;
- `Tests/Unit/test_input_tick_boundary.cpp`.

## Resultado esperado

**Status:** `characterization pending`

Um resultado verde demonstra o contrato de boundary já implementado. Não fecha, por si só, a propriedade mais forte de frame-rate independence do input ao vivo.
