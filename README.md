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
│   │   └── Config.h                   ← constantes globais (LOGICAL_WIDTH/HEIGHT, física)
│   ├── Graphics/                      ← motor gráfico Vulkan
│   │   ├── VulkanContext.h/.cpp
│   │   ├── Window.h/.cpp              ← janela GLFW
│   │   ├── Swapchain.h/.cpp
│   │   ├── RenderPass.h/.cpp
│   │   ├── Renderer.h/.cpp            ← drawFrame(Player, Camera, Level*)
│   │   ├── Camera.h/.cpp              ← projeção world→NDC, tracking vertical
│   │   └── Pipeline.h/.cpp            ← shaders SPIR-V + viewport dinâmico
│   ├── Logic/                         ← motor de jogo
│   │   ├── InputManager.h/.cpp
│   │   ├── Physics.h/.cpp             ← Vec2, AABB, Fixed Timestep 60Hz
│   │   ├── Player.h/.cpp              ← Commitment Jump (60°), bloqueio aéreo
│   │   ├── Level.h/.cpp               ← plataformas + resolveCollision + streaming
│   │   └── ReplayManager.h/.cpp       ← save states + replay determinístico
│   └── Assets/
│       ├── Shaders/
│       │   ├── base.vert / base.frag  ← GLSL (push constants)
│       │   └── *.spv                  ← compilados por glslc [build artifact]
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
│   │   └── test_campaign.cpp          ← chama ai_validator --campaign via system()
│   └── Integration/
│       ├── test_vulkan_init.cpp
│       ├── test_window.cpp
│       ├── test_swapchain.cpp
│       ├── test_render_pass.cpp
│       ├── test_renderer.cpp
│       └── test_pipeline.cpp
├── external/
│   ├── doctest/doctest.h              ← framework de testes header-only v2.5.0
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
| [GLFW](https://www.glfw.org/) | 3.4 | `external/glfw/` | Windowing Vulkan cross-platform, zlib |
| Vulkan SDK | 1.4.x | sistema (`VULKAN_SDK`) | API gráfica principal + `glslc` para shaders |

---

## 4. Regras de Teste e Versionamento

- **Pre-commit**: bloqueia commits com qualquer teste a falhar (`make tests`).
- **Pre-push**: valida a campanha completa com `ai_validator.py --campaign`.
- **TDD**: testes escritos na fase "Ideia", antes da implementação.
- **Imutabilidade**: testes antigos nunca são alterados para acomodar código novo.
- **Testes confirmados**: **68/68** (v7.4) — 67 testes de motor + `test_campaign`.

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
- 7.4 ✅ **Níveis gerados por simulação** — `levelgen.py` com `ascending_x_at()` (guard contra colisão lateral na subida, 100% robustez ±8%/±10px); `test_campaign.cpp` integra `ai_validator` no ciclo doctest; fixes Windows (Unicode, duplicate symbol)

**Fase 8: UI e Polishing** *(planeada)*

**Fase 9: Editor de Níveis Visual** *(planeada)*
Editor com rato, validação IA automática em background, dificuldade por nível/campanha.

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

**Versão actual: 7.4**

| Ficheiro | Versão | Notas |
|---|---|---|
| VulkanContext.h/.cpp | v2.3/v2.4 | |
| Window.h/.cpp | v2.3 | |
| Swapchain.h/.cpp | v2.40 | |
| RenderPass.h/.cpp | v2.5 | |
| Renderer.h/.cpp | v6.2b | drawFrame(Player, Camera, Level*) |
| Camera.h/.cpp | v6.4 | follow() tracking vertical Lerp |
| Pipeline.h/.cpp | v5.1 | shaders SPIR-V + viewport dinâmico |
| Config.h | v7.2 | PLAYER_WIDTH=16, MIN_JUMP=250, MAX_JUMP=600 |
| InputManager.h/.cpp | v3.3 | injectRawState() para replay |
| Physics.h/.cpp | v7.1/v7.2 | config-driven, collides() static |
| Player.h/.cpp | v7.1/v7.2 | config-driven, inércia natural |
| Level.h/.cpp | v7.2 | appendFromFile, chunk padronizado a LOGICAL_HEIGHT |
| ReplayManager.h/.cpp | v3.3 | save states + replay |
| base.vert/base.frag | v5.1 | shaders GLSL |
| main.cpp | v7.3 | spawn Y=40, campaign streaming |
| ai_validator.py | v7.4 | ASCII-safe (fix Windows cp1252) |
| levelgen.py | v7.4 | ascending_x_at(), clearance 40px, 100% robustez |
| test_campaign.cpp | v7.4 | doctest → system(ai_validator --campaign) |
| reorganize.py | v7.4 | routing por campaign.txt (Unused/NaoValidados) |
