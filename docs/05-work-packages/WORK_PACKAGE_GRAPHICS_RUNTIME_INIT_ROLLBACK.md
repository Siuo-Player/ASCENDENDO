# Work Package — GraphicsRuntime init rollback contract

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `GraphicsRuntime / Vulkan lifecycle`  
**Work Package:** `9.6 GraphicsRuntime initialization rollback`  
**Branch:** `fix/9-6-graphics-runtime-init-rollback`  
**Base:** `main` @ `87c195b559c15cf6b548150499a45c7b04a09f85`

## Objetivo

Tornar `GraphicsRuntime::init()` fail-closed quando uma etapa intermédia falha e definir retry semantics sem alterar ownership global de GLFW nem introduzir uma nova abstraction layer.

## Descoberta / evidência

A implementação atual inicializa, por ordem, `Window`, `VulkanContext`, surface, `Swapchain`, `RenderPass`, `Pipeline` e `RendererFacade`, retornando imediatamente em qualquer erro. Os componentes individuais possuem cleanup/destruction paths, mas `GraphicsRuntime` não faz rollback explícito antes de devolver `false`.

A consequência é que um objeto pode ficar parcialmente inicializado até à destruição do próprio objeto. Como `init()` não documenta nem garante retry safety, uma segunda chamada no mesmo objeto não constitui atualmente um contrato seguro.

A ordem de destruição dos membros é compatível com as dependências:

```text
RendererFacade
→ Pipeline
→ RenderPass
→ Swapchain
→ VulkanContext
→ Window
```

## Decisão

Implementar rollback explícito de `GraphicsRuntime` através de uma função local de cleanup na ordem inversa da aquisição. Em qualquer falha durante `init()`, o runtime regressa a um estado neutro antes de devolver `false`.

Após rollback:

```text
isInitialized() == false
Window not created
VulkanContext not initialized
Swapchain not initialized
RenderPass not initialized
Pipeline not initialized
RendererFacade not initialized
```

Uma nova chamada a `init()` passa a ser segura desde esse estado neutro.

Não alterar o contrato de `Window`: GLFW continua a ser inicializado/terminado no process boundary.

## Alternativas consideradas

### A — One-shot / failed init terminal

Mais simples, mas deixa estado parcial dentro do objeto e não transforma `false` num estado operacional limpo.

### B — Rollback automático no `init()`

Escolhida. Preserva a composição existente, reforça o lifecycle contract e permite retry sem adicionar uma nova camada.

### C — Construção transacional com temporários locais

Mais forte em atomicidade, mas incompatível com a ownership atual sem refatoração mais ampla e desnecessária para este Gate.

## Inclui

- cleanup/rollback explícito de `GraphicsRuntime`;
- retry seguro depois de uma falha de inicialização;
- testes do estado neutro após falhas que possam ser induzidas deterministicamente;
- atualização de arquitetura/dívida/work package;
- validação Linux/headless e ASan/UBSan.

## Não inclui

- alteração de `VulkanContext` para novos ownership models;
- nova `Application` class;
- swapchain recreation policy;
- injeção genérica de Vulkan calls;
- Windows CI;
- mudança de queue selection.

## Dependências / consumidores

```text
GraphicsRuntime
├── Window
├── VulkanContext
├── Swapchain
├── RenderPass
├── Pipeline
└── RendererFacade
```

Consumers: `main.cpp`, integration tests de graphics, destruction/cleanup paths dos componentes.

## Validação

1. preservar o build e comportamento nominal existente;
2. provar cleanup em falhas induzíveis sem mocks artificiais, sempre que o baseline oferecer uma condição determinística;
3. repetir `init()` após rollback e demonstrar estado consistente;
4. normal Linux/headless;
5. ASan/UBSan;
6. documentar qualquer failure path que continue impossível de injetar sem nova abstração.

## Definition of Ready

- [x] `GraphicsRuntime::init()` inspecionado;
- [x] ownership/destruction order dos membros inspecionados;
- [x] `VulkanContext::shutdown()` confirmado como rollback-safe;
- [x] `Swapchain`/`Pipeline`/`RenderPass`/`RendererFacade` cleanup APIs confirmadas;
- [x] alternativa one-shot versus rollback avaliada;
- [x] escopo negativo definido.

## Definition of Done

- [ ] rollback centralizado e idempotente;
- [ ] retry contract documentado;
- [ ] testes relevantes verdes;
- [ ] ASan/UBSan verdes;
- [ ] documentação canónica atualizada;
- [ ] PR integrada;
- [ ] remaining Vulkan lifecycle gaps explicitamente mantidos como debt.
