# ASCENDENDO

**Motor de jogo 2D customizado + Vertical Precision Platformer**
**Autor:** Rafael Gomes Bernardo | **Auxiliado por:** Claude (Anthropic)
**Repositório:** https://github.com/Siuo-Player/ASCENDENDO

---

## 1. Sobre o Projeto

Motor 2D com renderização Vulkan explícita e física determinística em Fixed Timestep. Cada frame de input é gravável em replay e cada estado da simulação é revertível via save states. O jogo foca-se em **salto parabólico com inércia (Commitment Jump)** — ângulo fixo de 60°, sem controlo aéreo — e em dificuldade gerada e validada algoritmicamente.

---

## 2. Estrutura do Projeto

```
ASCENDENDO/
├── .vscode/
│   ├── c_cpp_properties.json          ← IntelliSense (Vulkan, GLFW, doctest)
│   └── tasks.json                     ← atalhos de build no VS Code
├── Game/
│   ├── Core/
│   │   ├── Config.h                   ← constantes globais (LOGICAL_WIDTH/HEIGHT, física, EDITOR_GRID_SNAP)
│   │   ├── CampaignID.h               ← ID determinístico (FNV-1a) p/ diferenciar campanhas
│   │   ├── GameAction.h/.cpp          ← acções lógicas (MoveLeft, Pause, Quit, ...), independentes de tecla
│   │   ├── KeyBindings.h/.cpp         ← GameAction → tecla(s) físicas, persistência em controls.cfg
│   │   └── Viewport.h/.cpp            ← janela→espaço lógico (letterbox), hit-test das caixas de MENU/PAUSED
│   ├── Graphics/                      ← motor gráfico Vulkan
│   │   ├── VulkanContext.h/.cpp
│   │   ├── Window.h/.cpp              ← janela GLFW
│   │   ├── Swapchain.h/.cpp
│   │   ├── RenderPass.h/.cpp
│   │   ├── Renderer.h/.cpp            ← drawFrame(Player,Camera,Level*,State); attachText()
│   │   ├── Camera.h/.cpp              ← projeção world→NDC, tracking vertical
│   │   ├── Pipeline.h/.cpp            ← shaders SPIR-V + viewport dinâmico (retângulos)
│   │   ├── TextPipeline.h/.cpp        ← pipeline dedicada p/ texto (descriptor set + sampler)
│   │   ├── FontRenderer.h/.cpp        ← baking stb_truetype + atlas GPU (texto TTF real)
│   │   └── BitmapFont.h               ← fallback 5×5 (usado se TTF indisponível)
│   ├── Logic/                         ← motor de jogo
│   │   ├── InputManager.h/.cpp
│   │   ├── Physics.h/.cpp             ← Vec2, AABB, Fixed Timestep 60Hz
│   │   ├── Player.h/.cpp              ← Commitment Jump (60°), bloqueio aéreo
│   │   ├── Level.h/.cpp               ← plataformas + resolveCollision + streaming
│   │   ├── ReplayManager.h/.cpp       ← save states + replay determinístico
│   │   └── RunHistory.h               ← regista runs completas (Development/Runs/runs.csv)
│   └── Assets/
│       ├── Shaders/
│       │   ├── base.vert / base.frag  ← GLSL retângulos sólidos (push constants)
│       │   ├── text.vert / text.frag  ← GLSL texto (UV + atlas sampler)
│       │   └── *.spv                  ← compilados por glslc [build artifact]
│       ├── Fonts/
│       │   └── UIFont.ttf             ← fonte TTF (Orbitron, OFL — substituível)
│       ├── Sprites/
│       │   ├── personagem.png         ← sprite do protagonista (de .pixil, optimizado optipng)
│       │   └── Source/
│       │       └── *.pixil            ← originais Pixilart (preservados p/ reedição)
│       └── Levels/
│           ├── campaign.txt           ← lista ordenada de niveis da campanha activa
│           ├── inicio.lvl             ┐
│           ├── zigzag.lvl             ├── niveis da campanha activa
│           ├── precipicio.lvl         ┘
│           ├── Unused/                ← niveis validos fora da campanha
│           └── NaoValidados/          ← niveis em construcao / fisicamente invalidos
├── Development/
│   ├── dev_log.txt                    ← diário append-only
│   ├── project_structure.txt          ← mapa de arquitectura (auto-gerado)
│   ├── Runs/
│   │   └── runs.csv                   ← historico de runs completas (gerado em runtime)
│   ├── Settings/
│   │   └── controls.cfg               ← teclas reatribuídas (gerado por uma futura UI CONTROLS; se ausente, usa defaults)
│   ├── AI_Validation/
│   │   ├── ai_validator.py            ← validador BFS (--campaign bloqueia pre-push)
│   │   └── sim/                       ← simulador Python fiel ao motor C++
│   │       ├── engine.py              ← Body, physics_step, resolve_collision
│   │       ├── solver.py              ← find_charge_for_target + robustness_test
│   │       ├── flag_solver.py         ← verificação FLAG por overlap durante o voo
│   │       └── levelgen.py            ← gerador de niveis por simulação directa
│   └── LevelEditor/
│       └── level_validator.py         ← validador standalone por linha de comando
├── Tests/
│   ├── test_runner.cpp                ← define main via DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
│   ├── log_testes.txt                 ← output do último make tests (gerado automaticamente)
│   ├── Unit/
│   │   ├── test_placeholder.cpp
│   │   ├── test_vulkan_context.cpp
│   │   ├── test_input.cpp
│   │   ├── test_physics.cpp
│   │   ├── test_player.cpp
│   │   ├── test_replay.cpp
│   │   ├── test_camera.cpp
│   │   ├── test_level.cpp             ← 16 test cases (Level + appendFromFile)
│   │   ├── test_campaign.cpp          ← chama ai_validator --campaign via system()
│   │   ├── test_campaign_id.cpp       ← 7 testes (CampaignID: determinismo, sensibilidade)
│   │   ├── test_run_history.cpp       ← 3 testes (RunHistory: formatação, CSV append)
│   │   ├── test_game_action.cpp       ← 8 testes (GameAction: tabela de acções, round-trip)
│   │   ├── test_keybindings.cpp       ← 21 testes (KeyBindings: defaults, persistência, InputManager real)
│   │   ├── test_viewport.cpp          ← 15 testes (letterbox, conversão de coordenadas, hit-test de menu)
│   │   └── test_input_mouse.cpp       ← 7 testes (botões de rato + cursor, InputManager real)
│   └── Integration/
│       ├── test_vulkan_init.cpp
│       ├── test_window.cpp
│       ├── test_swapchain.cpp
│       ├── test_render_pass.cpp
│       ├── test_renderer.cpp
│       ├── test_pipeline.cpp
│       └── test_text_pipeline.cpp     ← TextPipeline + FontRenderer (2 testes)
├── external/
│   ├── doctest/doctest.h              ← framework de testes header-only v2.5.0
│   ├── stb/stb_truetype.h             ← baking de fontes TTF (header-only, MIT)
│   └── glfw/ (include/ + lib-vc2022/)
├── scripts/
│   ├── pre-commit.sh                  ← bloqueia commits com testes a falhar
│   └── pre-push.sh                    ← valida campanha com ai_validator antes do push
├── build/                             ← artefactos de compilação [IGNORADO NO GIT]
├── main.cpp                           ← entry point do jogo (loop RAII + streaming)
├── Makefile
├── deps.py
└── reorganize.py                      ← organiza ficheiros + valida e roteia niveis .lvl
```

