# Editor de Níveis do ASCENDENDO

O Level Editor é integrado nativamente no executável do jogo, operando sob o estado `GameState::EDITOR`. Não é uma ferramenta externa descartável: é um componente central do produto concebido para permitir que qualquer jogador crie, teste, valide e partilhe desafios verticais de precisão.

---

## 1. Princípios de Desenho e Ergonomia

1. **Grid-First e Snapping Visível**: Todas as operações de edição respeitam a quantização ao grid físico de 4 px (`EDITOR_GRID_SNAP`), com grelha visual a cada 16 px (`EDITOR_GRID_VISUAL_SPACING`) e linhas principais a cada 64 px.
2. **Duplo Modal de Interação (Teclado e Rato)**: Todas as operações essenciais podem ser operadas 100% por teclado ou com o auxílio do rato.
3. **Histórico Transacional com Undo/Redo por Gesto**: Um arrasto ou redimensionamento completo representa uma única transação no histórico de undo/redo (`Ctrl+Z` / `Ctrl+Y`), independentemente do número de frames decorridos. Uma nova edição limpa a pilha de redo.
4. **Estado Derivado Canónico (Spawn e FLAG)**:
   - **SPAWN**: O spawn do jogador é estado derivado a partir do chão implícito ($Y=0..16$) e da posição horizontal canónica ($X=320$). Nunca é serializado como uma ferramenta de authoring separada.
   - **FLAG**: O objetivo final da campanha é derivado automaticamente no último nível da campanha a partir da plataforma authored mais alta. Nunca é serializado como entidade solta.
5. **Pré-visualização Balística em Tempo Real (Cones de Alcance)**:
   - Baseado em Aramini et al. (2018) e Steve Swink (2008), o editor renderiza os arcos balísticos do Commitment Jump a 60° a partir do topo-centro da plataforma selecionada ou do chão do spawn.
   - **Arco Ciano Suave**: Alcance de toque rápido ($c=0.0$, altura máxima de $23.9\text{ px}$).
   - **Arco Dourado Brilhante**: Alcance com carga completa ($c=1.0$, altura máxima de $137.7\text{ px}$).
   - O autor inspeciona visualmente se o salto pretendido é exequível sem adivinhação.

---

## 2. Modos de Edição e Manipulação

### STAMP (Modo Padrão)
- Clique esquerdo em espaço vazio cria instantaneamente uma plataforma com o tamanho do preset selecionado (Small: 64×16, Medium: 128×20, Large: 256×24).
- Alterna para modo DRAG com a tecla `[G]`.

### DRAG (Criação por Arrasto)
- Clique e arrasto define a geometria exata da plataforma, quantizada de imediato ao grid.

### Manipulação Direta e Redimensionamento
- Clicar sobre uma plataforma existente seleciona-a, destacando-a com uma borda amarela brilhante.
- Quatro pegas de canto (*corner handles*) quadradas de 8×8 px permitem redimensionar diretamente a plataforma em qualquer direção.
- Arrastar o corpo da plataforma move-a mantendo as suas dimensões.
- A tecla `[Delete]` ou `[Backspace]`, ou o botão direito do rato, apaga a plataforma selecionada.

---

## 3. Validação Física e Diagnóstico Visível

O editor inclui um validador assíncrono em segundo plano:
- **Plataformas Verdes**: Plataformas fisicamente alcançáveis a partir do chão inicial através de saltos válidos a 60°.
- **Plataformas Vermelhas**: Plataformas inalcançáveis ou isoladas no espaço.
- O renderer consome um `EditorRenderSnapshot` já enriquecido com os diagnósticos do validador, sem recalcular física pesada no ciclo de desenho.

---

## 4. Tabela de Atalhos de Teclado no Editor

| Ação Semântica | Atalho Default | Função |
|---|---|---|
| `MoveLeft` | A / ← | Mover cursor para a esquerda |
| `MoveRight` | D / → | Mover cursor para a direita |
| `EditorPanUp` | W / ↑ | Deslocar janela de visão para cima |
| `EditorPanDown` | S / ↓ | Deslocar janela de visão para baixo |
| `EditorToggleMode` | G | Alternar entre STAMP e DRAG |
| `EditorSizeDown` | [ | Reduzir preset de tamanho (L $\to$ M $\to$ S) |
| `EditorSizeUp` | ] | Aumentar preset de tamanho (S $\to$ M $\to$ L) |
| `DeleteSelection` | DELETE / BACKSPACE | Eliminar plataforma selecionada |
| `Undo` | Ctrl + Z | Desfazer última alteração de histórico |
| `Redo` | Ctrl + Y | Refazer alteração desfeita |
| `Playtest` | P | Testar o nível imediatamente no motor real |
| `Pause` / Voltar | ESC | Sair do editor e regressar ao Menu |
