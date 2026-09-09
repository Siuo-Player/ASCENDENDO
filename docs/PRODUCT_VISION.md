# ASCENDENDO — Visão do produto

> Documento canónico de visão: define o jogo, a experiência e as propriedades deliberadas do produto. Decisões funcionais ficam em `docs/PRODUCT_DECISIONS.md`; a ordem de execução fica em `docs/ROADMAP.md`.

## 1. O que é

ASCENDENDO é um **precision / masochist vertical platformer** construído sobre um motor 2D próprio.

A experiência deve ser simples de compreender e difícil de dominar: ler o espaço, escolher onde aterrar, carregar o salto, libertá-lo e comprometer-se com a trajectória. A profundidade nasce da precisão, desenho de plataformas, memória muscular, repetição e aprendizagem.

O produto é simultaneamente:

```text
JOGO + AUTORIA + PLAYTEST + VALIDAÇÃO + DETERMINISMO/REPLAY
+ CAMPANHAS + COMUNIDADE + ASSETS/LICENCIAMENTO + DISTRIBUIÇÃO
```

## 2. Regras de jogo 1.0

### Commitment Jump

- salto parabólico;
- ângulo base `60°`;
- sem controlo aéreo;
- força dependente do tempo de carga;
- carga contínua enquanto Jump está premido;
- sem reset/ciclo durante a mesma carga;
- crescimento linear em 1.0;
- função parametrizável para experiências futuras.

A força apresentada ao jogador deve representar directamente o estado real da simulação. **Não existe UI `0–255` ou outra escala artificial de precisão.** A barra pode ser contínua, visual e colorida por intensidade.

### Movimento e controlos

Defaults: `A`/`←` esquerda, `D`/`→` direita, `Space` Jump. O jogador pode remapear livremente as acções disponíveis.

## 3. Estrutura espacial

A largura é uma constante do jogo em 1.0:

```text
SCREEN = 640 × 360
LEVEL  = 640 × (N × 360)
```

Um `.lvl` representa um **level vertical composto por N screens de 640×360**. `N` é variável. Não há largura configurável.

A progressão é exclusivamente ascendente. O jogador pode cair para screens inferiores e regressar fisicamente, mas nunca progride deliberadamente para baixo.

Esta estrutura existe para aumentar a diversidade e escala dos desafios sem abandonar a legibilidade de uma tela de `640×360`. Uma campanha pode assim conter níveis curtos ou longos sem transformar cada screen num espaço excessivamente denso.

## 4. Conteúdo 1.0

Só existem três entidades de gameplay:

- plataformas estáticas;
- jogador/spawn;
- FLAG.

A FLAG fica no fim do level, na última screen. Não há morte em 1.0, nem estado de failure/death. O jogador pode apenas sair e regressar ao menu.

O formato e as interfaces devem ser extensíveis para futuras entidades como perigos, moving platforms, triggers, colectáveis obrigatórios, checkpoints e outros objectivos, mas essas mecânicas permanecem fora de 1.0.

## 5. Transição e streaming

A passagem entre screens é contínua. O jogo deve manter simultaneamente a zona actual e as zonas necessárias para uma transição suave, pré-carregando a próxima e mantendo zonas anteriores recuperáveis durante uma queda.

O carregamento nunca deve ser uma pausa visível nem alterar o resultado determinístico da simulação.

## 6. Level Editor

O Level Editor edita o **level completo**, mas preserva a screen de `640×360` como unidade de autoria e leitura.

O editor tem dois modos de utilização igualmente válidos:

```text
teclado apenas
teclado + rato
```

As operações essenciais têm sempre acções semânticas e bindings de teclado: navegação vertical, selecção, colocar, mover, apagar, trocar ferramenta, undo, redo, guardar, testar, validar e sair. O rato/drag é acelerador, não requisito.

O playtest acontece dentro do jogo e não grava automaticamente as alterações.

## 7. Estado de edição e alterações pendentes

O documento em memória é separado do estado persistido.

`Esc` abre um carrinho de alterações quando existem mudanças pendentes. O utilizador pode guardar tudo, descartar tudo ou rever alterações. A revisão permite desfazer selectivamente mudanças e manter as restantes quando suportado.

Isto assenta num histórico geral de undo/redo.

## 8. Campaign Editor

O Campaign Editor gere a sequência de levels como playlist/timeline vertical.

1.0 começa com `select → reorder → open`, seguido de retorno preservando contexto, validação e escrita de `campaign.txt`.

A criação/remoção de entries directamente nesta UI continua uma questão de investigação UX. Não é uma dependência para estabilizar a arquitectura.

## 9. Campanhas 1.0

Mínimo oficial: **10 campanhas e 575 levels**:

| campanhas | levels |
|---:|---:|
| 1 | 10 |
| 4 | 25 |
| 3 | 50 |
| 1 | 100 |
| 1 | 250 |

Cada campanha evolui aproximadamente do fácil para o difícil/extremo; campanhas grandes podem usar blocos internos. O catálogo global deve concentrar-se numa dificuldade normal e ter menos extremos.

A escala nominal de dificuldades será derivada da fórmula/modelo real de dificuldade. Não existe ainda um número arbitrário congelado de classes.

## 10. Validação, dificuldade e runs

Conteúdo publicável deve passar pelo validador e pelo difficulty tester. A validação distingue formato, geometria, regras físicas, progressão, percurso e dificuldade.

Uma run é determinística:

```text
spawn → inputs/movimentos gravados → FLAG
```

O replay é reexecutado pelo motor e validado novamente. O tempo resultante dessa execução válida é a medida autoritativa.

A métrica de ranking é tempo. Devem poder existir melhor tempo global, média, melhor jogador e estimativas de tempo por rotas rápidas/difíceis e fáceis/demoradas.

## 11. Visual

Direcção 1.0: **indie, simples, pixel-art básica**. A legibilidade do gameplay é prioritária.

Foreground: jogador e plataformas. Background: atmosfera/profundidade. Parallax: camadas com velocidades relativas.

Assets base usam peças `16×16`, mas a colocação no mundo é pixel-perfect e não está limitada a uma grelha de 16 px. Sprites são opções curadas.

Todo asset externo tem ficha formal de origem/licença no repositório, inclusive CC0.

## 12. Áudio

1.0 inclui música retro simples livre para redistribuição e efeitos para salto/carga/libertação, aterragem, conclusão e UI/editor. Efeitos não-musicais podem ter takes múltiplos com selecção aleatória controlada; a música permanece estável.

Pesquisa aprofundada de autoria musical fica para perto do fecho de 1.0.

## 13. Comunidade e publicação

A unidade principal de publicação é a **campanha**. `.lvl` continua a ser unidade técnica de armazenamento/import/export do level.

Uma camada futura de package pode reunir campaign identity, levels, layout, metadata, difficulty, validation e assets.

1.0 requer import/export de campanhas e um site público para partilha e rankings.

## 14. Distribuição e horizonte

Obrigatório para 1.0:

- Windows x64 standalone;
- campanha validada;
- difficulty tester;
- site público de partilha/ranking.

Versão web é desejável, não obrigatória.

A arquitectura deve ser aberta a mods, expansões e jogos derivados em direcções futuras, mas sem introduzir em 1.0 mecânicas que ainda não tenham uma razão de design.
