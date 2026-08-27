# Plano da branch atual

**Bloco do roadmap:** `9.6 Base Engineering Gate`

**Work Package:** `9.6 Main Loop / GameSession Boundary`

**Branch:** `refactor/9-6-game-session-boundary`

## Base confirmada

`main` integra:

- `GameStateMachine` como dono das transições de estado;
- `SimulationOrchestrator` como dono da orquestração fixed-step;
- `GraphicsRuntime` como dono do core gráfico e da sua ownership graph;
- `PresentationRuntime` como dono dos recursos de texto/sprite;
- `RuntimePaths` para asset/user-data roots;
- PR #70 com evidência Linux ASan/UBSan explícita.

## Descoberta desta tranche

`main.cpp` ainda cria e coordena diretamente `Level`, `PhysicsWorld`, `SimulationOrchestrator`, `Player`, `CampaignRuntime` e `EditorSession`, além de aplicar a política de estados. Estes objetos têm um lifetime de sessão coerente e não dependem de ownership Vulkan para existir.

A investigação também mostrou que `InputManager` já é uma fronteira de input com conhecimento de GLFW e que `Camera`/renderer pertencem à apresentação. Colocá-los dentro de uma nova `Application` nesta fase misturaria responsabilidades.

## Decisão

A primeira extração será `logic::GameSession`.

```text
GameSession
├── GameStateMachine
├── CampaignRuntime
├── Level
├── PhysicsWorld
├── SimulationOrchestrator
├── Player
└── EditorSession
```

`GameSession` não possui `Window`, `VulkanContext`, `Swapchain`, `RenderPass`, `Pipeline`, `RendererFacade`, `PresentationRuntime` ou `Camera`.

A entry point mantém process/GLFW lifetime, bootstrap de caminhos, composição gráfica/apresentação, polling de janela e submissão de frame. Um `Application` nominal fica adiado até existir uma fronteira de bootstrap/lifecycle justificável por ownership real.

## Em escopo

- implementar `GameSession` sem alterar regras de gameplay;
- mover ownership e estado de sessão reais para a nova fronteira;
- mover a política de transições associada a esses objetos;
- preservar streaming, reset, completion, run history e editor return state;
- adicionar testes da nova fronteira;
- atualizar documentação após validação.

## Fora de escopo

- `Application` monolítica;
- RenderSnapshot geral;
- input-system redesign;
- graphics/Vulkan lifecycle redesign;
- gameplay/physics tuning;
- save/schema redesign;
- performance optimization.

## Dependências

```text
GraphicsRuntime ──→ Window/Vulkan/RendererFacade
PresentationRuntime ──→ presentation resources
InputManager + KeyBindings ──→ GameSession update
Camera ──→ presentation
GameSession ──→ stateful gameplay/editor runtime
```

## Validação

```text
unit tests da GameSession
→ make tests
→ Linux/Clang/headless Vulkan
→ campaign validation
→ revisão da ownership graph
```

## Critério de saída

```text
GameSession owns the agreed session state
+ main.cpp no longer owns those session rules
+ behavior preserved by tests
+ CI green
+ architecture/roadmap/WP synchronized
+ no new Application/RenderSnapshot coupling
```

## Próxima decisão

Depois desta tranche, reavaliar o que resta em `main.cpp`: bootstrap/configuration, frame-loop/presentation submission e qualquer dependência residual. Só então decidir se outra fronteira é necessária.
