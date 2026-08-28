# Arquitetura do ASCENDENDO

## Objetivo

O ASCENDENDO é um jogo 2D C++20 com um motor pequeno e específico do jogo construído sobre Vulkan + GLFW. A arquitetura deve privilegiar determinismo, baixo custo de runtime, testes sem GPU sempre que possível e uma release portátil Windows x64.

O projeto é um **game framework específico do ASCENDENDO**, não uma engine genérica. Só se criam abstrações genéricas quando resolvem acoplamento real.

## Direção arquitetural

```text
Application / process composition
├── Runtime
│   ├── GameSession
│   │   ├── GameStateMachine
│   │   ├── CampaignRuntime
│   │   ├── Level
│   │   ├── PhysicsWorld
│   │   ├── SimulationOrchestrator
│   │   ├── Player
│   │   └── EditorSession
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

A implementação atual ainda está parcialmente concentrada em `main.cpp`. O renderer legado `Renderer.cpp/.h` já não está presente; a consolidação seguinte é reduzir responsabilidades de `main.cpp` e expandir fronteiras explícitas de presentation apenas quando houver consumidores reais.

A primeira extração da entry point é `GameSession`: uma fronteira de runtime sem ownership Vulkan/presentation. A composição global de processo continua fora desta tranche; não existe ainda uma classe `Application` concreta.

## Fluxo por frame

A direção pretendida é:

```text
GLFW / hardware
      ↓
InputManager
      ↓
GameAction / KeyBindings
      ↓
GameSession
  ├── GameStateMachine
  ├── EditorSession
  ├── SimulationOrchestrator
  └── CampaignRuntime
      ↓
render-state extraction
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

`main.cpp` mantém o ciclo de vida do processo, bootstrap gráfico, resolução inicial de caminhos, composição de `GraphicsRuntime`/`PresentationRuntime`, polling de janela e submissão de frames.

A primeira fronteira de runtime é `logic::GameSession`. Ela concentra o estado mutável de gameplay/editor/campanha e a política de transições que atualmente estavam co-localizados no entry point.

```text
main.cpp
  ├── process / GLFW lifetime
  ├── RuntimePaths bootstrap
  ├── GraphicsRuntime
  ├── PresentationRuntime
  ├── InputManager / KeyBindings
  ├── Camera
  └── GameSession
          ├── GameStateMachine
          ├── CampaignRuntime
          ├── Level
          ├── PhysicsWorld
          ├── SimulationOrchestrator
          ├── Player
          └── EditorSession
```

`GameSession` não possui `Window`, `VulkanContext`, `Swapchain`, `RenderPass`, `Pipeline`, `RendererFacade`, `PresentationRuntime` ou `Camera`. Isto evita que a primeira extração misture domínio com ownership de GPU ou apresentação.

A decomposição deve continuar incrementalmente. `Application` permanece uma direção conceptual, não uma obrigação nominal: só deve ser criada quando existir uma responsabilidade de composição/lifecycle claramente isolável.

## Renderer e `RenderSnapshot`

O Gate 9.6 está formalmente fechado. A primeira tranche da migração `RenderSnapshot` foi integrada no PR #129.

### Estado atual

O world/player path de `RendererFacade::drawFrame()` e `WorldRenderer::draw()` já recebe um `gfx::RenderSnapshot` em vez de consultar diretamente `logic::Player` e `logic::Level`. `EditorRenderSnapshot` continua a ser um contrato local separado para o editor.

### Fronteira atual

```text
Player / Level runtime state
          ↓
  RenderSnapshotBuilder
          ↓
     RenderSnapshot
          ↓
 RendererFacade / WorldRenderer
```

`RenderSnapshot` é um value object transitório de presentation. Não contém:

- tipos de domínio (`Player`, `Level`, `Vec2`);
- `Camera` ou referências a objetos vivos;
- Vulkan handles/resources;
- ownership;
- lógica de gameplay;
- estado sem consumidor concreto no presentation path.

A primeira tranche ficou deliberadamente limitada ao world/player path. O contrato efetivamente implementado é:

```text
RenderSnapshot
├── player { bounds, facingDirection }
├── platforms[] { x, y, width, height }
└── flag { visible, x, y, width, height }
```

A `Camera` permanece separada por ser estado de presentation e transformação world→NDC.

### Regra de construção

A construção do snapshot copia/transforma estado existente; não introduz regras novas de physics, gameplay ou campaign. O resultado é determinístico para o mesmo estado de runtime.

