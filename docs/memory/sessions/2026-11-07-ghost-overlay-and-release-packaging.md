# Sessão: 2026-11-07 — Ghost Replay Overlay em Tempo Real e Empacotamento de Release 1.0

## Tarefas Concluídas
1. **Apresentação de Fantasma / Ghost Overlay em Tempo Real**:
   - `Game/Graphics/RenderSnapshot.h`: Adicionada `RenderGhostSnapshot` ao snapshot de renderização imutável.
   - `Game/Graphics/PresentationConfig.h`: Configurada a cor e transparência do fantasma (`COLOR_GHOST_R=0.0f, G=0.94f, B=1.0f, ALPHA=0.55f`).
   - `Game/Graphics/RenderSnapshotBuilder.h` e `.cpp`: Suporte opcional a `ghostPlayer` sem quebrar chamadas legadas.
   - `Game/Graphics/WorldRenderer.cpp`: Renderização translúcida do fantasma em sprite ou geometria vetorial antes do jogador.

2. **Simulação de Fantasma em `GameSession`**:
   - Deteção e carregamento automático de `Replays/latest_<campaignID>.replay` do utilizador.
   - Simulação física paralela a 60 Hz consumindo a sequência de inputs gravada.
   - Nova ação `GameAction::ToggleGhost` mapeada para a tecla `G` durante o gameplay, permitindo ligar/desligar o fantasma com um clique.

3. **Validação do Empacotamento de Release 1.0 (`package_release.py`)**:
   - `make game` gerou o binário de release otimizado `build/game/game.exe` (0.8 MB).
   - Executado `Development/Tools/package_release.py` com sucesso:
     - Gerado `ascendendo-v1.0.0-windows-x64.zip` (0.56 MB).
     - Hash SHA-256 verificado: `e89e78a9626581dca23c4d089672d4f9f29b9f3a9f32ec820ad5298ba2e47c78`.

4. **Testes Automatizados**:
   - Adicionados testes unitários em `test_render_snapshot.cpp` e `test_game_session.cpp`.
   - `make tests-fast`: 416/416 casos de teste passados, 69.061 asserções.
   - `make tests`: 449/449 casos de teste passados, 69.256 asserções.
   - `py -m pytest Tests/`: 53/53 testes Python passados em 7.48s.

## Decisões Documentadas
- Criado [ADR-0008](file:///c:/Users/ASUS/Documents/Ascendendo/docs/memory/decisions/ADR-0008-in-game-ghost-replay-presentation.md).
