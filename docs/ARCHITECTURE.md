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
│   └── Renderer / Vulkan
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

A implementação atual ainda está parcialmente concentrada em `main.cpp` e `Renderer.cpp`. A consolidação destas fronteiras é uma tarefa explícita do roadmap e não deve ser adiada para depois de save/import/share.

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
Renderer
      ↓
Vulkan
```

O renderer não deve decidir regras de gameplay, editor ou campanha.

## `main.cpp` e estado da aplicação

`main.cpp` é atualmente o orquestrador e ainda contém inicialização Vulkan, campanha, física, máquina de estados, menus e editor. Isto é dívida técnica prioritária.

A direção é extrair gradualmente:

- `Application`: ciclo de vida e loop;
- `GameStateMachine`: estados e transições;
- `Simulation`: fixed timestep e ticks de gameplay;
- `EditorSession`: estado/interação do editor;
- `Renderer`: apresentação.

A extração deve ser incremental e testada, não uma reescrita completa do motor.

## Renderer e `RenderSnapshot`

A API atual recebe diretamente `Player`, `Level`, `Camera`, `GameState`, seleção de menu e timer. Isso funciona, mas cria acoplamento entre presentation e runtime.

A direção é introduzir um snapshot de renderização, por exemplo:

```cpp
struct RenderSnapshot {
    CameraRenderData camera;
    std::vector<PlatformRenderData> platforms;
    PlayerRenderData player;
    HudRenderData hud;
    MenuRenderData menu;
    EditorRenderData editor;
};
```

O snapshot contém dados compactos e transitórios; não possui Vulkan resources nem lógica de jogo.

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

Gameplay não deve consultar `Key::SPACE`, `Key::A`, etc. diretamente. A dívida atual em `Player` deve ser removida antes de funcionalidades de input adicionais.

## Tempo de simulação

O fixed timestep de 60 Hz permanece. O sistema deve, porém, impedir uma recuperação ilimitada depois de um frame muito longo/minimização.

Requisitos:

- limite de passos de simulação por frame;
- clamp de `dt` quando necessário;
- nenhuma entrada `NaN`/`Inf` na simulação;
- determinismo preservado para replay.

## Física e colisão

`Physics` deve possuir a política temporal e o estado físico; `Level` deve fornecer geometria/dados de nível, não concentrar decisões específicas de resposta física.

A resolução atual baseada em penetration depth/velocidade é adequada ao jogo atual, mas deve ser tratada como uma implementação de gameplay, não como um resolvedor geométrico universal. Continuous collision detection só entra quando o design exigir velocidades/entidades que possam sofrer tunneling.

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

`Config.h` é atualmente transversal demais. Novas constantes devem ser organizadas por domínio (`physics`, `render`, `window`, `editor`, `gameplay`). A migração pode ser incremental.

## Editor

A fronteira atual é a correta:

```text
InputManager
    ↓
EditorSession
    ↓
EditorInteractionController
    ↓
LevelEditorDocument / LevelData
```

Operações de edição devem evoluir para comandos transacionais:

```text
CreatePlatform
MovePlatform
ResizePlatform
DeletePlatform
        ↓
undoStack / redoStack
```

Um drag completo deve ser uma operação lógica única.

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

O número de assertions não é uma métrica suficiente; devem ser cobertas invariantes, malformed input, boundaries, runtime paths e falhas de inicialização.

## Formatos e partilha futura

O formato `.lvl` deve ter versão explícita antes de save/import público:

```text
VERSION 1
...
```

Mapas devem permanecer declarativos, sem scripts, includes, paths arbitrários ou execução de código.

A validação local do EXE é sempre a autoridade final antes de um mapa ser jogável.

## Máquina de estados atual

```text
PLAYING
   ├── Pause → PAUSED
   └── FLAG final → CREDITS → MENU

PAUSED
   ├── Continue → PLAYING
   └── Credits → CREDITS → PAUSED

MENU
   ├── Start → PLAYING
   ├── Editor → EDITOR
   └── Credits → CREDITS → MENU

EDITOR
   └── ESC → MENU
```

A futura extração para `GameStateMachine` deve manter estas transições e torná-las testáveis sem Vulkan.
