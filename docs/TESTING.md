# Matriz de Testes, Validação e Garantia de Qualidade

## Objetivo

O ASCENDENDO adota uma abordagem de engenharia de software com verificação contínua e cobertura multidimensional. Sendo um platformer masoquista de precisão matemática e determinismo bit-a-bit, os testes garantem que cada salto, colisão, renderização e cofre de dados opera com fiabilidade absoluta sob qualquer condição de hardware ou sistema operativo.

---

## 1. Taxonomia Completa de Testes no ASCENDENDO

A suíte de testes do projeto divide-se em categorias formais com objetivos bem definidos:

### 1.1 Testes Unitários e de Componente (`Tests/Unit/`)
Executados em CPU sem dependência de hardware gráfico (GPU):
- **Cinemática e Física**: `test_physics.cpp`, `test_physics_properties.cpp`, `test_level_collision.cpp`, `test_jump_trajectory.cpp`.
- **Lógica de Gameplay**: `test_player.cpp`, `test_simulation_orchestrator.cpp`, `test_game_state_machine.cpp`.
- **Estrutura de Níveis e I/O**: `test_level.cpp`, `test_level_data.cpp`, `test_level_data_io.cpp`, `test_level_file_loading.cpp`.
- **Editor de Níveis e Interação**: `test_editor_session.cpp`, `test_editor_session_keyboard.cpp`, `test_editor_undo_gestures.cpp`, `test_editor_validation.cpp`, `test_editor_trajectory.cpp`.
- **Câmara e Enquadramento**: `test_camera.cpp` (verificação de Lerp e safety viewport clamping).
- **Entrada e Mapeamento**: `test_input.cpp`, `test_input_mouse.cpp`, `test_keybindings_core.cpp`, `test_keybindings_input.cpp`, `test_keybindings_persistence.cpp`.
- **Segurança e Criptografia**: `test_security.cpp`, `test_profile_storage.cpp`, `test_account_manager.cpp` (NIST SHA-256, HMAC-SHA256, PPID e contenção de caminhos).

### 1.2 Testes de Integração (`Tests/Integration/`)
Validam a comunicação entre subsistemas e o driver Vulkan / GLFW:
- **Pipeline e Shaders**: `test_pipeline.cpp`, `test_sprite_pipeline.cpp`, `test_render_pass.cpp`.
- **Ciclo de Apresentação e Swapchain**: `test_swapchain.cpp`, `test_window.cpp`, `test_presentation_runtime.cpp`, `test_renderer.cpp`.
- **Streaming Multi-Ecrã**: `test_multi_screen_streaming_run.cpp` (streaming contínuo de chunks verticais sem travamentos).

### 1.3 Testes de Sistema e Resiliência (`Tests/System/`)
Validam o sistema como um todo integrado sob estresse operacional:
- `test_system_resilience.cpp`: Ciclo completo de inicialização a frio, carregamento de cofre, execução de subida contínua de 10.000 ticks, invariância de enquadramento da câmara em quedas livres, e encerramento limpo sem fugas de memória (*zero leak*).

### 1.4 Testes de Aceitação e Evidência de Replay (`Tests/Acceptance/`)
Validam requisitos funcionais canónicos do produto:
- `test_replay_run_evidence.cpp`: Validação de execuções completas de speedrun através de sequências reais de ticks de input gravadas.
- `test_stable_gameplay_loop.cpp`: Estabilidade a longo prazo do loop de jogo (100.000+ ticks).

### 1.5 Testes de Ponta a Ponta / E2E (`Tests/E2E/`)
Simulam o percurso completo do utilizador:
- `test_e2e_gameplay_lifecycle.cpp`: Menu Inicial $\to$ Seleção de Campanha $\to$ Início de Run $\to$ Conquista da FLAG $\to$ Registo de Replay $\to$ Verificação no Modo Espectador.

### 1.6 Testes Não-Funcionais: Desempenho, Carga, Estresse, Volume e Escalabilidade (`Tests/Performance/`)
Ficheiro central: `test_physics_benchmarks.cpp`:
- **Performance Benchmark**: Simulação física a alta frequência (> 30.000 Hz, mais de 500x tempo real).
- **Carga (Load Testing)**: 50.000 ticks contínuos de simulação sem desvio de ponto flutuante nem overflow.
- **Estresse (Stress Testing)**: Resolução de colisões contra 5.000 plataformas simultâneas em torre de 20.000 metros (com `bodyAABB` e `probe` hoisted).
- **Volume**: Serialização e compressão de mais de 25.000 ticks de replay binário.
- **Escalabilidade**: Verificação de complexidade linear $O(N)$ e invariância com aumento de chunks de nível.

