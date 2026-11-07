# ADR-0009: Speedrun HUD, Barra Ergonómica de Salto e Navegador de Campanhas

## Estado
Aceite (2026-11-07)

## Contexto
O ASCENDENDO é uma plataforma de precisão vertical sem mortes orientada ao time-attack, corrida contra o relógio e partilha comunitária.
Para atingir o padrão de excelência da versão 1.0 e satisfazer os requisitos do utilizador e do ROADMAP.md:
1. **Ergonomia dos controlos e clareza de força**: O Commitment Jump (salto a 60° com carga linear de 0.4s) necessitava de uma barra visual de força visível acima da cabeça do avatar para feedback tátil e espacial imediato durante a preparação do salto.
2. **Speedrun HUD e telemetria de subida**: Durante a ascensão vertical, o jogador precisa de ver o tempo com precisão de centésimos de segundo (`MM:SS.cc`), o recorde pessoal da campanha (`PB`), a altitude alcançada em metros (`ALTURA: X m`, onde 16 px = 1 bloco = 1 metro), o estado do fantasma (`[G] GHOST`) e o split delta face ao fantasma em tempo real (verde se avançado `▲ +X m`, vermelho se atrasado `▼ -X m`).
3. **Seletor e Navegador de Campanhas no Menu**: O catálogo oficial dispõe de 10 campanhas canónicas com progressão vertical (totalizando 610 níveis). O menu principal necessitava de permitir navegar, inspecionar estatísticas de conclusão e alternar ativamente entre as diferentes campanhas (via `W`/`S`, `Up`/`Down`, `[` / `]`, ou cliques do rato nas setas).

## Decisões

1. **Barra de Carga de Salto Desacoplada (`chargeRatio`)**:
   - `RenderPlayerSnapshot` recebe o campo `float chargeRatio = 0.0f;` (`[0.0f, 1.0f]`).
   - `WorldRenderer` desenha a barra acima da cabeça do avatar em coordenadas de mundo apenas quando `chargeRatio > 0.01f`, com transição de cor suave de amarelo para âmbar/vermelho à medida que a força atinge 100%.

2. **Telemetria e HUD de Speedrun (`RenderHudSnapshot`)**:
   - O builder de snapshot recebe `RenderHudSnapshot` sem acoplamento a tipos de simulação ou ficheiros.
   - Apresentação em `UiRenderer::drawGameplayHud` com fundo translúcido discreto no topo do ecrã, formatação de alta precisão em centésimos (`formatTimerHundredths`) e cálculo determinístico de altitude derivado de $Y_{player} / 16.0f$.

3. **Navegador e Gestor de Campanhas em `GameSession`**:
   - `GameSession` mantém `availableCampaigns_` e `activeCampaignIndex_`.
   - Métodos `selectCampaign(index)`, `selectNextCampaign()` e `selectPreviousCampaign()` permitem ciclo contínuo e carregamento atómico da playlist e do respetivo fantasma (`latest_<campaignID>.replay`).
   - `MenuRenderSnapshot` fornece a lista de campanhas, contagem de níveis e recordes de perfil ao `UiRenderer::drawMenu`.

## Consequências
- Os 453 testes de unidade e integração C++ e os 53 testes Python passam com 100% de sucesso (69,292 asserções validadas).
- O executável do jogo e o pacote distribuível `ascendendo-v1.0.0-windows-x64.zip` contêm a experiência completa e responsiva pronta a jogar.
