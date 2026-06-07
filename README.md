# ASCENDENDO

**Motor de jogo 2D customizado + Vertical Precision Platformer**
**Autor:** Rafael Gomes Bernardo | **Auxiliado por:** Claude (Anthropic)
**Repositório:** https://github.com/Siuo-Player/ASCENDENDO

---

## 1. Sobre o Projeto

Motor 2D com renderização Vulkan explícita e física determinística em Fixed Timestep. Cada frame de input é gravável em replay e cada estado da simulação é revertível via save states. O jogo foca-se em salto por carga e dificuldade gerada algoritmicamente.

---

## 2. Estrutura do Projeto

```
ASCENDENDO/
├── .vscode/
│   └── c_cpp_properties.json      ← IntelliSense (paths Vulkan, GLFW, doctest)
├── Game/
│   ├── Graphics/                  ← motor gráfico Vulkan
│   │   ├── VulkanContext.h/.cpp   ← Instance + Device + Queues + Surface
│   │   ├── Window.h/.cpp          ← janela GLFW (stub sem GLFW)
│   │   ├── Swapchain.h/.cpp       ← double buffering + V-Sync
│   │   ├── RenderPass.h/.cpp      ← define clear + store por frame
│   │   └── Renderer.h/.cpp         ← Framebuffers + drawFrame(r,g,b)
│   ├── Logic/                     ← física, input, Fixed Timestep (Fase 3+)
│   └── Assets/                    ← texturas, mapas, matrizes de colisão
├── Development/
│   ├── dev_log.txt                ← diário append-only (rastreado pelo git)
│   ├── LevelEditor/
│   └── AI_Validation/
├── Tests/
│   ├── test_runner.cpp            ← define main via doctest
│   ├── Unit/
│   │   ├── test_placeholder.cpp   ← infra + sizeof de tipos
│   │   └── test_vulkan_context.cpp
│   └── Integration/
│       ├── test_vulkan_init.cpp   ← instance, versão driver, GPUs
│       ├── test_window.cpp        ← janela GLFW + surface
│       ├── test_swapchain.cpp     ← swapchain 800×600
│       ├── test_render_pass.cpp   ← pipeline completa até RenderPass
│       └── test_renderer.cpp      ← 500 frames coloridos (visual, ~8s)
├── external/
│   ├── doctest/doctest.h          ← framework de testes (header-only)
│   └── glfw/                      ← instalado manualmente (ver 8.1)
│       ├── include/GLFW/
│       └── lib-vc2022/
├── scripts/
│   └── pre-commit.sh              ← bloqueia commits com testes a falhar
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
| Vulkan SDK | 1.4.341.1 | sistema (`VULKAN_SDK`) | API gráfica principal |

---

## 4. Regras de Teste e Versionamento

- **Pre-commit hook**: nenhum commit passa sem 100% dos testes a verde.
- **TDD**: testes escritos na fase "Ideia", antes da implementação.
- **Imutabilidade**: testes antigos não são alterados para acomodar código novo.
- **Testes atuais**: **36 a passar** (18 Fases 1–2 + 7 InputManager + 11 Physics) (2 Unit + 3 Init + 4 Window + 1 Swapchain + 1 RenderPass + 2 Renderer + 5 VulkanContext).

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

**Fase 2: Motor Gráfico Base (Vulkan)**
- 2.1 ✅ SDK Vulkan integrado (instance, driver, GPUs — headless)
- 2.2 ✅ `VulkanContext` (Instance + PhysicalDevice + Device + Queues)
- 2.3 ✅ `Window` (GLFW) + Surface Vulkan
- 2.4 ✅ `Swapchain` (double buffering, V-Sync, 800×600)
- 2.5 ✅ `RenderPass` (clear + store, layout para apresentação)
- **2.6 ← AQUI** — `Renderer` (Framebuffers + Command Buffers + Sync → **primeira janela colorida**)

**Fase 3: Física e Input**
- **3.1 ← AQUI** — `InputManager` (key states, GLFW callback) + `Physics` (Vec2, AABB, Fixed Timestep, gravidade)
- 3.2 → Movimento horizontal do personagem
- 3.3 → Save States + gravação de replays de inputs

**Fase 4: Level Streaming e Câmera**
- Mapas interligados, câmera ortográfica, object pooling Vulkan

**Fase 5: Ferramentas (Level Editor)**

**Fase 6: Jogo + IA de Validação**
- Salto por carga (barra UI + valor numérico da força)
- Validador algorítmico de dificuldade (grafos + parábolas)

**Fase 7: V2 / New Game+**
- Física deslizante, novas mecânicas

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
| Vulkan SDK | ✅ | https://vulkan.lunarg.com/sdk/home |
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
```

### 8.3 Ciclo de Desenvolvimento (TDD)

```bash
# 1. Escrever o teste primeiro
# 2. Implementar até passar:
make tests
# 3. Commit automático protegido pelo hook:
git add . && git commit -m "feat: descrição"
```

---

## 9. Esquema de Versão dos Ficheiros

Cada ficheiro tem `@version` no seu cabeçalho a indicar quando foi modificado pela última vez.

```
vX.Y[Z...]  onde:
  X    = Fase principal (1–8, conforme README)
  .Y   = Sub-passo da fase  (ex: 2.3 = Fase 2, Passo 3)
  .YZ  = Pequena atualização dentro do sub-passo (ex: 2.40 = 1ª fix em 2.4)
  .YZW = Atualização ainda mais granular (ex: 2.401, 2.410...)
```

**Versão atual do projeto: 3.1**

| Ficheiro | Versão |
|---|---|
| VulkanContext.h | v2.3 |
| VulkanContext.cpp | v2.4 |
| Window.h/.cpp | v2.3 |
| Swapchain.h/.cpp | v2.40 |
| RenderPass.h/.cpp | v2.5 |
| Renderer.h/.cpp | v2.6 |
| InputManager.h/.cpp | v3.1 |
| Physics.h/.cpp | v3.1 |
