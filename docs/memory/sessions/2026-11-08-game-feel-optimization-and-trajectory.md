# Sessão 2026-11-08: Otimização de Viewport Culling, Game Feel e Trajetória Analítica

## Tarefas Concluídas
1. **Frustum Culling de Viewport**:
   - `ShapeRenderer.cpp`: Descarte imediato de primitivas fora do retângulo visível da câmara (`[camPos, camPos + 640x360]`), poupando binds de pipeline e `vkCmdDraw`.
   - `SpriteRenderer.cpp`: Descarte de sprites fora do viewport.
2. **Game Feel (Anticipation Squash)**:
   - `WorldRenderer.cpp`: Implementada compressão de antecipação visual durante o carregamento de salto (squash vertical até 12%), preservando a base dos pés e a caixa física de colisão intacta.
3. **Módulo Analítico de Trajetória (`JumpTrajectory`)**:
   - `Game/Logic/JumpTrajectory.h` e `.cpp`: Implementado cálculo exato de velocidade inicial, tempo ao ápice, altura do ápice e envelope analítico de alcance a 60°.
   - `Tests/Unit/test_jump_trajectory.cpp`: Criada suite de testes unitários para validar a matemática balística.
4. **Documentação e Literatura**:
   - Incorporadas referências de Steve Swink (*Game Feel*), Maddy Thorson (*Celeste*) e Derek Yu (*Spelunky*) em `docs/SCIENTIFIC_REFERENCES.md`.
   - Registado ADR-0012.

## Testes Executados
- 486 testes unitários e de integração locais executados.
