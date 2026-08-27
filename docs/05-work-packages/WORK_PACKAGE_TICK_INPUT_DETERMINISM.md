# Work Package — tick-input determinism contract

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Input / Simulation / Replay`
**Work Package:** `9.6 tick-scoped input semantics and replay evidence`
**Branch:** `fix/9-6-tick-input-determinism`
**Base:** `main` @ `9f42dd7d3db5969844ac94c0045332ec07d18717`

## Objetivo

Transformar o gap documentado de input frame-scoped versus simulation tick numa propriedade verificável, sem criar uma abstração de replay apenas por nome.

## Finding confirmado

`GameSession::update()` entrega um `InputManager` a `SimulationOrchestrator::advance()`. Quando um frame produz vários fixed steps, o mesmo input, incluindo `justPressed`/`justReleased`, é reutilizado em todos os ticks.

Isto não prova que o jogo seja atualmente não determinístico, mas impede a alegação mais forte de replay tick-exact.

## Decisão antes da implementação

Introduzir uma unidade semântica `TickInput` para a simulação.

Contrato:

```text
frame sample
    ↓
TickInput(tick=0)
    ↓
TickInput(tick=1)
    ↓
...
```

- ações contínuas (`left`, `right`, `jumpHeld`) podem permanecer iguais em todos os ticks derivados do mesmo frame;
- `jumpPressed`/`jumpReleased` pertencem somente ao primeiro tick que consome a amostra do frame;
- `Player` consome `TickInput`, não consulta diretamente a noção de frame;
- a representação deve poder ser fornecida diretamente por um harness de simulação sem GLFW.

## Inclui

- `TickInput`/equivalente semântico estreito;
- adaptação de `InputManager` para produzir input por tick;
- `Player` e `SimulationOrchestrator` usando a unidade tick-scoped;
- testes D2–D4 mínimos: contínuo, edge-event e frame repartition;
- documentação da semântica temporal.

## Não inclui

- sistema de gravação/persistência de replay completo;
- redesign de `ReplayManager` além do necessário para a unidade tick;
- RenderSnapshot;
- alterações de gameplay não necessárias ao contrato;
- GLFW/test harness dependente de GPU.

## Dependências

`InputManager` → `SimulationOrchestrator` → `Player` → `PhysicsWorld`.

## Consumidores

`Player`, `SimulationOrchestrator`, testes de input/física/replay e futura validação tick-by-tick.

## Critérios de validação

1. Mesmo `TickInput` + mesmo estado inicial produz o mesmo estado por tick.
2. `justPressed`/`justReleased` são consumidos apenas no tick semântico correspondente.
3. A mesma sequência de comandos por tick permanece igual quando os ticks são agrupados em frames diferentes.
4. Testes continuam independentes de GLFW.

## Riscos

- alterar a semântica de edge events do gameplay atual;
- confundir a unidade de input com persistência de replay;
- manter código de compatibilidade que permita regressar ao consumo frame-scoped.

## Definition of Ready

- [x] Estudos atuais consultados;
- [x] gap confirmado no código atual;
- [x] consumidores identificados;
- [x] contrato tick-scoped definido antes da implementação.

## Definition of Done

- [ ] unidade tick-scoped implementada;
- [ ] `Player` e `SimulationOrchestrator` migrados;
- [ ] testes D2–D4 verdes;
- [ ] evidência frame-repartition preservada;
- [ ] documentação canónica atualizada;
- [ ] PR pronta para merge.

## Relação com o Gate 9.6

Este WP não fecha todo o eixo de replay. D5–D7 (reprodução de estado terminal e persistência) permanecem trabalho futuro. O objetivo deste bloco é estabelecer a unidade temporal correta sobre a qual a evidência de replay pode ser construída.
