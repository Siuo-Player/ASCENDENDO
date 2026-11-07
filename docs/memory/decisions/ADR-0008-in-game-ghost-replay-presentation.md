# ADR-0008: In-Game Ghost Replay Overlay e Simulação de Fantasma em Tempo Real

## Contexto
Com a implementação do formato de replay determinístico e autenticado (`RunReplay` em ADR-0007), surgiu a oportunidade de oferecer feedback visual imediato ao jogador durante as subidas de campanha. Em jogos de precisão e speedrunning, competir lado a lado contra a representação visual (*Ghost / Fantasma*) do seu próprio melhor tempo pessoal (PB - *Personal Best*) ou da corrida de um rival eleva a motivação, o ritmo de aprendizagem e a capacidade de optimização da trajetória balística.

## Decisão

### 1. Desacoplamento da Apresentação (`RenderGhostSnapshot`)
- O tipo de domínio do jogador fantasma não polui o pipeline de renderização Vulkan.
- Foi adicionada a estrutura imutável de apresentação `RenderGhostSnapshot` em `RenderSnapshot.h`, contendo apenas `bounds`, `facingDirection`, `alpha` e `visible`.
- Em `WorldRenderer.cpp`, o fantasma é desenhado antes do jogador com uma tonalidade azul ciano luminoso translúcido (`COLOR_GHOST_ALPHA = 0.55f`), garantindo distinguibilidade imediata do jogador real sem poluição visual.

### 2. Simulação Paralela Determinística em `GameSession`
- `GameSession` deteta e carrega automaticamente o ficheiro `latest_<campaignID>.replay` do cofre do jogador em `userdata/Accounts/<hash>/Replays/`.
- Durante o avanço de ticks fixos a 60 Hz em `core::GameState::PLAYING`, o `ghostPlayer_` avança passo a passo consumindo a sequência exata de inputs gravada.
- O fantasma colide com as mesmas plataformas através de `level_.resolveCollision(ghostPlayer_->body)`, mas não interfere na hitbox ou movimento do jogador principal.

### 3. Controlo de Apresentação e Atalho (Tecla `G`)
- Foi adicionada a ação de jogo `GameAction::ToggleGhost` mapeada por omissão para a tecla `G` durante o gameplay (`core::GameState::PLAYING`).
- O jogador pode ligar ou desligar o fantasma a qualquer instante sem interromper a fluidez da subida.

## Consequências
- A experiência de jogo ganha um elemento competitivo tangível e viciante (bater a própria marca visualmente).
- Preserva o determinismo absoluto e o modelo de renderização orientado a snapshots.
