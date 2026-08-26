# Arquitetura do ASCENDENDO

## Objetivo

O ASCENDENDO é um jogo 2D C++20 com um motor pequeno e específico do jogo construído sobre Vulkan + GLFW. A arquitetura deve privilegiar determinismo, baixo custo de runtime, testes sem GPU sempre que possível e uma release portátil Windows x64.

O projeto é um **game framework específico do ASCENDENDO**, não uma engine genérica. Só se criam abstrações genéricas quando resolvem acoplamento real.

## Direção arquitetural

```text
Application
├── Runtime
│   ├── GameStateMachine
│   ├── Simulation
│   │   ├── Physics
│   │   ├── Player
│   │   └── LevelRuntime
│   ├── Campaign
│   └── Replay / RunHistory
│
├── Editor
│   ├── EditorSession
│   ├── LevelData / LevelEditorDocument
│   ├── EditorInteractionController
│   └── Commands / UndoRedo
│
├── Presentation
│   ├── RenderSnapshot
│   ├── UI/HUD data
│   └── RendererFacade / Vulkan passes
│
├── Core
│   ├── ViewportTransform
│   ├── Input / GameAction / KeyBindings
│   ├── domain configuration
│   └── Paths / asset resolution
│
└── Services
    ├── Level IO / Validation
    ├── Asset loading
    └── Local user data
```

A implementação atual ainda está parcialmente concentrada em `main.cpp`. O renderer legado `Renderer.cpp/.h` já não está presente; a consolidação seguinte é reduzir responsabilidades de `main.cpp` e fechar a fronteira `RenderSnapshot`.

## Fluxo por frame

A direção pretendida é:

```text
GLFW / hardware
      ↓
InputManager
      ↓
GameAction / KeyBindings
      ↓
GameStateMachine / EditorSession
      ↓
Simulation / LevelData / EditorDocument
      ↓
RenderSnapshot
      ↓
RendererFacade
      ↓
World / UI / Editor passes
      ↓
Vulkan
```

O renderer não deve decidir regras de gameplay, editor ou campanha.

## `main.cpp` e estado da aplicação

`main.cpp` ainda concentra inicialização, composição do runtime, estados, campanha, física, editor, streaming, persistência e submissão de frames. Esta é a próxima dívida arquitetural prioritária do Base Engineering Gate.

A extração deve ser incremental:

- `Application`: ciclo de vida/composição;
- `GameStateMachine`: estados e transições;
- `Simulation`: fixed timestep e ticks de gameplay;
- `EditorSession`: estado/interação do editor;
- `RendererFacade` / passes: apresentação.

A decomposição deve seguir responsabilidades reais e ownership/lifetime, não o limite de linhas. Não fazer uma reescrita completa do motor.

O work package atual está registado em `docs/WORK_PACKAGE_MAIN_LOOP_DECOMPOSITION.md`.

## Renderer e `RenderSnapshot`

A arquitetura integrada já não usa `Renderer.cpp/.h`, mas a fronteira de dados entre runtime e presentation ainda não está completa.

Atualmente, a presentation continua a receber dados de domínio diretamente em partes do caminho de rendering. A direção é:

```text
Player / Level / GameState
          ↓
     RenderSnapshot
          ↓
    RendererFacade
          ↓
    rendering passes
```

O snapshot deve conter dados compactos, transitórios e próprios de presentation; não deve possuir Vulkan resources, lógica de jogo nem tipos de domínio.

O PR #20 foi encerrado/superseded e não é uma tranche ativa. O histórico do PR/commits é suficiente para comparação; não manter uma branch dele apenas como laboratório permanente.

## Modelo comum de níveis

Runtime e editor devem convergir para um `LevelData` declarativo e independente de Vulkan/GLFW:

```text
.lvl / campaign source
        ↓
      LevelData
      ├── Runtime view
      └── Editor view
```

Isto evita que `Level` e `LevelEditorDocument` evoluam como dois modelos concorrentes do mesmo conteúdo.

## Input

O fluxo correto é único:

```text
hardware
   ↓
InputManager
   ↓
KeyBindings
   ↓
GameAction
   ↓
Gameplay / Editor
```

Gameplay não deve consultar `Key::SPACE`, `Key::A`, etc. diretamente.

## Tempo de simulação

O fixed timestep de 60 Hz permanece. O sistema deve impedir recuperação ilimitada depois de um frame muito longo/minimização.

Requisitos:

- limite de passos de simulação por frame;
- clamp de `dt` quando necessário;
- nenhuma entrada `NaN`/`Inf` na simulação;
- determinismo preservado para replay.

## Física e colisão

`Physics` deve possuir a política temporal e o estado físico; `Level` deve fornecer geometria/dados de nível, não concentrar decisões específicas de resposta física.

A resolução atual baseada em penetration depth/velocidade é adequada ao jogo atual, mas deve ser tratada como uma implementação de gameplay, não como um resolvedor geométrico universal.

## Vulkan

A seleção de device/queues deve validar explicitamente:

- graphics queue;
- present queue;
- `VK_KHR_swapchain`;
- features obrigatórias;
- surface formats/present modes/capabilities.

Graphics e present podem coincidir ou ser queues diferentes.

Wrappers Vulkan devem ser não-copiáveis e, quando necessário, movíveis com ownership claro.

## Paths, assets e dados do utilizador

Runtime não pode depender do current working directory.

A resolução deve distinguir:

```text
install/executable root
  → assets, shaders, fonts, levels do jogo

user data root
  → settings, runs, saves, mapas importados e temporários
```

Isto é requisito para a futura build portable.

## Configuração

`Config.h` é atualmente transversal demais. Novas constantes devem ser organizadas por domínio (`physics`, `render`, `window`, `editor`, `gameplay`).

## Editor

A fronteira atual é:

```text
InputManager
    ↓
EditorSession
    ↓
EditorInteractionController
    ↓
LevelEditorDocument / LevelData
```

Operações de edição devem evoluir para comandos transacionais. Um drag completo deve ser uma operação lógica única.

## Testes e CI

A matriz mínima pretendida é:

```text
Linux normal tests
Linux ASan/UBSan
Linux headless Vulkan smoke
Linux campaign validation
Windows build + tests
Windows game build/link
```

O workflow atual já fornece evidência observável de source-size, Vulkan headless, build/testes e campaign validation em Linux. Windows, sanitizers e matriz de hardware continuam a faltar.

A suite recente validada contra o estado atual produz **167 test cases e 901 assertions**, mas a contagem não substitui testes de invariantes, malformed input, boundaries, runtime paths e falhas de inicialização.

## Formatos e partilha futura

O formato `.lvl` deve ter versão explícita antes de save/import público:

```text
VERSION 1
...
```

Mapas devem permanecer declarativos, sem scripts, includes, paths arbitrários ou execução de código.
