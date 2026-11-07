# Sessão: 2026-11-07 — Replay Viewer (Watch Mode), Opções de Fantasma e Taxonomia Completa de Testes

## Objetivo
Atender aos pedidos do utilizador:
1. Tornar o Fantasma do melhor tempo uma opção explícita configurável (ligar/desligar `[G]`) e permitir assistir diretamente ao melhor run gravado (`[R] ASSISTIR RECORDE`) ou correr contra ele no navegador de campanhas.
2. Expandir substancialmente a abrangência e rigor da engenharia de testes do projeto, cobrindo a taxonomia formal: Unit, Integration, System, End-to-End (E2E), Acceptance (UAT), Functional, Non-functional, Regression, Smoke, Sanity, White-box, Black-box, Gray-box, Performance, Load, Stress, Volume, Scalability, Security/Penetration, Usability, Accessibility, Compatibility, Portability, Localization (i18n), Mutation, Specification-based e A/B testing.

## Tarefas Concluídas

1. **Replay Viewer & Opções de Fantasma**:
   - `Game/Core/GameState.h`: Adicionado estado `GameState::REPLAY_WATCH`.
   - `Game/Core/GameStateMachine.h` / `.cpp`: Adicionado `enterReplayWatch()`.
   - `Game/Core/GameAction.h` / `.cpp`: Adicionado `GameAction::WatchReplay` ("Assistir Recorde / Replay") com mapeamento serializado.
   - `Game/Core/KeyBindings.h` / `.cpp`: Mapeada a tecla `Key::R` por omissão para `WatchReplay`.
   - `Game/Logic/InputManager.h`: Adicionada constante `Key::R = 82`.
   - `Game/Logic/GameSession.h` / `.cpp`:
     - Implementado `beginWatchingBestReplay(float logicalWidth)`: carrega a campanha e executa os ticks do `ghostReplay_`.
     - Tratamento do loop `REPLAY_WATCH` com avanço determinístico de ticks, seguimento de câmara e retorno automático ao menu no fim do replay ou tecla ESC/Q.
     - Suporte a `toggleGhost()` no Menu e durante gameplay, refletido em `menuRenderSnapshot()` e `hudSnapshot()`.
   - `Game/Graphics/UiRenderer.cpp`:
     - Menu principal exibe: `FANTASMA: LIGADO [G] | [R] ASSISTIR RECORDE | W/S CAMPANHA`.
     - HUD em modo Replay Watch exibe banner superior `[REPLAY] MM:SS.cc / MM:SS.cc` e atalho de saída.
   - `main.cpp`: Atualizado para tratar `GameState::REPLAY_WATCH` na câmara, na apresentação e no ciclo de renderização.

2. **Novas Suites de Teste por Categoria**:
   - `Tests/E2E/test_e2e_gameplay_lifecycle.cpp`: Smoke (<10ms), Sanity e ciclo E2E completo (Menu $\to$ Campanha $\to$ Salto $\to$ Streaming $\to$ Flag $\to$ Assinatura Replay $\to$ Replay Watch $\to$ Menu).
   - `Tests/Performance/test_physics_benchmarks.cpp`: Benchmark de simulação física (>30.000 Hz), Load testing (50.000 ticks contínuos), Stress testing (5.000 plataformas densas), Volume testing (25.000 ticks), Scalability (escalonamento temporal e sub-tick).
   - `Tests/Security/test_security_penetration.cpp`: Pen testing de rotação de 1 bit com rejeição criptográfica HMAC obrigatória, payloads truncados, path traversal sandbox escape (`../../`, device names `CON`, `PRN`), e neutralização de fórmulas maliciosas em CSV (`=`, `+`, `-`, `@`).
   - `Tests/Usability/test_usability_and_accessibility.cpp`: Navegação circular, completude de nomes de ação, WCAG AA contraste de cores ($L_1/L_2 \ge 4.5:1$), tokens de daltonismo (`+`, `-`, `=`), letterbox/pillarbox para 5 aspect ratios, UTF-8 em português.
   - `Tests/Regression/test_known_regressions.cpp`: Head-bonk ceiling collision, plataformas degeneradas, derivação estrita da flag no nível final, e invariância CRLF/LF em Replays.
   - `Tests/Unit/test_mutation_and_specifications.cpp`: Validação formal do salto a 60° ($\tan(60^\circ) = \sqrt{3}$), controlo aéreo zero, verificação de eliminação de mutantes de física e harness de testes A/B.

3. **Correções de Robustez Descobertas pelos Novos Testes**:
   - `Game/Logic/RunReplay.cpp`: Corrigido bug onde a primeira linha com quebra de linha Windows CRLF (`\r\n`) impedia o reconhecimento de `ASCENDENDO_REPLAY_V1` no `deserialize()`.
   - Adicionada validação de sanidade em `RunReplay::deserialize` (rejeição de tempos negativos ou NaN e verificação da diretiva `INPUTS:`).
   - `Makefile`: Integradas as novas pastas de teste (`Performance`, `Security`, `E2E`, `Usability`) no `TEST_SRCS`.

## Testes e Validação Executados
- `make tests-fast`: 447 testes TDD passaram com 100% de sucesso (69.593 asserções).
- `make tests`: 480 testes totais (incluindo Vulkan e gráficos) passaram com 100% de sucesso (69.788 asserções).
- `py -m pytest Tests/`: 53 testes Python passaram com 100% de sucesso em 4.62s.
- `python Development/Tools/validate_repository_state.py`: Estado das 10 campanhas e 610 níveis 100% consistente.
- `make game`: Binário compilado com zero warnings (`-Wall -Wextra -Werror`).
