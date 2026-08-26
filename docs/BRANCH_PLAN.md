# Plano da branch atual

**Bloco do roadmap:** `9.6 Base Engineering Gate`

**Work Package:** `Application / graphics ownership boundary`

## Base confirmada

O `main` atual já integrou:

- source-size gate alinhado com a política documental;
- decomposição dos testes de `KeyBindings` e `Level`;
- `GameStateMachine` como dono das transições de runtime;
- `SimulationOrchestrator` como dono da orquestração fixed-step;
- `main.cpp` já delega a simulação para `SimulationOrchestrator`.

PR #46 demonstrou a primeira propriedade importante: a entry point deixou de possuir diretamente o ciclo `PhysicsWorld::advance -> Player::update -> Level::resolveCollision`.

## Descoberta desta tranche

A decomposição seguinte não deve criar uma classe `Application` genérica apenas para reduzir linhas. `Window` e `VulkanContext` são objetos RAII e não-copiáveis, e o seu lifetime é semanticamente relevante para surface/swapchain/device. A composição gráfica deve respeitar essa ownership graph.

## Decisão

A próxima fronteira será definida a partir de ownership real:

```text
Window
  ↓ creates surface
VulkanContext
  ↓ owns device/queues/surface
Swapchain / render infrastructure
  ↓
RendererFacade
```

A primeira implementação deve criar uma composição pequena, testável e explicitamente não-copiável. Não criar uma abstraction layer Vulkan genérica.

## Em escopo

- mapear os objetos gráficos e a ordem de destruição;
- definir uma fronteira de composição mínima;
- preservar exatamente o comportamento atual de bootstrap/failure paths;
- adicionar uma propriedade/invariante testável à nova fronteira;
- manter `main.cpp` como entry point fino, sem reescrever o renderer.

## Fora de escopo

- `RenderSnapshot` geral;
- novas abstrações genéricas de Vulkan;
- otimizações;
- mudanças de gameplay;
- Campaign Editor;
- PCG/difficulty systems.

## Validação

```text
ownership/lifetime documentados
→ failure paths preservados
→ build/tests
→ headless Vulkan
→ campaign validation
```

## Critério de saída

```text
nova fronteira possui responsabilidade única
+ ownership/lifetime explícitos
+ cópia proibida quando necessário
+ comportamento/failure paths preservados
+ teste/invariante relevante
+ CI verde
+ ARCHITECTURE/ROADMAP sincronizados
```
