# Sessão: 2026-11-07 — Speedrun HUD, Jump Meter e Navegador de Campanhas

## Tarefas Realizadas
1. **Ergonomia do Commitment Jump**:
   - Adicionado `chargeRatio` a `RenderPlayerSnapshot` em `RenderSnapshot.h`.
   - Adicionada cópia de `player.chargeRatio()` em `RenderSnapshotBuilder.cpp`.
   - Implementada barra visual de força acima da cabeça do avatar em `WorldRenderer.cpp` com gradiente de cor amarela a âmbar.
2. **Speedrun HUD e Telemetria**:
   - Adicionada estrutura `RenderHudSnapshot` em `RenderSnapshot.h`.
   - Implementada formatação com precisão de centésimos de segundo (`MM:SS.cc`) em `UiRenderer.cpp`.
   - Adicionado HUD superior com tempo decorrido, recorde pessoal (`PB`), estado do fantasma (`[G] GHOST`), split delta de altitude (`▲ +X m` / `▼ -X m`), nome da campanha e altitude em metros (`ALTURA: X m`).
3. **Navegador e Seletor de Campanhas Interativo**:
   - `GameSession` atualizado para gerir a lista de campanhas catalogadas (`availableCampaigns_`), seleção ativa (`activeCampaignIndex_`), ciclo circular (`selectNextCampaign`, `selectPreviousCampaign`) e seleção direta (`selectCampaign`).
   - `UiRenderer::drawMenu` enriquecido com painel interativo de campanha exibindo título, índice (`[01/10]`), número de níveis, recorde pessoal e vitórias.
   - Navegação integrada com teclas `W`/`S`, setas `UP`/`DOWN`, `[` / `]`, e cliques de rato nas setas `<` e `>`.
   - Carregamento automático do fantasma de speedrun correspondente à campanha selecionada.
4. **Testes e Validação**:
   - Novos testes unitários em `Tests/Unit/test_render_snapshot.cpp` para `chargeRatio` e `RenderHudSnapshot`.
   - Novos testes unitários em `Tests/Unit/test_game_session.cpp` para navegação circular de campanhas, proteção de limites e snapshots de menu e HUD.
   - Execução de 453 testes C++ com 69,292 asserções (100% sucesso).
   - Execução de 53 testes Python com 100% sucesso.
   - Binário `build/game/game.exe` e pacote zip `dist/Release/ascendendo-v1.0.0-windows-x64.zip` compilados e gerados com sucesso.
5. **Decisões Documentadas**:
   - Registado `docs/memory/decisions/ADR-0009-speedrun-hud-jump-meter-and-campaign-browser.md`.
