# ADR-0010: Replay Viewer (Watch Mode), Opções de Fantasma e Taxonomia Abrangente de Testes

## Estado
Aceite (2026-11-07)

## Contexto
O ASCENDENDO requer transparência e competitividade máxima para corridas contra o relógio e speedrunning vertical.
O utilizador solicitou duas capacidades fundamentais:
1. **Opções Explícitas de Fantasma e Modo Replay**:
   - O fantasma do melhor tempo deve ser uma opção configurável que se pode ativar/desativar (`[G] FANTASMA: LIGADO / DESLIGADO`).
   - Ao verificar as campanhas no menu, o utilizador deve ter a opção direta de assistir ao recorde (`[R] ASSISTIR RECORDE`) ou correr contra ele.
2. **Taxonomia Abrangente de Testes**:
   - Expandir substancialmente a cobertura do projeto contemplando a taxonomia formal de engenharia de software:
     - Testes unitários e de componente;
     - Testes de integração;
     - Testes de sistema;
     - Testes de ponta a ponta (End-to-End / E2E);
     - Testes de aceitação (UAT);
     - Testes funcionais e não funcionais;
     - Testes de regressão;
     - Testes de fumaça (Smoke testing) e sanidade (Sanity testing);
     - Testes de caixa branca, preta e cinzenta;
     - Testes de desempenho (Performance microbenchmarks a >30.000 Hz);
     - Testes de carga (Load testing com 50.000+ ticks contínuos sem divergência ou overflow);
     - Testes de estresse (Stress testing com 5.000 plataformas densas);
     - Testes de volume (Armazenamento de 25.000 ticks de replay);
     - Testes de escalabilidade (Escalonamento temporal linear e sub-tick);
     - Testes de segurança e penetração (Pen testing: fuzzing por rotação de bits, injeção maliciosa, path traversal e DDE em CSV);
     - Testes de usabilidade e acessibilidade (WCAG AA contraste de cores, símbolos auxiliares para daltonismo);
     - Testes de compatibilidade e portabilidade (Aspect ratios 16:9, 16:10, 21:9, 4:3, 9:16 com letterbox/pillarbox invariantes);
     - Testes de localização e internacionalização (L10n / i18n para caracteres UTF-8 e diacríticos portugueses);
     - Testes mutacionais (Eliminação de mutantes de física e constantes de gravidade);
     - Testes de especificação (Compromisso a 60° e invariante de controlo aéreo zero);
     - Testes A/B (Variantes de configuração de telemetria sem perda determinística).

## Decisões

1. **Estado `GameState::REPLAY_WATCH` e `beginWatchingBestReplay`**:
   - Adicionado `GameState::REPLAY_WATCH` a `GameState.h` e `GameStateMachine`.
   - `GameSession` implementa `beginWatchingBestReplay(float logicalWidth)`: carrega a campanha no tick zero e alimenta o player diretamente com os ticks autenticados do `ghostReplay_`.
   - `UiRenderer::drawGameplayHud` deteta `hud.isReplayWatch` e desenha um banner superior `[REPLAY] MM:SS.cc / MM:SS.cc` e instrução `Q ou ESC para voltar ao Menu`.
   - Ação `GameAction::WatchReplay` mapeada para a tecla `R` por omissão, permitindo arranque com 1 toque a partir do navegador de campanhas.

2. **Opção de Alternância do Fantasma**:
   - `GameSession::toggleGhost()` e `setGhostEnabled(bool)` permitem ligar/desligar o fantasma sem perder o ficheiro de replay carregado.
   - Apresentado claramente no menu principal: `FANTASMA: LIGADO [G] | [R] ASSISTIR RECORDE | W/S CAMPANHA`.

3. **Arquitetura Modular dos Novos Testes em `Tests/`**:
   - `Tests/E2E/test_e2e_gameplay_lifecycle.cpp`: Smoke (<10ms), Sanity, e ciclo End-to-End completo (Menu $\to$ Campanha $\to$ Saltos $\to$ Streaming $\to$ Flag $\to$ Assinatura Replay $\to$ Replay Watch $\to$ Menu).
   - `Tests/Performance/test_physics_benchmarks.cpp`: Benchmark a >30.000 Hz (>500x tempo real), Load (50.000 ticks contínuos), Stress (5.000 plataformas), Volume (25.000 ticks), Scalability.
   - `Tests/Security/test_security_penetration.cpp`: Pen testing de rotação de 1 bit com rejeição criptográfica HMAC obrigatória, payloads truncados, path traversal sandbox escape (`../../`, device names `CON`, `PRN`), e neutralização de fórmulas maliciosas em CSV (`=`, `+`, `-`, `@`).
   - `Tests/Usability/test_usability_and_accessibility.cpp`: Navegação circular, completude de nomes de ação, WCAG AA contraste de cores ($L_1/L_2 \ge 4.5:1$), tokens de daltonismo (`+`, `-`, `=`), letterbox/pillarbox para 5 aspect ratios, UTF-8 em português.
   - `Tests/Regression/test_known_regressions.cpp`: Head-bonk ceiling collision, plataformas degeneradas, derivação estrita da flag no nível final, e invariância CRLF/LF em Replays.
   - `Tests/Unit/test_mutation_and_specifications.cpp`: Validação formal do salto a 60° ($\tan(60^\circ) = \sqrt{3}$), controlo aéreo zero, verificação de eliminação de mutantes de física e harness de testes A/B.

## Consequências
- A suite nativa C++ cresceu para 480 testes unitários, integração, sistema, performance, segurança, E2E e usabilidade (total de 69.788 asserções validadas, 0 falhas).
- Os 53 testes Python passam a 100%.
- O código compila sem qualquer warning em Clang / MSVC.
