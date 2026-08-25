# Auditoria da base — 25/08/2026

## Objetivo

Esta auditoria regista a revisão posterior à integração da 9.4. O projeto melhorou significativamente com a migração incremental do renderer e com os snapshots do editor. Os problemas prioritários agora são sobretudo **fronteiras incompletas entre as novas abstrações, caminhos legados ainda ativos e regras documentadas que ainda não estão implementadas**.

A regra de manutenção é: primeiro fechar a base; só depois construir conteúdo significativo sobre ela.

## Estado atual

A direção arquitetural é considerada boa, mas a implementação ainda está intermédia:

```text
arquitetura-alvo
      ↓
implementação parcial
      ↓
adapters / caminhos legados / contratos incompletos
```

Isto não implica reescrever o motor. A estratégia continua sendo migração incremental, com testes antes e depois de cada fronteira.

## P0 — corrigir antes de novas funcionalidades significativas

### 1. Unificar o input de gameplay

`GameAction`/`KeyBindings` já existem, mas o `Player` ainda consulta teclas físicas diretamente (`Key`, `SPACE`, `A/D`). Isto permite configurações em que o menu afirma uma binding e o gameplay utiliza outra.

**Objetivo:**

```text
hardware → InputManager → KeyBindings → GameAction → Gameplay
```

Nenhuma regra de gameplay deve depender diretamente de uma tecla física quando já existir uma ação abstrata equivalente.

### 2. Fechar o contrato do editor de uma única tela

A documentação estabelece um Level Editor fixo de `640x360`, mas ainda existem caminhos de código que movem a câmera. Ao mesmo tempo, `EditorSession`/`EditorRenderer` tratam a câmera como fixa.

**Objetivo:** escolher e implementar um único contrato:

```text
Level Editor = uma tela fixa 640x360
```

A transformação usada pelo cursor e a usada pelo renderer devem ser a mesma. Nenhum pan inútil deve permanecer.

### 3. Corrigir o fixed timestep

O acumulador ainda permite catch-up ilimitado após um frame muito longo.

**Objetivo:**

- limitar passos de simulação por frame;
- definir política para tempo perdido/minimização;
- rejeitar `NaN`/`Inf`;
- manter replay determinístico.

### 4. Fechar ciclo de vida e erros de Vulkan

A sequência de frames deve ser segura quando falham submit/present/acquire. Em particular, um fence resetado não pode ficar perdido sem possibilidade de sinalização.

Também é necessário tratar explicitamente `VK_ERROR_OUT_OF_DATE_KHR` e `VK_SUBOPTIMAL_KHR` no fluxo de swapchain.

### 5. Eliminar a dependência de uma única graphics queue para apresentar

A seleção de device deve verificar separadamente graphics/present support, extensões e capabilities de surface. Graphics e present podem coincidir, mas não podem ser assumidas como a mesma queue em todos os devices.

## P1 — consolidar as fronteiras arquiteturais

### Renderer

A nova decomposição é mantida:

```text
RendererCore
ShapeRenderer
WorldRenderer
UiRenderer
EditorRenderer
```

Mas o objetivo final é remover os caminhos legados:

```text
Renderer antigo → RendererFacadeAdapter → RendererFacade
```

`RendererFacadeAdapter` é uma ponte de migração e não uma camada permanente.

`Renderer` antigo deve ser eliminado depois de migrar os seus consumidores e testes.

### RenderSnapshot geral

O editor já possui `EditorRenderSnapshot`. O gameplay ainda passa objetos de domínio diretamente para a apresentação.

Objetivo:

```text
Runtime / Editor
      ↓
RenderSnapshot
      ↓
RendererFacade
      ↓
Vulkan
```

O renderer não deve conhecer `Player`, `Level`, `GameState` ou regras da campanha como tipos de domínio.

### Application / GameStateMachine / Simulation

`main.cpp` continua como orquestrador de demasiados subsistemas. A extração deve ser incremental:

```text
Application
 ├── GameStateMachine
 ├── Simulation
 ├── Campaign
 ├── Editor
 └── Presentation
```

Não fazer uma reescrita monolítica.

### Ownership Vulkan

Preferir RAII/ownership explícito. Componentes internos de `RendererFacade` não devem usar `new/delete` quando membros ou `std::unique_ptr` forem suficientes.

### LevelData

Runtime, editor e parser devem convergir para o mesmo modelo declarativo `LevelData`.

### Undo/Redo

Operações editoriais devem evoluir para comandos transacionais. Um drag completo deve produzir uma única operação lógica no undo stack.

## P1 — runtime e distribuição

### Paths

Separar:

```text
install/executable root → assets do jogo
user data root → settings/runs/saves/importados
```

Eliminar dependência do current working directory e evitar escrever runtime data em `Development/`.

### Source-size policy

Manter uma única ferramenta e uma única política. O gate atual é a autoridade; ferramentas legadas equivalentes devem ser removidas.

A métrica de tamanho é um guardrail, não uma métrica de qualidade arquitetural.

## P2 — qualidade e CI

A matriz documentada deve evoluir para:

```text
Linux normal
Linux ASan/UBSan
Linux headless Vulkan
Linux campaign
Windows build + tests
Windows game build
```

Adicionar:

- replay regression tick-by-tick;
- property/invariant tests;
- malformed level tests;
- swapchain/error-path tests onde possível;
- editor undo/redo tests;
- hardware/software capability matrix.

## P2 — render snapshot e escalabilidade do editor

`EditorRenderSnapshot` copia os AABB do documento para cada atualização. Isto é aceitável na escala atual, mas deve permanecer acompanhado por profiling antes de mudar para cache/incremental.

A separação futura pode distinguir:

```text
DocumentRenderData
InteractionRenderData
ValidationRenderData
```

se o snapshot se tornar demasiado acoplado.

## Não tratar como prioridade imediata

- ECS completo;
- CCD universal;
- networking/WebSockets;
- render graph complexo;
- otimizações de instrução sem profiling;
- arquitetura genérica de engine.

Estas técnicas só entram quando um problema medido as justificar.

## Critério para sair da auditoria

A base fica pronta para construção de conteúdo quando:

1. input é totalmente orientado a ações;
2. Level Editor tem uma única interpretação de viewport;
3. fixed timestep tem limite defensivo;
4. swapchain/queues/error paths são robustos;
5. adapter/renderer legado deixam de ser necessários;
6. gameplay usa `RenderSnapshot` em vez de objetos de domínio diretamente;
7. `LevelData` é a fronteira comum;
8. paths/runtime data são independentes do source tree;
9. CI cobre Windows + sanitizers + build do jogo conforme a capacidade da infraestrutura;
10. testes cobrem as novas fronteiras.

## Relação com a investigação

Consultar sempre:

- `docs/TECHNICAL_REFERENCES.md` — Vulkan, C++, profiling, hardware, packaging e testing;
- `docs/SCIENTIFIC_REFERENCES.md` — física, replay, level design, dificuldade e playtesting;
- `docs/RESEARCH_INDEX.md` — subsistema → evidência;
- `docs/PRODUCT_DECISIONS.md` — comportamento pretendido;
- `docs/ARCHITECTURE.md` — arquitetura-alvo;
- `docs/ROADMAP.md` — ordem e gates.
