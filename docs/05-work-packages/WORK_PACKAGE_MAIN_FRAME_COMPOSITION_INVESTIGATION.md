# Work Package — Investigação da composição de frame do entry point

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Runtime / frame composition`
**Work Package:** `9.6 main.cpp frame-composition investigation`
**Branch:** `docs/9-6-main-entrypoint-investigation`
**Base:** `main` após PR #85 (`7693d18de347ad5c45be668824cd26e4e7dedda9`)
**Classificação:** `INVESTIGATION`

## Objetivo

Determinar se a responsabilidade ainda concentrada em `main.cpp` contém uma fronteira de composição real que possa ser extraída sem criar uma classe `Application` nominal nem duplicar `GameSession`, `PresentationRuntime`, `SimulationOrchestrator` ou `RuntimeBootstrap`.

## Evidência observada

O `main.cpp` atual concentra cinco grupos distintos:

```text
Process lifetime
├── GlfwRuntime
└── shutdown / device idle

Startup
├── RuntimeBootstrap
├── GraphicsRuntime
├── PresentationRuntime
├── InputManager + KeyBindings
└── GameSession

State-presentation coordination
├── window titles
├── Camera reset on state transitions
└── Camera follow during PLAYING

Frame orchestration
├── input.beginFrame()
├── window.pollEvents()
├── GameSession::update()
├── camera coordination
└── RendererFacade::drawFrame()

Process-level diagnostics
├── startup/fallback messages
└── campaign completion reporting
```

A existência destes grupos não implica automaticamente cinco classes novas. A extração deve seguir ownership e responsabilidade, não tamanho físico.

## Boundary candidates

### Candidato A — `FrameCoordinator`

Responsabilidade potencial:

```text
poll input/events
→ update GameSession
→ advance Camera according to state
→ apply state-transition presentation effects
→ submit current presentation frame
```

Esta é a única fronteira identificada nesta investigação com coesão potencial suficiente para uma extração posterior.

### O que `FrameCoordinator` não deve possuir

- `GLFW` global lifetime;
- `GraphicsRuntime` ownership;
- `RuntimeBootstrap`/paths/campaign discovery;
- `GameSession` internals ou ownership dos seus serviços;
- Vulkan device/swapchain lifecycle;
- regras de gameplay;
- parsing/validation de levels;
- `RendererFacade` ownership, salvo se uma decisão posterior demonstrar que a coordenação de frame deve ser owner explícito.

### Candidato rejeitado — `Application`

Não existe evidência suficiente para criar uma classe genérica `Application` apenas para diminuir `main.cpp`. Tal classe tenderia a acumular lifecycle, services, rendering, camera e gameplay e a tornar-se um service container.

### Candidato rejeitado — mover Camera para `GameSession`

`Camera` é presentation state. Colocá-la em `GameSession` inverteria a fronteira estabelecida no roadmap e ligaria estado de gameplay a presentation.

### Candidato rejeitado — mover títulos de janela para `GameStateMachine`

Títulos e feedback de janela são presentation/UI concerns, não transições de domínio.

## Decisão provisória

**DECISION:** não implementar já uma nova classe.

**REASON:** o recorte `FrameCoordinator` é plausível, mas precisa de um contrato explícito de ownership e de confirmação de que a sua API não se transforma num segundo `GameSession` ou num facade de rendering.

A próxima tranche de implementação, caso a investigação permaneça consistente, deve limitar a API a dados/serviços necessários à composição de um frame. O objeto não deve conhecer detalhes internos de `GameSession` nem expor decisões de gameplay.

## Contrato que deve ser testado antes da extração

Uma futura implementação deverá preservar pelo menos:

1. mesma ordem observável: `beginFrame → pollEvents → session.update → camera update → state presentation → drawFrame`;
2. mesma semântica de `GameSessionUpdateResult`;
3. mesma regra de reset/follow da `Camera`;
4. mesmo comportamento em `quitRequested`;
5. nenhuma dependência adicional de modelos de domínio no renderer;
6. nenhuma mudança na semântica de input edge/fixed-step;
7. shutdown e `vkDeviceWaitIdle()` continuam fora da coordenação de frame.

## Relação com os estudos

O estudo de `RuntimeBootstrap` estabelece que a extração seguinte não deve voltar a misturar bootstrap, rendering e gameplay. A investigação atual é coerente com esse limite: `FrameCoordinator` é uma hipótese estreita de composição, não uma arquitectura global.

O Base Engineering Gate continua a exigir separadamente:

- replay/tick-level determinism;
- malformed/error-path evidence;
- Vulkan lifecycle/queue failure-path evidence;
- architecture review final;
- evidence matrix transversal.

## Definition of Ready para a futura implementação

- [x] `main.cpp` atual inspecionado após PR #85;
- [x] `RuntimeBootstrap` tratado como fronteira existente;
- [x] responsabilidades de frame separadas conceptualmente;
- [x] candidata `FrameCoordinator` identificada;
- [x] `Application` nominal explicitamente rejeitada;
- [ ] contrato de ownership da futura fronteira aprovado pela revisão arquitetural;
- [ ] testes de equivalência preparados.

## Definition of Done desta investigação

- [x] responsabilidades atuais documentadas;
- [x] fronteira candidata classificada;
- [x] anti-patterns explicitamente rejeitados;
- [x] nenhuma alteração de runtime necessária nesta fase;
- [x] próximo passo condicionado a contrato e testes.

## Próximo passo

Preparar uma branch de implementação curta apenas quando o contrato de `FrameCoordinator` estiver suficientemente determinado. Em paralelo, não avançar para `RenderSnapshot` geral enquanto o Gate 9.6 ainda tiver os restantes gaps técnicos abertos.
