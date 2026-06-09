# ASCENDENDO

**Motor de jogo 2D customizado + Vertical Precision Platformer**
**Autor:** Rafael Gomes Bernardo | **Auxiliado por:** Claude (Anthropic)
**Repositório:** https://github.com/Siuo-Player/ASCENDENDO

---

## 1. Sobre o Projeto

Motor 2D com renderização Vulkan explícita e física determinística em Fixed Timestep. Cada frame de input é gravável em replay e cada estado da simulação é revertível via save states. O jogo foca-se em **salto parabólico com inércia (Commitment Jump)** e dificuldade gerada algoritmicamente.

---

## 2. Estrutura do Projeto

```
ASCENDENDO/
├── .vscode/
│   └── c_cpp_properties.json          ← IntelliSense (Vulkan, GLFW, doctest)
├── Game/
│   ├── Core/
│   │   └── Config.h                   ← constantes globais (LOGICAL_WIDTH/HEIGHT, FIXED_STEP)
│   ├── Graphics/                      ← motor gráfico Vulkan
│   │   ├── VulkanContext.h/.cpp       ← Instance + Device + Queues + Surface
│   │   ├── Window.h/.cpp              ← janela GLFW (stub sem GLFW)
│   │   ├── Swapchain.h/.cpp           ← double buffering + V-Sync
│   │   ├── RenderPass.h/.cpp          ← clear + store por frame
│   │   ├── Renderer.h/.cpp            ← Framebuffers + drawFrame(Player, Camera)
│   │   ├── Camera.h/.cpp              ← projeção world→NDC, viewport lógico
│   │   └── Pipeline.h/.cpp            ← shaders SPIR-V + viewport/scissor dinâmicos
│   ├── Logic/                         ← motor de jogo
│   │   ├── InputManager.h/.cpp        ← estados de teclas + callbacks GLFW
│   │   ├── Physics.h/.cpp             ← Vec2, AABB, Fixed Timestep, gravidade
│   │   ├── Player.h/.cpp              ← Commitment Jump, movimento horizontal
│   │   └── ReplayManager.h/.cpp       ← save states + gravação/replay de inputs
│   └── Assets/
│       └── Shaders/
│           ├── base.vert              ← vertex shader (push constants)
│           ├── base.frag              ← fragment shader
│           ├── base.vert.spv          ← compilado por glslc [build artifact]
│           └── base.frag.spv          ← compilado por glslc [build artifact]
├── Development/
│   ├── dev_log.txt                    ← diário append-only (rastreado pelo git)
│   └── project_structure.txt         ← mapa de arquitectura
├── Tests/
│   ├── test_runner.cpp                ← define main via doctest
│   ├── Unit/
│   │   ├── test_placeholder.cpp       ← infra + sizeof de tipos
│   │   ├── test_vulkan_context.cpp    ← 5 test cases
│   │   ├── test_input.cpp             ← 7 test cases
│   │   ├── test_physics.cpp           ← 15 test cases
│   │   ├── test_player.cpp            ← Commitment Jump + bloqueio aéreo
│   │   ├── test_replay.cpp            ← save states + replay determinístico
│   │   └── test_camera.cpp            ← projeção world→NDC
│   └── Integration/
│       ├── test_vulkan_init.cpp       ← instance, versão driver, GPUs
│       ├── test_window.cpp            ← janela GLFW + surface
│       ├── test_swapchain.cpp         ← swapchain 800×600
│       ├── test_render_pass.cpp       ← pipeline até RenderPass
│       ├── test_renderer.cpp          ← 500 frames coloridos (visual ~8s)
│       └── test_pipeline.cpp          ← pipeline gráfica + shaders
├── external/
│   ├── doctest/doctest.h              ← framework de testes (header-only)
│   └── glfw/
│       ├── include/GLFW/
│       └── lib-vc2022/
├── scripts/
│   └── pre-commit.sh                 ← bloqueia commits com testes a falhar
├── build/                             ← artefactos de compilação [IGNORADO NO GIT]
├── main.cpp                           ← entry point do jogo (loop RAII)
├── Makefile
├── deps.py
└── reorganize.py
```

