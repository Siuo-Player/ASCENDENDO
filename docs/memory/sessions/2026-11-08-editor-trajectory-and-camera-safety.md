# Sessão de Trabalho — 2026-11-08 (Trajetória no Editor & Clamping de Câmara)

## Objetivo
Implementar a pré-visualização de trajetória balística analítica no editor de níveis e o enquadramento de segurança da câmara visual com base na literatura científica e de game feel (*Aramini et al. 2018*, *Smith et al. 2008*, *Steve Swink 2008*).

## Tarefas Concluídas
1. **Camera Safety Viewport Clamping (`Game/Graphics/Camera.cpp`)**:
   - Clamping dinâmico nas margens verticais: `MIN_BOTTOM_MARGIN = 24.0f` e `MIN_TOP_MARGIN = 32.0f`.
   - Garante que quedas livres em velocidade terminal mantêm o avatar visível e enquadrado.
   - Adicionados 2 novos testes unitários em `Tests/Unit/test_camera.cpp`.
2. **Editor Trajectory Preview (`EditorRenderSnapshot.h`, `EditorSession.cpp`, `EditorRenderer.cpp`)**:
   - Amostragem instantânea de arcos balísticos com `JumpTrajectory::sampleTrajectory` a 60° (mínimo 23.9px em ciano e máximo 137.7px em dourado).
   - Renderização no editor a partir do topo-centro de plataformas selecionadas ou do ponto de spawn.
   - Adicionados testes unitários em `Tests/Unit/test_editor_trajectory.cpp`.
3. **Interação com Rato no Menu (`Game/Logic/GameSession.cpp`)**:
   - Suporte a clique na barra de Fantasma / Replay para alternar estado ou assistir ao recorde.
4. **Decisão de Arquitetura**:
   - Criado `docs/memory/decisions/ADR-0013-editor-jump-trajectory-preview-and-camera-safety-clamping.md`.