`main.cpp` constrói o snapshot apenas nos estados `PLAYING`/`PAUSED`, quando o world path é consumido. Outros estados não fazem a cópia da geometria do mundo.

### Próxima expansão

A existência desta fronteira não implica um snapshot global único. `UiRenderer`, editor e outros passes devem ser avaliados individualmente. Um novo snapshot só deve ser criado quando a separação trouxer uma redução verificável de acoplamento, melhorar testabilidade ou estabelecer um contrato necessário entre subsistemas.

Não migrar editor, menus, replay e world rendering numa única mudança apenas para obter uma interface uniforme.

## Modelo comum de níveis

Runtime e editor convergem para um `LevelData` declarativo e independente de Vulkan/GLFW:

```text
.lvl / campaign source
        ↓
      LevelData
      ├── Runtime view
      └── Editor view
```

A responsabilidade atual de `LevelDataIO` é **parse/serialize**, não validação de schema completa. O parser não deve ser usado como autoridade de UGC ou como substituto da validação futura.

`Level` é atualmente um **modelo de mundo acumulado**: `appendFromData()` adiciona a geometria de cada chunk a `m_platforms`, usando `LOGICAL_HEIGHT` como avanço vertical. Um objeto `Level` não é semanticamente equivalente a uma única entrada da campanha.

Metadados como `name`, `hasFlag` e `flagBounds` ainda têm escopo de chunk/último `appendFromData()` enquanto a geometria tem escopo do mundo acumulado. Esta assimetria é dívida de modelo e deve ser resolvida por uma decisão de escopo explícita, não por patches de campos isolados.

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

As arestas `justPressed`/`justReleased` continuam atualmente amostradas no **render frame**. O fixed-step pode consumir vários ticks por frame; por isso não se deve assumir equivalência entre frame-edge e tick-edge em replay autoritativo. Um futuro contrato tick-exact deve usar um comando indexado por tick ou outra regra temporal explicitamente definida.

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

Importante: a resolução atual percorre plataformas sequencialmente e muta o corpo após cada contacto. Portanto a ordem de `Level::platforms()` é potencialmente parte da função de transição quando existem múltiplos contactos relevantes. Até existir uma decisão formal, não assumir que permutar a representação interna é semanticamente neutro.

## Vulkan

A seleção de device/queues deve validar explicitamente:

- graphics queue;
- present queue;
- `VK_KHR_swapchain`;
- features obrigatórias;
- surface formats/present modes/capabilities.

Graphics e present podem coincidir ou ser queues diferentes.

A capability matrix executável existente não equivale a cobertura de recovery. `GraphicsRuntime::init()` pode falhar depois de inicializar membros; a política de one-shot/retry e o rollback completo devem ser explicitados antes de depender de reinitialização.

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

## Bootstrap

`RuntimeBootstrap` (PR #76) é uma **fronteira de composição de startup**, não uma nova camada de runtime.

```text
RuntimePaths
    + user-data preparation
    + CampaignLoader
    + CampaignID
            ↓
      RuntimeBootstrapResult
```

Não deve possuir frame loop, `Camera`, Vulkan, presentation, `GameSession` ou estado mutável de gameplay.

A implementação atual ainda permite que `CampaignLoader` e `CampaignID` interpretem o source da campanha independentemente. O bootstrap reduz acoplamento do `main.cpp`, mas não fecha a arquitetura de campanha. Antes de UGC/web, a campanha deve convergir para uma autoridade documental/canónica comum.

## Configuração

`Config.h` é atualmente transversal demais. Novas constantes devem ser organizadas por domínio (`physics`, `render`, `window`, `editor`, `gameplay`).

## Editor

A fronteira atual é:

```text
InputManager
    ↓
GameSession / EditorSession
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

O workflow atual fornece evidência observável de source-size, Vulkan headless, build/testes e campaign validation em Linux. O workflow separado de ASan/UBSan também está integrado. Windows e matriz de hardware continuam a depender dos testes reais do runner.

A contagem de testes é evidência de cobertura executável, mas não substitui testes de invariantes, malformed input, boundaries, runtime paths e falhas de inicialização.

## Formatos e partilha futura

O formato `.lvl` deve ter versão explícita antes de save/import público:

```text
VERSION 1
...
```

Na Fase 10, o fluxo deve ser:

```text
parse
→ validate envelope
→ identify schema version
→ migrate known old version
→ validate migrated representation
→ normalize
→ runtime
```

Mapas devem permanecer declarativos, sem scripts, includes, paths arbitrários ou execução de código.
