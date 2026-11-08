# ADR-0012: Game Feel Anticipation, Frustum Culling & Analytical Jump Trajectory

## Contexto
Com base no estudo de literatura de Game Design e Game Feel (*Steve Swink 2008: Game Feel*, *Maddy Thorson 2018: Celeste*, *Smith et al. 2008* e *Aramini et al. 2018*), procurou-se otimizar a eficiência de renderização em campanhas verticais longas e melhorar a comunicação visual da acumulação de energia sem quebrar o determinismo físico a 60 Hz.

## Decisão
1. **Frustum Culling de Viewport (`ShapeRenderer` & `SpriteRenderer`)**:
   - Primitivas completamente fora do viewport da câmara (`[camPos, camPos + 640x360]`) são descartadas no CPU antes de qualquer bind de pipeline, `vkCmdPushConstants` ou `vkCmdDraw`.
   - Elimina milhares de chamadas redundantes de desenho em campanhas com 50 a 250 ecrãs verticais (onde centenas de plataformas residem abaixo ou acima do ecrã atual).
2. **Game Feel: Anticipation Squash Visual (`WorldRenderer`)**:
   - Durante o carregamento do salto (`player.chargeRatio > 0`), o avatar sofre uma deformação subtil de antecipação (compressão até 12% na altura com ligeira expansão de 10% na largura), mantendo os pés ancorados à superfície.
   - A caixa de colisão canónica do `PhysicsBody` mantém-se estritamente rígida ($16 \times 16$) e determinística.
3. **Módulo de Trajetória Analítica (`JumpTrajectory`)**:
   - Criação de `Game/Logic/JumpTrajectory` fornecendo equações balísticas fechadas para a velocidade inicial, tempo ao ápice, altura máxima e envelope de alcance a 60°.
   - Permite amostragem discreta da parábola para ferramentas de autor e diagnósticos no editor.
4. **Atualização da Literatura de Referência**:
   - Incorporação de Swink (2008), Thorson & Berry (2018) e Yu (2016) em `docs/SCIENTIFIC_REFERENCES.md`.

## Consequências
- Aumento de framerate e eficiência da GPU em ascensões de grande altitude.
- Sensação de salto mais orgânica e responsiva sem alterar a física subjacente.
- Base matemática sólida para expansões futuras do Level Editor.
