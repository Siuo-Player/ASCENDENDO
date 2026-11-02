# Gotcha 0001 — Física de Cabeçada (Head-Bonk) e FLAG Derivada

- **Área:** Física de Plataformas / Testes de Replay / Validação
- **Data:** 2026-11-02

## Problema

1. O jogador possui uma caixa de colisão de 16×16 (`config::PLAYER_WIDTH`, `config::PLAYER_HEIGHT`).
2. O Commitment Jump lança a um ângulo fixo de 60°. Com força máxima (`PLAYER_MAX_JUMP = 600`), o vértice da parábola ascende $\Delta y = 137.7\text{ px}$ a $\Delta x = 159\text{ px}$.
3. Se uma plataforma superior estiver posicionada acima da trajetória de subida onde a parte inferior (underside) toca no topo da cabeça do jogador, ocorre uma colisão de cabeçada ("head-bonk"): a velocidade vertical `vy` é anulada e o jogador cai antes de conseguir alcançar a plataforma.
4. Plataformas de aterragem devem ser posicionadas de modo a que a trajetória descendente chegue por cima (`player.body.bounds().max.y <= platform.y` no momento da aterragem).
5. A FLAG nunca é serializada nos ficheiros `.lvl`. O motor deriva a FLAG automaticamente no último nível da playlist a partir da plataforma com maior coordenada Y. A área da FLAG fica imediatamente no topo dessa plataforma com altura fixa de 40 unidades.
