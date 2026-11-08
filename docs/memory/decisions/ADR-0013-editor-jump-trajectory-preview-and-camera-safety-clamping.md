# ADR-0013: Pré-visualização de Trajetória Balística no Editor e Clamping de Segurança na Câmara

## Estado
Aprovado

## Contexto
1. **Authoring de Precisão no Editor de Níveis**: Conforme demonstrado por Aramini et al. (2018) e Smith et al. (2008), o authoring de platformers de precisão sem feedback visual de alcance físico induz a erros de design e níveis involuntariamente impassáveis. Com o módulo `JumpTrajectory` em forma analítica fechada, o editor pode fornecer feedback instantâneo da envolvente balística.
2. **Continuidade Espacial e Enquadramento da Câmara (Game Feel)**: Steve Swink (2008, *Game Feel*, Capítulo 13) estabelece que uma câmara suave nunca pode permitir que o avatar escape do frustum visível durante acelerações bruscas (como quedas em velocidade terminal ou ressaltos verticais intensos).
3. **Usabilidade de Rato no Menu Principal**: Permitir clique direto na barra de estado do Fantasma/Replay para alternar o fantasma (metade esquerda) ou iniciar a reprodução do recorde (metade direita).

## Decisão
1. **EditorRenderSnapshot & EditorRenderer**:
   - Adicionar vetores de pontos discretos (`trajectoryMinRight`, `trajectoryMaxRight`, `trajectoryMinLeft`, `trajectoryMaxLeft`) ao `EditorRenderSnapshot`.
   - `EditorSession::renderSnapshot()` amostra as trajetórias analíticas a partir do topo-centro da plataforma selecionada ou do chão do spawn.
   - `EditorRenderer` desenha os arcos tracejados (ciano suave a 65% de opacidade para salto mínimo de 23.9px; dourado/âmbar brilhante a 85% de opacidade para salto de carga total de 137.7px).
2. **Camera Safety Framing (`Camera::follow`)**:
   - Manter a interpolação suave (Lerp exponencial com `alpha = speed * dt`) no interior do enquadramento normal (35% vertical offset).
   - Aplicar clamping rígido de emergência: jogador nunca fica a menos de 24px da borda inferior da câmara em quedas livres velozes, e nunca a menos de 32px da borda superior em lançamentos bruscos.
3. **Zero Impacto no Determinismo da Simulação**:
   - Todas as alterações são estritamente de apresentação, authoring e HUD. O `PhysicsBody` canónico ($16 \times 16$, gravidade $-980\text{ px/s}^2$, $60\text{ Hz}$) permanece bit-a-bit invariante.

## Consequências
- Os autores de níveis conseguem inspecionar visualmente se um salto é exequível antes de testar.
- Eliminação definitiva de qualquer caso onde o jogador fique temporariamente invisível na base do ecrã durante quedas de grande altitude.
- Operação 100% offline, determinística e sem alocações em tempo de execução crítico.