### Routing automático de níveis (`reorganize.py`)

| Estado do `.lvl` | Destino |
|---|---|
| Válido + em `campaign.txt` | `Game/Assets/Levels/` |
| Válido + **não** em `campaign.txt` | `Game/Assets/Levels/Unused/` |
| Inválido / incompleto | `Game/Assets/Levels/NaoValidados/` |

---

## 3. Gestão de Dependências

Nenhuma dependência é adicionada sem justificação no `dev_log.txt` e sem pedido explícito de permissão.

| Biblioteca | Versão | Localização | Justificação |
|---|---|---|---|
| [doctest](https://github.com/doctest/doctest) | 2.5.0 | `external/doctest/` | Header-only, zero overhead, MIT |
| [stb_truetype](https://github.com/nothings/stb) | single-header | `external/stb/` | Baking de fontes TTF para atlas, MIT/Public Domain |
| [stb_image](https://github.com/nothings/stb) | single-header | `external/stb/` | Descodificação de PNG para sprites, MIT/Public Domain |
| [GLFW](https://www.glfw.org/) | 3.4 | `external/glfw/` | Windowing Vulkan cross-platform, zlib |
| Vulkan SDK | 1.4.x | sistema (`VULKAN_SDK`) | API gráfica principal + `glslc` para shaders |

---

## 4. Regras de Teste e Versionamento

- **Pre-commit**: bloqueia commits com qualquer teste a falhar (`make tests`).
- **Pre-push**: valida a campanha completa com `ai_validator.py --campaign`.
- **TDD**: testes escritos na fase "Ideia", antes da implementação.
- **Imutabilidade**: testes antigos nunca são alterados para acomodar código novo.
- **Testes confirmados**: **110/110** (Fase 9.1, confirmado por execução real de Rafael — `make tests-verbose -j8`, Windows/Clang22, commit `fe8f924`), 485 assertions. **+22 testes / +61 assertions** (`test_viewport.cpp` + `test_input_mouse.cpp`, Fase 9.2) verificados nesta sessão — compilados e corridos a sério, motor inteiro ligado com sucesso contra Vulkan/GLFW reais. Total **esperado 132/132, 546 assertions** — a confirmar no próximo `make tests-verbose`.

---

## 5. Diário de Desenvolvimento

`Development/dev_log.txt` — append-only, nunca editar entradas anteriores.

```
[YYYY-MM-DD HH:MM:SS]
Ideia: [o que vamos implementar e o comportamento esperado]
Implementado: [como correu, problemas e soluções]
```

---

## 6. Planeamento de Desenvolvimento

**Fase 1: Ambiente e Infraestrutura** ✅
Makefile cross-platform, doctest, pre-commit hook, deps.py.

**Fase 2: Motor Gráfico Base (Vulkan)** ✅
`VulkanContext`, `Window` (GLFW), `Swapchain`, `RenderPass`, `Renderer` → primeira janela colorida.

**Fase 3: Física, Input e Replay** ✅
`InputManager`, `Physics` (Vec2, AABB, Fixed Timestep 60Hz, GRAVITY=-980),
`Player` (Commitment Jump 60°, MIN_JUMP=250/MAX_JUMP=600 px/s),
`ReplayManager` (save states + replay determinístico).

**Fase 4: Câmera e Viewport** ✅
`Camera` — projeção world→NDC, tracking vertical com Lerp (jogador no 35% inferior do ecrã).

**Fase 5: Pipeline Gráfica** ✅
Shaders GLSL → SPIR-V, `Pipeline` Vulkan (viewport/scissor dinâmicos),
`Config.h` (LOGICAL_WIDTH=640, LOGICAL_HEIGHT=360), `Renderer` integra Player + Camera.

**Fase 6: Mecânicas de Jogo** ✅
- 6.1 Commitment Jump — ângulo fixo 60°, bloqueio aéreo total
- 6.2 `Level` — plataformas AABB, colisão MTV (topo one-way + lados sólidos)
- 6.2b Renderer renderiza plataformas
- 6.2c Física corrigida — paredes absolutas X, queda de borda sem inércia
- 6.3 Barra de força — UI Verde→Vermelho (isCharging)
- 6.4 Camera tracking — follow() Lerp vertical

**Fase 7: Level Design e Campanha** ✅
- 7.0 Level Streaming — `appendFromFile()`, `campaign.txt`, `FLAG`, chunks de 360px
- 7.1 Refactoring config-driven — `PLAYER_WIDTH`, inércia natural, `physics::collides()`
- 7.2 Debug HUD — `[SPACE] Força: XX%` no terminal durante a carga
- 7.3 Fix crítico — chunks padronizados; spawn corrigido; LOGICAL_WIDTH/HEIGHT trocados nos validadores Python corrigidos
- 7.4 Níveis gerados por simulação — `levelgen.py` com `ascending_x_at()` (guard contra colisão lateral na subida, 100% robustez ±8%/±10px); `test_campaign.cpp` integra `ai_validator` no ciclo doctest; fixes Windows (Unicode, duplicate symbol)
- 7.5 GameState (PLAYING/CREDITS/MENU) — FLAG visual (mastro+pano+zona de touchdown), pedestal sólido sob a FLAG, ecrã de créditos, menu A/D+ESPAÇO, spawn corrigido (acima da 1ª plataforma)

**Fase 8: UI e Polishing** ✅
- 8.1 ✅ **Texto TTF real** — `TextPipeline` (pipeline Vulkan dedicada, descriptor set + sampler) + `FontRenderer` (baking via `stb_truetype`, atlas R8_UNORM na GPU). Substitui o `BitmapFont` pixelado em CREDITS/MENU (mantido como fallback automático). Fonte actual: Orbitron (OFL), facilmente substituível.
- 8.2 ✅ **Menu reestruturado** — `GameState::PAUSED` (ESC pausa sem terminar a run, mundo congelado visível por baixo de overlay+menu), botão SAIR em ambos os menus (fim-de-run e pausa), CRÉDITOS acessível dos dois (regressa a quem chamou), timer HUD (pausa fora de PLAYING), registo de runs (`Development/Runs/runs.csv`) com ID determinístico de campanha (`CampaignID.h`, FNV-1a)
- 8.3 ✅ **Fixes visuais + Sprites** — FLAG sem zona de touchdown (cor estranha), timer HUD corrigido (clipping no topo do ecrã dava aspecto de texto garbled), espaçamento de CREDITOS recalculado, testes flaky corrigidos (`remove_all` throwing→não-throwing, pastas de teste consolidadas em `build/test_tmp/`). `SpritePipeline`+`SpriteRenderer` (jogador em pixel-art via `.pixil`→PNG automático no `reorganize.py`, optimizado com `optipng`)

**Fase 9: Editor de Níveis Visual** *(em progresso — scope confirmado com Rafael, ver detalhe abaixo)*
Editor corre **dentro do próprio motor** (novo `GameState::EDITOR`, não uma ferramenta à parte) — decisão explícita de Rafael: "é uma parte essencial do jogo". Scope dividido em 6 sub-fases:
- 9.1 ✅ **Sistema de Configuração de Controlos** — `GameAction` (9 acções lógicas) + `KeyBindings` (mapeamento reatribuível, várias teclas por acção, `Development/Settings/controls.cfg`). Pause/UIConfirm/UILeft/UIRight ligados a `main.cpp`; nova acção Quit (default `Q`) sai directamente do PAUSED/MENU sem precisar navegar até "Sair". **Confirmado por execução real** (110/110, 485 assertions, commit `fe8f924`). **Pendente**: ecrã CONTROLS visual (listar + remapear "prime uma tecla"), e ligar MoveLeft/MoveRight/Jump ao gameplay real (exige tocar `Player.cpp`, testado e estável — aguarda confirmação explícita antes de mexer).
- 9.2 ✅ **Rato** — `InputManager` ganha posição do cursor + botões (`MouseButton::LEFT/RIGHT/MIDDLE`, mesmo padrão de injecção de eventos testável sem GLFW já usado para teclado). `Viewport.h/.cpp` (novo): conversão janela→espaço lógico com letterbox (espelha `Renderer.cpp`) + hit-test das 3 caixas de menu. **MENU e PAUSED já aceitam clique** — seleciona e confirma na mesma acção, como um botão normal. **22 testes novos, 61 assertions, verificados nesta sessão** (ver secção 4). **Nota para o futuro CONTROLS**: `hitTestMenuBox()` está escrito para exactamente 3 caixas fixas — generalizar para uma lista de N acções fica para quando esse ecrã for implementado.
- 9.3 **Estado EDITOR: infraestrutura e acesso** — câmara livre (sem física; `Player`/`PhysicsWorld` desligados), grelha de fundo desenhada com a pipeline de sólidos existente, acesso por tecla dedicada (via `KeyBindings`) *e* opção no MENU (para quem esquecer a tecla).
- 9.4 **Manipulação de entidades** — plataformas (colocar/mover/apagar, snap ao `EDITOR_GRID_SNAP` de `Config.h`, actualmente 4px para testes, candidato futuro 16px); spawn (Y sempre fixo no chão do 1º nível, X editável mas travado dentro dos limites seguros da própria plataforma); FLAG (**uma por campanha, não por nível** — só editável no nível que ocupar a última posição em `campaign.txt`).
- 9.5 **Guardar + Validar (assíncrono)** — grava `.lvl` no formato já usado por `Level::appendFromFile`; validação (`ai_validator.py`) corre em thread separada com barra de progresso e tempo estimado; o utilizador pode sair do editor enquanto corre, resultado chega depois como notificação.
- 9.6 **Gestão de campanha no editor** — lista de níveis arrastável (estilo *playlist*) dentro do próprio editor; reescreve apenas `campaign.txt` — não move ficheiros entre `Levels/`/`Unused/`/`NaoValidados/`, isso continua a cargo do `reorganize.py`.

**Nota v8.2 ainda pendente de encaixar numa sub-fase**: escolha de sprites diferentes (chão/plataforma/decoração), criação/edição de sprites, regras para troca de sprites — confirmado por Rafael como parte da Fase 9, mas ainda sem sub-fase própria atribuída.

**Ideia registada para o futuro, não aprovada nem escopada**: partilha de campanhas/níveis/runs entre máquinas ("multiplayer" no sentido de dados partilhados, não jogo em tempo real) — ver secção 6.1.

**Fase 10: Distribuição — "Release Build"** *(planeada — um dos últimos passos, definido por Rafael em v8.3)*

O jogo final deve ser um executável **portable e standalone**: o jogador corre-o com um duplo-clique, sem terminal visível, sem instalação, sem tocar em nada fora da sua própria pasta. Cinco regras obrigatórias, definidas antes de qualquer implementação:

| # | Regra | Notas técnicas |
|---|---|---|
| 1 | **Sem terminal** — o `.exe` corre como aplicação gráfica nativa, nunca abre uma janela de consola | Precisa de indicar ao *linker* que o subsistema é `WINDOWS`, não `CONSOLE`. A flag exacta depende do modo como o Clang está a invocar o *linker* neste projecto (`Target: x86_64-pc-windows-msvc` no stack actual): `-mwindows` é a flag GNU/MinGW clássica; para um *link* em modo MSVC pode ser antes `-Wl,/SUBSYSTEM:WINDOWS`. **Por confirmar qual se aplica quando chegar a altura** — não assumir já qual funciona sem testar. |
| 2 | **Pacote portable (sandboxing total)** — tudo contido na própria pasta, zero efeitos fora dela | Sem instalador, sem escrever fora da pasta do jogo. |
| 3 | **Paths relativos e locais** — nunca tocar em `AppData`, `Documentos`, ou no Registo do Windows | `Development/Runs/runs.csv` já usa caminho relativo — compatível com esta regra tal como está. Rever qualquer código futuro (save states, configurações) para a mesma disciplina. |
| 4 | **Falha graciosa + requisitos** — sem downloads automáticos para o jogador; se o hardware não tiver os requisitos (ex: sem suporte Vulkan), mostrar uma caixa de diálogo nativa e fechar de forma limpa | Ponto de entrada natural: `VulkanContext::init()` já devolve `bool` em caso de falha, mas o `main.cpp` actual **não verifica esse valor de retorno** — é aqui que a checagem e o `MessageBoxW` (Windows) entram. Precisa de compilação condicional (`#ifdef _WIN32`) para não partir o build Linux. Mensagem sugerida por Rafael: *"Desculpe, não dá para rodar o projeto sem os requisitos necessários."* Qualquer download obrigatório em contexto de programador (SDKs, dependências) exige consentimento explícito — nunca automático. |
| 5 | **README reestruturado** — secções claramente separadas "Para Desenvolvedores" vs "Para Jogadores" | A experiência do jogador final (consentimento, natureza *standalone*, como descarregar/correr sem instalações extra) fica isolada das instruções de build actuais (que continuam a assumir Clang/Vulkan SDK/GLFW — irrelevantes para quem só quer jogar). |

Nenhuma destas regras está implementada — este é o registo do plano, a implementar quando o jogo estiver funcionalmente completo.

---

## 6.1 Planos Futuros (ideias por aprovar)

Ideias levantadas por Rafael. As da secção "Menu e navegação" e "Timer e runs" abaixo foram **implementadas na v8.1** (ver Fase 8.2 acima) — mantidas aqui como registo histórico da decisão. A secção "Fase 9" tem agora scope confirmado (ver Fase 9 na secção 6 acima); mantém-se aqui só a ideia de partilha entre máquinas, ainda por aprovar.

**Menu e navegação — ✅ implementado (v8.1):**
- ~~Botão "Sair"~~ → implementado em ambos os menus (fim-de-run e pausa)
- ~~Menu de pausa via ESC~~ → `GameState::PAUSED`, distinto do menu de fim-de-run, com opções próprias (CONTINUAR/CRÉDITOS/SAIR)
- ~~Botão "Créditos" dentro do(s) menu(s)~~ → implementado, com retorno ao estado que chamou (`creditsReturnState`)
- Ainda por fazer (não imediato): o botão "Começar" evolui para um selector de campanhas (ver Fase 9)

**Timer e runs — ✅ implementado (v8.1):**
- ~~Cronómetro que pausa automaticamente~~ → `elapsedTime` só acumula em `PLAYING`
- ~~Registo de runs completadas~~ → `Development/Runs/runs.csv` (timestamp, campanha, ID, tempo)
- ~~Questão em aberto: diferenciar campanhas~~ → resolvida com `CampaignID.h` (FNV-1a 64-bit sobre `campaign.txt` + `.lvl`s). Decisão tomada sem confirmação prévia de Rafael — sinalizar se preferir outra abordagem.

**Fase 9 — Editor de Níveis Visual: scope confirmado.** Ver secção 6 acima (9.1 a 9.6) para o detalhe completo — deixou de estar por especificar.

**Partilha de campanhas/níveis/runs entre máquinas — ideia registada, NÃO aprovada nem escopada:**
Rafael levantou a possibilidade de, no futuro, permitir partilhar campanhas/níveis/runs entre jogadores diferentes ("multiplayer" no sentido de dados partilhados — não jogo em tempo real). Explicitamente **decidível mais tarde**, sem scope nem implementação nesta fase. Nota técnica para quando for retomado: `CampaignID.h` já foi desenhado com isto em mente (hash determinístico FNV-1a, independente de máquina — ao contrário de `std::hash`) e os `.lvl` são texto simples portável, por isso a arquitectura actual não fecha esta porta. Cuidado a ter no Editor (Fase 9) para não a fechar sem querer: evitar paths absolutos ou identificadores dependentes da máquina local nos ficheiros gerados.

Ver secção 6 acima para o roadmap completo do que está de facto construído.

---

## 7. Stack Técnico

| Componente | Escolha | Notas |
|---|---|---|
| Linguagem | C++20 | controlo de memória, compatibilidade Vulkan |
| Compilador | Clang++ 22 (LLVM) | `Target: x86_64-pc-windows-msvc` |
| Build | GNU Make | portável via Git Bash / MSYS2 |
| API Gráfica | Vulkan 1.4 | explícita, determinística |
| Windowing | GLFW 3.4 | zlib license |
| Testes | doctest 2.5.0 | header-only, zero instalação |
| Shaders | GLSL → SPIR-V | compilado com `glslc` (Vulkan SDK) |
| Validação IA | Python 3.x | BFS com física real, sim/ fiel ao motor C++ |

---

## 8. Setup do Ambiente

### 8.1 Requisitos

| Ferramenta | Instalação |
|---|---|
| Clang++ ≥ 14 | `winget install LLVM.LLVM` + adicionar ao PATH |
| GNU Make | Git Bash (incluído) |
| Git | https://git-scm.com/ |
| Python ≥ 3.9 | `winget install Python.Python.3` |
| Vulkan SDK | https://vulkan.lunarg.com/sdk/home |
| GLFW 3.4 | https://www.glfw.org/download → extrair para `external/glfw/` |

### 8.2 Instalação

```bash
git clone https://github.com/Siuo-Player/ASCENDENDO && cd ASCENDENDO

# Instalar hooks de git
cp scripts/pre-commit.sh .git/hooks/pre-commit
cp scripts/pre-push.sh   .git/hooks/pre-push
chmod +x .git/hooks/pre-commit .git/hooks/pre-push

# Organizar ficheiros e validar níveis
python reorganize.py
```

### 8.3 Comandos de Desenvolvimento

```bash
# Compilar e testar (captura stdout + stderr no log)
python reorganize.py && make clean && make tests-verbose -j8 > Tests/log_testes.txt 2>&1

# Ver resultado do log
cat Tests/log_testes.txt

# Correr o jogo
make game && ./build/game.exe

# Só compilar (sem limpar)
make tests -j8

# Compilar shaders GLSL → SPIR-V
make shaders
# Se 'make shaders' nao apanhar text.vert/text.frag automaticamente
# (dependendo do padrao glob do Makefile), compilar manualmente:
glslc Game/Assets/Shaders/text.vert -o Game/Assets/Shaders/text.vert.spv
glslc Game/Assets/Shaders/text.frag -o Game/Assets/Shaders/text.frag.spv
```

### 8.4 Ciclo TDD

```bash
# 1. Escrever o teste primeiro
# 2. Implementar até passar:
make tests -j8
# 3. Commit (bloqueado pelo hook se falhar):
git add . && git commit -m "feat: descrição"
# 4. Push (valida campanha antes de enviar):
git push
```

### 8.5 Ferramentas de Nível

```bash
# Validar um nível isolado
python3 Development/AI_Validation/ai_validator.py Game/Assets/Levels/meu_nivel.lvl

# Validar campanha completa
python3 Development/AI_Validation/ai_validator.py --campaign

# Gerar novos níveis por simulação (produz .lvl na raiz → reorganize.py encaminha)
python3 Development/AI_Validation/sim/levelgen.py
python reorganize.py
```

---

## 9. Esquema de Versão

```
vX.Y[Z]  →  X = fase principal  |  .Y = sub-passo  |  Z = fix incremental
```

**Versão actual: 9.2** (ficheiros individuais podem mostrar versões anteriores — o seu próprio incremento local; ver histórico de cada um)

| Ficheiro | Versão | Notas |
|---|---|---|
| VulkanContext.h/.cpp | v2.3/v2.4 | |
| Window.h/.cpp | v2.3 | |
| Swapchain.h/.cpp | v2.40 | |
| RenderPass.h/.cpp | v2.5 | |
| Renderer.h/.cpp | v8.2 | attachSprite(), fix FLAG/timer/creditos |
| TextPipeline.h/.cpp | v7.6 | pipeline dedicada p/ texto, descriptor set |
| FontRenderer.h/.cpp | v7.6 | baking stb_truetype + atlas GPU R8_UNORM |
| SpritePipeline.h/.cpp | v8.2 | NOVO — pipeline dedicada p/ sprites, sampler NEAREST |
| SpriteRenderer.h/.cpp | v8.2 | NOVO — carga PNG via stb_image, upload GPU |
| BitmapFont.h | v7.5 | fallback (usado se TTF nao disponivel) |
| CampaignID.h | v8.1 | FNV-1a determinístico p/ ID de campanha |
| RunHistory.h | v8.1 | regista runs em Development/Runs/runs.csv |
| GameAction.h/.cpp | v9.1 | NOVO — 9 acções lógicas, nome PT + serializado |
| KeyBindings.h/.cpp | v9.1a | GameAction→tecla(s), persistência em controls.cfg |
| Viewport.h/.cpp | v9.2 | NOVO — letterbox + hit-test de menu (espelha Renderer.cpp) |
| Camera.h/.cpp | v6.4 | follow() tracking vertical Lerp |
| Pipeline.h/.cpp | v5.1 | shaders SPIR-V + viewport dinâmico |
| Config.h | v9.1 | + EDITOR_GRID_SNAP (Fase 9.4, actualmente 4px) |
| InputManager.h/.cpp | v9.2 | + Key::E/Q (9.1); + MouseButton/cursor (9.2). isLeft/isRight/isJump inalterados |
| Physics.h/.cpp | v7.1/v7.2 | config-driven, collides() static |
| Player.h/.cpp | v7.1/v7.2 | config-driven, inércia natural |
| Level.h/.cpp | v7.2 | appendFromFile, chunk padronizado a LOGICAL_HEIGHT |
| ReplayManager.h/.cpp | v3.3 | save states + replay |
| base.vert/base.frag | v5.1 | shaders GLSL (retângulos sólidos) |
| text.vert/text.frag | v7.6 | shaders GLSL dedicados a texto (UV+sampler) |
| sprite.vert/sprite.frag | v8.2 | NOVO — shaders GLSL p/ sprites (UV+flipX+tint) |
| main.cpp | v9.2 | + clique em PAUSED/MENU (clickedMenuBox); Pause/UIConfirm/UILeft/UIRight/Quit via KeyBindings |
| ai_validator.py | v7.4 | ASCII-safe (fix Windows cp1252) |
| levelgen.py | v7.4 | ascending_x_at(), clearance 40px, 100% robustez |
| test_campaign.cpp | v7.4 | doctest → system(ai_validator --campaign) |
| test_text_pipeline.cpp | v7.6 | 2 testes (TextPipeline + FontRenderer) |
| test_sprite_pipeline.cpp | v8.2 | NOVO — 2 testes (SpritePipeline + SpriteRenderer) |
| test_campaign_id.cpp | v8.2 | fix: remove_all throwing → nao-throwing |
| test_run_history.cpp | v8.2 | fix: idem; pastas de teste em build/test_tmp/ |
| test_game_action.cpp | v9.1 | NOVO — 8 testes (tabela de acções, round-trip) |
| test_keybindings.cpp | v9.1a | NOVO — 21 testes; v9.1a fix: ifstream nao fechado antes de remove() (so' falhava no Windows) |
| test_viewport.cpp | v9.2 | NOVO — 15 testes (letterbox, coordenadas, hit-test) |
| test_input_mouse.cpp | v9.2 | NOVO — 7 testes (rato no InputManager); ficheiro separado de test_input.cpp de propósito |
| reorganize.py | v9.2 | + rotas Viewport.h/.cpp, test_viewport.cpp, test_input_mouse.cpp |

---

## 10. Optimização de Espaço (requisito permanente)

**Princípio orientador do projecto** (Rafael, v8.2): o motivo de existir uma engine própria em vez de usar Unity/Godot/etc., e de o jogo usar pixel-art simples, é precisamente **otimizar espaço** — tanto em tempo de desenvolvimento como, principalmente, em footprint em disco/memória do jogo final. Este é um requisito **permanente e cumulativo**: cada nova funcionalidade deve ser avaliada também por este critério, não só pela funcionalidade em si. Pode ser feito incrementalmente ("pode se ir fazendo") — não é bloqueante para outras entregas.

**O que já está em prática:**
- **Sprites**: `.pixil` → PNG, sempre passado por `optipng -o7` automaticamente no `reorganize.py` (lossless — verificado byte-a-byte com stb_image antes/depois). Não se usa formato indexado (PNG8) manualmente: testado empiricamente que um bom optimizador PNG (`optipng`) já encontra a codificação óptima sozinho, tornando a escolha manual de formato desnecessária.
- **Fontes**: um único atlas TTF (stb_truetype) em vez de múltiplas imagens de texto pré-renderizadas.
- **Níveis**: ficheiros `.lvl` de texto simples (não binários), comprimem excelentemente em git.

**Por fazer / considerar (incremental, sem prioridade fixa):**
- Ver também **Fase 10 (Distribuição / Release Build)** na secção 6 — o pacote portable final é, em última análise, a expressão mais visível deste princípio: tudo contido, nada desperdiçado, nada instalado.
- Verificar se `optipng` está disponível no ambiente de build de Rafael (Windows) — se não, documentar instalação (`choco install optipng` ou binário directo).
- À medida que houver mais sprites (chão, plataformas, decoração), considerar um **atlas único** (spritesheet) em vez de uma textura por sprite — reduz overhead de binds/descriptor sets e pode comprimir melhor em conjunto do que sprites individuais.
- Avaliar se os `.spv` (shaders compilados) valem a pena ser versionados no git ou gerados sempre no build (actualmente versionados; ficheiros pequenos, benefício de espaço improvável de compensar a perda de reprodutibilidade imediata).
- Quando o editor de níveis (Fase 9) existir, garantir que a interface de escolha de sprites não introduz duplicação de assets (reutilizar o mesmo PNG para múltiplas instâncias no nível, nunca copiar).

---