---

## 3. Gestão de Dependências

Nenhuma dependência é adicionada sem justificação no `dev_log.txt` e sem pedido explícito de permissão. O `deps.py` implementa este contrato.

| Biblioteca | Versão | Localização | Justificação |
|---|---|---|---|
| [doctest](https://github.com/doctest/doctest) | 2.5.0 | `external/doctest/` | Header-only, zero overhead, MIT |
| [GLFW](https://www.glfw.org/) | 3.4 | `external/glfw/` | Windowing Vulkan cross-platform, zlib |
| Vulkan SDK | 1.4.341.1 | sistema (`VULKAN_SDK`) | API gráfica principal + `glslc` para shaders |

---

## 4. Regras de Teste e Versionamento

- **Pre-commit hook**: nenhum commit passa sem 100% dos testes a verde.
- **TDD**: testes escritos na fase "Ideia", antes da implementação.
- **Imutabilidade**: testes antigos não são alterados para acomodar código novo.
- **Testes confirmados**: **54/54** a passar (v5.3) — `test_player.cpp` refatorizado em v6.1.

---

## 5. Diário de Desenvolvimento

Ficheiro `Development/dev_log.txt` — append-only. Formato obrigatório:

```
[YYYY-MM-DD HH:MM:SS]
Ideia: [o que vamos implementar]
Implementado: [como correu, problemas e soluções]
```

---

## 6. Planeamento de Desenvolvimento

**Fase 1: Ambiente e Infraestrutura** ✅ CONCLUÍDA
- Makefile cross-platform, doctest, hook de pre-commit, deps.py

**Fase 2: Motor Gráfico Base (Vulkan)** ✅ CONCLUÍDA
- 2.1 ✅ SDK Vulkan integrado (instance, driver, GPUs — headless)
- 2.2 ✅ `VulkanContext` (Instance + PhysicalDevice + Device + Queues)
- 2.3 ✅ `Window` (GLFW) + Surface Vulkan
- 2.4 ✅ `Swapchain` (double buffering, V-Sync)
- 2.5 ✅ `RenderPass` (clear + store, layout para apresentação)
- 2.6 ✅ `Renderer` (Framebuffers + Command Buffers + Sync → **primeira janela colorida**)

**Fase 3: Física, Input e Replay** ✅ CONCLUÍDA
- 3.1 ✅ `InputManager` + `Physics` (Vec2, AABB, Fixed Timestep 60Hz, gravidade)
- 3.2 ✅ `Player` (movimento horizontal 200 px/s + salto por carga MIN=300/MAX=750 px/s)
- 3.3 ✅ `ReplayManager` (save states + gravação/replay determinístico de inputs)

**Fase 4: Câmera e Viewport** ✅ CONCLUÍDA
- 4.1 ✅ `Camera` (projeção world→NDC, viewport lógico com letterboxing)

**Fase 5: Pipeline Gráfica** ✅ CONCLUÍDA
- 5.1 ✅ Shaders GLSL → SPIR-V (`base.vert`/`base.frag`) + `Pipeline` Vulkan (viewport/scissor dinâmicos)
- 5.2 ✅ `Config.h` (constantes globais: LOGICAL_WIDTH=640, LOGICAL_HEIGHT=360, FIXED_STEP)
- 5.3 ✅ `Renderer` integra `Player` + `Camera` → **primeiro quadrado a cair em tempo real** (54/54 testes ✅)

**Fase 6: Mecânicas de Jogo** ← AQUI
- 6.1 ✅ **Commitment Jump** — salto parabólico com inércia (bloqueio aéreo de inputs, ângulo 60°, `main.cpp` RAII)
- 6.2 → *(a definir)*

**Fase 7: Level Design e Geração Procedural** *(planeada)*

**Fase 8: UI e Polishing** *(planeada)*
- Barra de carga do salto + valor numérico de força

**Fase 9: IA de Validação** *(planeada)*
- Validador algorítmico de dificuldade (grafos + parábolas)

---

## 7. Stack Técnico

| Componente | Escolha | Notas |
|---|---|---|
| Linguagem | C++20 | controlo de memória, `concepts`, compatibilidade Vulkan |
| Compilador | Clang++ 22 (LLVM) | `Target: x86_64-pc-windows-msvc` |
| Build | GNU Make (manual) | portável via Git Bash / MSYS2 |
| API Gráfica | Vulkan 1.3 | explícita, determinística, cross-platform |
| Windowing | GLFW 3.4 | propositado para Vulkan, zlib license |
| Testes | doctest 2.5.0 | header-only, zero instalação |
| Shaders | GLSL → SPIR-V | compilado com `glslc` (incluído no Vulkan SDK) |

---

## 8. Setup do Ambiente

### 8.1 Requisitos

| Ferramenta | Estado | Instalação |
|---|---|---|
| Clang++ ≥ 14 | ✅ | `winget install LLVM.LLVM` + adicionar ao PATH |
| GNU Make | ✅ | Git Bash (incluído) ou `choco install make` |
| Git | ✅ | https://git-scm.com/ |
| Python ≥ 3.9 | ✅ | `winget install Python.Python.3` |
| Vulkan SDK | ✅ | https://vulkan.lunarg.com/sdk/home (inclui `glslc`) |
| GLFW 3.4 | ✅ | https://www.glfw.org/download.html → `glfw-3.4.bin.WIN64.zip` |

**Instalar GLFW:**
1. Descarregar `glfw-3.4.bin.WIN64.zip`
2. Extrair `include/` e `lib-vc2022/` para `external/glfw/`
3. `make clean && make tests` — o Makefile deteta automaticamente

### 8.2 Primeiros Passos

```bash
git clone https://github.com/Siuo-Player/ASCENDENDO && cd ASCENDENDO
python deps.py                       # verificar dependências
python reorganize.py                 # organizar ficheiros descarregados
cp scripts/pre-commit.sh .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
make tests                           # silencioso — mostra só resumo
make tests-verbose                   # detalhado — mostra cada assertion
make tests-fast                      # rápido — ignora testes Vulkan/GLFW
make shaders                         # compila shaders GLSL → SPIR-V
make game                            # compila o binário do jogo
```

### 8.3 Ciclo de Desenvolvimento (TDD)

```bash
# 1. Escrever o teste primeiro
# 2. Implementar até passar:
make tests-fast          # iteração rápida (sem Vulkan)
make tests               # validação completa antes de commit
# 3. Commit automático protegido pelo hook:
git add . && git commit -m "feat: descrição"
```

---

## 9. Esquema de Versão dos Ficheiros

Cada ficheiro tem `@version` no seu cabeçalho a indicar quando foi modificado pela última vez.

```
vX.Y[Z...]  onde:
  X    = Fase principal (1–9, conforme README)
  .Y   = Sub-passo da fase  (ex: 2.3 = Fase 2, Passo 3)
  .YZ  = Pequena atualização dentro do sub-passo (ex: 2.40 = 1ª fix em 2.4)
  .YZW = Atualização ainda mais granular (ex: 2.401, 2.410...)
```

**Versão atual do projeto: 6.1**

| Ficheiro | Versão | Notas |
|---|---|---|
| VulkanContext.h | v2.3 | |
| VulkanContext.cpp | v2.4 | |
| Window.h/.cpp | v2.3 | |
| Swapchain.h/.cpp | v2.40 | |
| RenderPass.h/.cpp | v2.5 | |
| Renderer.h/.cpp | v5.3 | integra Player + Camera |
| Camera.h/.cpp | v4.1 | projeção world→NDC |
| Pipeline.h/.cpp | v5.1 | shaders SPIR-V + viewport dinâmico |
| Config.h | v5.2 | constantes globais |
| InputManager.h/.cpp | v3.1 | |
| Physics.h/.cpp | v3.3 | m_accumulator exposto para ReplayManager |
| Player.h/.cpp | v6.1 | Commitment Jump, bloqueio aéreo |
| ReplayManager.h/.cpp | v3.3 | save states + replay |
| base.vert / base.frag | v5.1 | shaders GLSL |
| main.cpp | v6.1 | entry point RAII |
