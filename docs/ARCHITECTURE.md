# Arquitetura do ASCENDENDO

## Visão geral

O projeto é um jogo 2D C++20 com um motor pequeno e explícito construído sobre Vulkan + GLFW. A simulação usa fixed timestep de 60 Hz e a renderização é separada da lógica de jogo.

A divisão principal é:

```text
main.cpp
  ├── Core      → configuração, ações, bindings, viewport, campaign ID
  ├── Logic     → input, física, jogador, níveis, replay, histórico
  └── Graphics  → janela, Vulkan, swapchain, pipelines, renderer, câmera
```

## Fluxo por frame

1. `InputManager::beginFrame()` limpa estados transitórios.
2. GLFW entrega eventos de teclado/rato.
3. `main.cpp` converte os eventos em ações através de `KeyBindings`.
4. O estado do jogo decide a máquina de estados: `PLAYING`, `PAUSED`, `CREDITS`, `MENU` ou `EDITOR`.
5. Em `PLAYING`, `PhysicsWorld::advance()` acumula tempo e determina quantos passos fixed-step devem correr.
6. `Player::update()` aplica input e atualiza a física; `Level::resolveCollision()` resolve as plataformas.
7. A câmera acompanha o jogador em gameplay ou é livre no editor.
8. `Renderer::drawFrame()` grava comandos Vulkan e apresenta o frame.

## Responsabilidades

### `Game/Core`

- `Config.h`: invariantes globais e parâmetros de gameplay/editor.
- `GameAction.*`: API lógica independente de teclas físicas.
- `KeyBindings.*`: mapeamento e persistência das teclas.
- `Viewport.*`: transformação window → logical space e hit-testing.
- `CampaignID.h`: identificador determinístico de campanha.

### `Game/Logic`

- `InputManager`: estados current/justPressed/justReleased para teclado e rato.
- `Physics`: `Vec2`, `AABB`, gravidade, fixed timestep e limites absolutos.
- `Player`: Commitment Jump e movimento horizontal.
- `Level`: representação das entidades jogáveis e colisões.
- `ReplayManager`: save states/replay determinístico.
- `RunHistory`: persistência de runs terminadas.

### `Game/Graphics`

- `VulkanContext`: instance, physical device, logical device, queue e surface.
- `Window`: integração GLFW.
- `Swapchain`/`RenderPass`: apresentação Vulkan.
- `Pipeline`: desenho de geometria simples via push constants.
- `TextPipeline`/`FontRenderer`: texto TTF em atlas.
- `SpritePipeline`/`SpriteRenderer`: sprites reutilizáveis.
- `Camera`: transformação de espaço do mundo.
- `Renderer`: único ponto de gravação dos comandos gráficos por frame.

## Regras de dependência

A lógica de jogo não deve conhecer detalhes de Vulkan. `Renderer` recebe estado e dados de `Logic` e converte-os em comandos gráficos.

`Game/Core` deve conter abstrações reutilizáveis e não deve depender de `Renderer`.

`main.cpp` é atualmente o orquestrador. Uma futura refatoração pode extrair `GameApp`/`EditorController`, mas não deve ser feita durante a Fase 9.4 sem necessidade concreta.

## Recursos e ownership

Objetos Vulkan seguem RAII: o proprietário destrói o recurso no seu destrutor. Referências como `Renderer::m_ctx` e `Renderer::m_swapchain` não possuem os objetos; estes vivem em `main.cpp` por mais tempo.

Texturas e sprites devem ser partilhados por referência. Entidades de nível devem guardar dados compactos (geometria + IDs), não cópias de bytes de texturas.

## Máquina de estados

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
   └── Pause/ESC → MENU
```

Durante `EDITOR`, a física do jogador não avança. O editor manipula um documento de nível e usa a mesma resolução lógica 640×360 e o mesmo grid de snap para produzir ficheiros que o motor consegue carregar diretamente.