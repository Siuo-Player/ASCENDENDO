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

A implementação atual ainda está parcialmente concentrada em `main.cpp`. O renderer legado `Renderer.cpp/.h` já não está presente em `main`; a consolidação seguinte é reduzir responsabilidades de `main.cpp` e fechar a fronteira `RenderSnapshot`.

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

`main.cpp` continua a concentrar inicialização, composição do runtime, estados, campanha, física, editor e persistência. Isto é dívida técnica prioritária.

A direção é extrair gradualmente:

- `Application`: ciclo de vida e composição;
- `GameStateMachine`: estados e transições;
- `Simulation`: fixed timestep e ticks de gameplay;
- `EditorSession`: estado/interação do editor;
- `RendererFacade` / passes: apresentação.

A extração deve ser incremental e testada, não uma reescrita completa do motor.

A decomposição deve seguir responsabilidades. O limite de tamanho de ficheiros é apenas um sinal para iniciar esta análise; não é uma razão para criar ficheiros artificiais.

## Renderer e `RenderSnapshot`

A arquitetura integrada em `main` já não usa `Renderer.cpp/.h`, mas a fronteira de dados entre runtime e presentation ainda não está completa.

Atualmente, `WorldRenderer` recebe diretamente `logic::Player` e `logic::Level`, além da `Camera`. Esta dependência mantém presentation acoplada aos modelos de domínio.

A direção é:

```text
Player / Level / GameState
          ↓
     RenderSnapshot
          ↓
    RendererFacade
          ↓
    rendering passes
```

O snapshot contém dados compactos, transitórios e próprios de presentation; não possui Vulkan resources, lógica de jogo nem tipos de domínio.

PR #20 (`refactor(renderer): migrate WorldRenderer to RenderSnapshot`) é o work item atual desta migração. Até ser integrada, esta fronteira deve ser considerada **em transição**, não concluída.

Exemplo de contrato:

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

O workflow de `main` ainda agrega `make clean`, `make game` e `make tests-verbose` num único step; esta limitação de observabilidade está documentada em `docs/CI.md`.

Run #281 falhou no step agregado de build/teste, mas a causa detalhada permanece desconhecida sem diagnóstico observável. O número de assertions não é uma métrica suficiente; devem ser cobertas invariantes, malformed input, boundaries, runtime paths e falhas de inicialização.

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

## Arquitetura ↔ planeamento

Arquitetura e planeamento são tratados como dois lados da mesma estrutura de engenharia. O work breakdown não deve ser definido independentemente das fronteiras técnicas: mudanças arquiteturais podem criar ou remover work packages, alterar dependências e exigir novos critérios de validação.

```text
Arquitetura
   ↕
WBS / roadmap
   ↕
Branch / PR
   ↕
Testes + documentação
```

Cada work package deve declarar as dependências relevantes, os consumidores afetados e o seu critério de saída. Uma dependência não é considerada gerida apenas porque está expressa no código; também é necessário conhecer o impacto da sua alteração sobre consumidores, testes e documentação.

A modularidade deve ser avaliada pela redução de responsabilidades e dependências relevantes, não pelo número de classes. A literatura de socio-technical congruence mostra que modularização técnica não representa por si só todas as dependências de trabalho que exigem coordenação.

Mudanças de arquitetura, de processo ou de gate devem ser documentadas antes da implementação correspondente. Para o processo operacional, ver `docs/DEVELOPMENT_PROTOCOL.md` e `docs/PROJECT_MANAGEMENT.md`.

**Referências:** Tausworthe, *The Work Breakdown Structure in Software Project Management* (1979); Cataldo, Herbsleb & Carley, *Socio-technical congruence* (2008); Bick et al., *Coordination challenges in large-scale software development* (2018); Kruchten et al., *Building up and Exploiting Architectural Knowledge* (2005).