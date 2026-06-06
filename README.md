# ASCENDENDO

**Motor de jogo 2D customizado + Vertical Precision Platformer**
**Autor:** Rafael Gomes Bernardo | **Auxiliado por:** Claude (Anthropic)

---

## 1. Sobre o Projeto

Motor 2D com renderização Vulkan explícita e física determinística em Fixed Timestep. Cada frame de input é gravável em replay e cada estado da simulação é revertível via save states. O jogo foca-se em salto por carga e dificuldade gerada algoritmicamente.

---

## 2. Estrutura do Projeto

```
ASCENDENDO/
├── Game/
│   ├── Graphics/          ← motor gráfico (Vulkan, janela, swapchain)
│   │   ├── VulkanContext.h/.cpp   ← Instance + Device + Queues
│   │   ├── Window.h/.cpp          ← janela GLFW (stub sem GLFW)
│   │   └── (Swapchain, Pipeline — Fase 2.4+)
│   ├── Logic/             ← física, input, Fixed Timestep, save states
│   └── Assets/            ← texturas, mapas, matrizes de colisão
├── Development/
│   ├── dev_log.txt        ← diário append-only (rastreado pelo git)
│   ├── LevelEditor/
│   └── AI_Validation/
├── Tests/
│   ├── test_runner.cpp    ← define main via doctest
│   ├── Unit/
│   │   ├── test_placeholder.cpp
│   │   └── test_vulkan_context.cpp
│   └── Integration/
│       ├── test_vulkan_init.cpp
│       └── test_window.cpp        ← ativo após instalar GLFW
├── external/
│   ├── doctest/           ← framework de testes (header-only)
│   └── glfw/              ← instalar manualmente (ver Secção 8.1)
├── scripts/
│   └── pre-commit.sh      ← bloqueia commits com testes a falhar
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
| [GLFW](https://www.glfw.org/) | 3.4 | `external/glfw/` | Windowing Vulkan cross-platform, zlib license |
| Vulkan SDK | 1.4.341.1 | sistema (`VULKAN_SDK`) | API gráfica principal |

---

## 4. Regras de Teste e Versionamento

- **Pre-commit hook**: nenhum commit passa sem 100% dos testes a verde.
- **TDD**: testes escritos na fase "Ideia", antes da implementação.
- **Imutabilidade**: testes antigos não são alterados para acomodar código novo.

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
- Makefile cross-platform (Windows Git Bash + Linux, Clang++/C++20)
- doctest integrado, hook de pre-commit ativo
- deps.py com permissão explícita antes de qualquer download

**Fase 2: Motor Gráfico Base (Vulkan)**
- 2.1 ✅ Integração do SDK Vulkan (testes headless: instance, versão, GPU)
- 2.2 ✅ `VulkanContext` (Instance + PhysicalDevice + Device + Queues)
- 2.3 ← **AQUI** — `Window` (GLFW) + Surface Vulkan
- 2.4 → Swapchain
- 2.5 → Render pass + framebuffers
- 2.6 → Primeiro ecrã visível (clear to color)

**Fase 3: Física e Input**
- Fixed Timestep, colisões AABB, input WASD/setas

**Fase 4: Level Streaming e Câmera**
- Mapas interligados, câmera ortográfica, object pooling

**Fase 5: Ferramentas (Level Editor)**

**Fase 6: Jogo + IA de Validação**
- Salto por carga (barra UI + força numérica)
- Validador algorítmico de dificuldade de mapas

**Fase 7: V2 / New Game+**
- Física deslizante, novas mecânicas

---

## 7. Stack Técnico

| Componente | Escolha | Notas |
|---|---|---|
| Linguagem | C++20 | `concepts`, `std::span`, controlo total de memória |
| Compilador | Clang++ 22 (LLVM) | `Target: x86_64-pc-windows-msvc` |
| Build | GNU Make (manual) | portável via Git Bash / MSYS2 |
| API Gráfica | Vulkan 1.3 | explícita, determinística, cross-platform |
| Windowing | GLFW 3.4 | propositado para Vulkan, zlib license |
| Testes | doctest 2.5.0 | header-only, zero instalação |
| Audio/Input avançado | SDL2 (Fase 3+) | a decidir |
| Shaders | GLSL → SPIR-V | compilado com `glslc` (Vulkan SDK) |

---

## 8. Setup do Ambiente

### 8.1 Requisitos e Instalação

| Ferramenta | Estado | Instalação |
|---|---|---|
| Clang++ ≥ 14 | ✅ instalado | `winget install LLVM.LLVM` |
| GNU Make | ✅ instalado | Git Bash ou `choco install make` |
| Git | ✅ instalado | https://git-scm.com/ |
| Python ≥ 3.9 | ✅ instalado | `winget install Python.Python.3` |
| Vulkan SDK | ✅ instalado | https://vulkan.lunarg.com/sdk/home |
| **GLFW 3.4** | ⬅ **instalar agora** | ver abaixo |

**Instalar GLFW (Fase 2.3):**
1. Descarregar `glfw-3.4.bin.WIN64.zip` de https://www.glfw.org/download.html
2. Extrair e copiar para `external/glfw/`:
   ```
   external/glfw/
   ├── include/GLFW/glfw3.h
   ├── include/GLFW/glfw3native.h
   └── lib-vc2022/glfw3.lib  (+ glfw3dll.lib, glfw3.dll)
   ```
3. Correr `make clean && make tests` — o Makefile deteta GLFW automaticamente

### 8.2 Primeiros Passos

```bash
git clone https://github.com/Siuo-Player/ASCENDENDO && cd ASCENDENDO
python deps.py                      # verificar dependências
python reorganize.py                # organizar ficheiros (após descarregar pacote)
cp scripts/pre-commit.sh .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
make tests                          # validar setup
```

### 8.3 Ciclo de Desenvolvimento (TDD)

```bash
# 1. Escrever teste (Tests/Unit/ ou Tests/Integration/)
# 2. Iterar até passar:
make tests
# 3. Commit (hook bloqueia se falhar):
git add . && git commit -m "feat: descrição"
```