### 1.7 Testes de Segurança e Testes de Penetração (`Tests/Security/`)
Ficheiro central: `test_security_penetration.cpp`:
- Rejeição de ficheiros corrompidos ou com HMAC adulterado (tamper resistance).
- Proteção contra timing attacks via comparação de hashes em tempo constante.
- Rejeição de directory traversal (`../../windows/system32`).
- Sanitização de caracteres proibidos e injeção de fórmulas CSV (`=cmd`, `@calc`).

### 1.8 Testes de Usabilidade e Acessibilidade (`Tests/Usability/`)
Ficheiro central: `test_usability_and_accessibility.cpp`:
- Suporte a remapeamento completo de teclas para canhotos e teclados não-QWERTY (AZERTY, Dvorak).
- Navegação completa no editor exclusivamente por teclado (sem exigir rato).
- Alternância de ecrã inteiro (`F11`) e preservação do aspecto lógico `16:9` com letterboxing automático.
- Contrastes visuais de HUD e texto legíveis segundo as diretrizes WCAG AAA.

### 1.9 Testes Mutacionais, de Especificação e A/B (`Tests/Unit/`)
Ficheiro central: `test_mutation_and_specifications.cpp`:
- **Especificações Físicas**: Garantia do ângulo fixo a 60° ($\tan(60^\circ) \approx 1.732$) e invariante de controlo aéreo zero (tentar mover no ar não altera $v_x$).
- **Testes Mutacionais**: Verificação de que pequenas alterações artificiais em constantes de colisão provocam falhas imediatas de teste (eliminação de mutantes).
- **Testes A/B**: Telemetria e variantes visuais sem quebra de determinismo.

### 1.10 Capturas Determinísticas PPM (Golden Master Testing)
Integrado no pipeline de CI em Linux:
- O motor headless renderiza frames em resoluções e aspect ratios canónicos (16:9, 21:9 e 4:3).
- O ficheiro `.ppm` gerado é comparado bit-a-bit com o hash SHA-256 do golden master para detetar qualquer desvio visual involuntário.

---

## 2. Matriz de CI (13 Workflows Automatizados no GitHub Actions)

| Job no CI | Plataforma / Ambiente | Ferramentas | O que valida |
|---|---|---|---|
| **Campaign Validation** | Ubuntu 24.04 LTS | Python 3.12 / PES | Validação física de alcançabilidade de todos os níveis da campanha. |
| **Linux C++20 Headless** | Ubuntu 24.04 LTS | Clang 18 + Lavapipe | Compilação C++20 rigorosa e execução da suíte completa de testes. |
| **Linux ASan + UBSan** | Ubuntu 24.04 LTS | Clang Address & UB Sanitizer | Fugas de memória, buffer overflows, use-after-free e comportamento indefinido. |
| **PPM Capture (16:9 Nível 0, 1, 2)** | Ubuntu 24.04 LTS | Vulkan Software Driver | Determinismo de renderização na proporção 16:9. |
| **PPM Capture (21:9 Nível 0, 1, 2)** | Ubuntu 24.04 LTS | Vulkan Software Driver | Determinismo de renderização em Ultrawide 21:9 (pillarboxing). |
| **PPM Capture (4:3 Nível 0, 1, 2)** | Ubuntu 24.04 LTS | Vulkan Software Driver | Determinismo de renderização em Retro 4:3 (letterboxing). |
| **Windows Clang C++20** | Windows Server 2025 | Clang-cl + MSVC CRT + Vulkan SDK | Build nativo x64 de lançamento, gates de warnings e testes executados nativamente. |

---

## 3. Comandos Úteis de Execução de Testes

### Execução Completa (Todos os Testes C++)
```powershell
make tests
```

### Execução Verbosa com Detalhe de Cada Asserção
```powershell
make tests-verbose
```

### Execução Rápida (Apenas Lógica, Sem Shaders/GPU)
```powershell
make tests-fast
```

### Validação Offline de Níveis e Campanhas (Python)
```powershell
py -m pytest Tests/
py Development/AI_Validation/validate_campaign.py --campaign Game/Assets/Levels/campaign.txt
```
