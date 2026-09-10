# Decisões de produto — ASCENDENDO

Documento canónico das decisões funcionais. Uma mudança de decisão deve actualizar este ficheiro e o roadmap na mesma tranche.

## 1. Identidade do jogo

ASCENDENDO é um **precision / masochist vertical platformer**. A simplicidade das regras é deliberada: a profundidade vem da precisão, da leitura espacial, do compromisso com cada salto e da aprendizagem.

Não existe adaptive difficulty nem assistência automática de dificuldade. A dificuldade é uma propriedade do conteúdo e da execução.

## 2. Mecânica base — Commitment Jump

- salto parabólico;
- ângulo base de `60°`;
- sem controlo aéreo na versão base 1.0;
- força determinada pelo tempo de carregamento;
- enquanto Jump está premido, a carga sobe continuamente até ao máximo;
- não faz reset nem ciclo durante a mesma carga;
- crescimento linear em 1.0;
- curva de carga parametrizável para experiências futuras sem alterar o contrato de input.

A UI da força deve mostrar a **carga real**. Não deve apresentar `0–255`, barras com resolução artificial ou qualquer número que sugira uma precisão que a simulação não possui. A visualização pode ser contínua e colorida por intensidade; o valor interno da física permanece a autoridade.

## 3. Input do jogador

Defaults:

- `A` + `←` = esquerda;
- `D` + `→` = direita;
- `Space` = Jump.

O jogador deve poder fazer rebind livre das acções suportadas.

## 4. Unidade espacial 1.0

A largura é **sempre e estritamente `640 px`**. Não existe largura configurável de level em 1.0.

Uma **screen** lógica é `640×360`.

Um **level** é uma sequência vertical de `N` screens:

```text
LEVEL
├── SCREEN 0   640×360
├── SCREEN 1   640×360
├── SCREEN 2   640×360
└── SCREEN N-1 640×360
```

`N` é variável; `1`, `3`, `5`, `10` e valores maiores são válidos. Cinco é uma dimensão de design possível, não uma constante do formato.

A altura total do level é `N × 360`.

A progressão é exclusivamente ascendente. O jogador pode cair para screens inferiores, mas isso representa regressão física e nunca progresso deliberado. Não existem ligações de navegação descendentes como parte da rota de 1.0.

Esta estrutura existe porque uma única screen oferece pouca diversidade estrutural quando o jogo mantém deliberadamente poucas mecânicas. Com várias screens, podemos criar níveis maiores e mais variados sem tornar cada unidade visualmente confusa.

## 5. Conteúdo de gameplay de 1.0

Entidades jogáveis oficiais:

- plataformas estáticas;
- personagem/spawn;
- FLAG.

A FLAG é o objectivo final e pertence à última screen do level. Em 1.0 não existem colectáveis, checkpoints, perigos ou entidades que matem o jogador.

O modelo deve permanecer extensível e versionável para permitir posteriormente perigos, plataformas móveis, triggers, colectáveis obrigatórios, checkpoints, objectivos adicionais e outros sistemas. Essas capacidades ficam desligadas/ausentes em 1.0.

## 6. Falha, saída e conclusão

Não existe um estado de “morte” nem uma aba de falha no gameplay de 1.0.

O jogador pode abandonar o level e voltar ao menu.

A arquitectura futura deve permitir entidades que introduzam morte ou outras condições de falha sem obrigar o núcleo 1.0 a usá-las.

A conclusão normal é `spawn → percurso → FLAG`.

## 7. Transição e streaming de levels/screens

A subida é apresentada como uma transição vertical contínua, não como teletransporte entre telas.

O runtime deve manter pelo menos as zonas necessárias acima e abaixo do jogador para que a passagem entre screens seja suave. Deve pré-carregar a próxima zona antes de ser necessária e manter zonas anteriores facilmente recuperáveis durante uma queda.

Uma política de cache/streaming pode ser maior do que o mínimo, desde que não introduza comportamento perceptível nem altere a simulação determinística.

## 8. Level Editor

O Level Editor deve editar um **level completo de largura 640**, composto por `N` screens verticais.

A screen continua a ser a unidade visual fundamental de `640×360`, mas o editor pode navegar verticalmente para trabalhar nas screens do mesmo level.

O editor deve suportar tanto:

```text
teclado apenas
```

como:

```text
teclado + rato
```

O rato é conveniência, nunca requisito de acessibilidade ou operação.

### Operações essenciais por teclado

As operações essenciais do editor devem ter ações semânticas e bindings próprios, incluindo no mínimo:

- navegar pela viewport vertical;
- seleccionar ferramenta/entidade;
- criar/colocar;
- mover;
- apagar;
- mudar preset/tamanho;
- undo;
- redo;
- guardar;
- testar;
- validar;
- sair/voltar.

Arrastar com rato pode ser mais rápido, mas todas as operações essenciais devem continuar a ser possíveis sem rato.

## 9. Playtest e estado não guardado

Testar um level dentro do editor não grava automaticamente as alterações.

O autor pode testar, regressar ao editor e continuar com o documento em memória.

Ao abandonar o editor com `Esc`, alterações pendentes são mostradas num **carrinho de alterações**. Deve ser possível:

- guardar todas;
- descartar todas;
- rever alterações;
- desfazer selectivamente algumas alterações e manter outras.

Isto assenta num histórico geral de undo/redo; o carrinho é uma apresentação de alterações pendentes, não um segundo sistema de histórico.

## 10. Campaign Editor

O Campaign Editor organiza levels como uma playlist/timeline vertical.

Em 1.0 a superfície inicial é:

- seleccionar;
- reordenar;
- abrir no Level Editor;
- voltar preservando contexto;
- validar;
- guardar a ordem em `campaign.txt`.

Criar/remover entries directamente da UI permanece uma questão aberta de UX, não uma lacuna arquitectural.

O Campaign Editor não move ficheiros físicos entre `Levels/`, `Unused/` e `NaoValidados/`; o routing continua sob responsabilidade de `reorganize.py`.

## 11. Identidade de campanhas

Uma campanha deve possuir pelo menos:

- username permanente do criador;
- display name mutável do criador;
- nome público da campanha;
- identificador estável;
- versão;
- número de levels/screens;
- dificuldade;
- estado de validação;
- metadados e agregados de runs quando existirem.

O mesmo criador não pode ter duas campanhas com o mesmo nome público. O stable ID continua a existir independentemente do nome.

Não usar thumbnails como requisito de distinção de campanhas em 1.0; nome, conta e metadata são suficientes.

## 12. Escala de conteúdo 1.0

Mínimo oficial:

| campanhas | levels por campanha |
|---:|---:|
| 1 | 10 |
| 4 | 25 |
| 3 | 50 |
| 1 | 100 |
| 1 | 250 |

Total mínimo: **10 campanhas / 575 levels oficiais**.

São mínimos; podem existir mais.

## 13. Dificuldade

Cada campanha deve ter progressão aproximadamente linear, do fácil para o extremo. Campanhas grandes podem ser divididas em blocos de dificuldade.

O catálogo global deve aproximar-se de uma distribuição centrada na dificuldade normal, com menos campanhas muito fáceis e muito extremas.

A escala nominal de dificuldades **não é fixada por um número arbitrário**. O número de classes deve ser derivado directamente do modelo/fórmula de dificuldade quando essa fórmula estiver definida. Se o modelo justificar 18, 19 ou outro número de separações, a UI e o catálogo usarão esse número.

## 14. Difficulty Tester e validação

A validação obrigatória de publicação distingue:

```text
formato
→ geometria
→ regras físicas
→ conectividade/progressão
→ análise de percurso
→ dificuldade
→ experiência humana
```

A comunidade deve passar pelo menos pelo validador antes de publicar.

Um level difícil pode ser válido. O validador não deve reescrever automaticamente o conteúdo para o tornar fácil.

## 15. Runs, replay e ranking

A métrica principal de desempenho é **tempo**.

Uma run começa no spawn e termina na FLAG. Não há mortes nem altura máxima em 1.0.

Os dados desejados incluem:

- melhor tempo global;
- tempo médio;
- melhor jogador;
- tempo estimado por rotas mais rápidas/difíceis;
- tempo estimado por rotas mais fáceis/demoradas.

A run deve ser determinística: o replay representa os movimentos/inputs e o motor reproduz e valida esses inputs novamente. O tempo obtido pela reprodução válida é a medida autoritativa da run.

A leaderboard pública pode ser construída em cima desse resultado quando a infraestrutura estiver pronta.

## 16. Visual

Direcção 1.0:

- indie;
- simples;
- pixel art básica;
- gameplay readability acima de decoração;
- foreground para personagem/plataformas;
- background em camadas;
- parallax com velocidades relativas diferentes.

Assets base podem ser compostos por peças `16×16`.

A colocação no mundo é pixel-perfect e não é limitada a uma grelha global de `16×16`.

### 16.1 Plataformas contínuas e auto-tiling

A unidade lógica que o autor coloca no level é uma **plataforma contínua de um material**, não uma sequência de tiles pintados manualmente.

A posição da plataforma no mundo é pixel-perfect e pode começar em qualquer coordenada válida. **Não existe lock obrigatório da posição a uma grelha global de 16×16.**

Para a composição visual canónica, a largura e a altura da plataforma devem ser **positivas e múltiplas de `16 px`**. Assim, por exemplo, uma plataforma pode começar em `x = 347` e ter `160 px` de largura, mas não `163 px`.

O sistema cria uma **malha visual local de `16×16`** a partir da geometria contínua da plataforma. Essa malha serve para decompor a superfície em peças de pixel art; não é uma nova autoridade de posicionamento ou colisão.

O auto-tiling decide automaticamente que sprite/variante deve ser desenhado em cada célula visual com base, entre outros factores suportados, em:

- exposição do topo, fundo e lados;
- cantos e extremidades;
- vizinhança cardinal e diagonal;
- continuidade e contacto com outras regiões;
- material da própria região e do vizinho;
- fronteiras entre materiais;
- variantes visuais elegíveis.

Plataformas/regiões de materiais diferentes podem ligar-se visualmente quando a sua geometria realmente partilha pixels/arestas ou contacto relevante. O sistema deve usar a geometria em world-space para determinar essas adjacências, mesmo quando as origens locais das regiões não estão alinhadas a uma grelha global.

A composição deve impedir escolhas visualmente incoerentes. Por exemplo, uma peça de topo com relva não deve aparecer numa zona que está visualmente coberta por outra região compatível imediatamente acima/abaixo.

A variedade pode incluir **randomização controlada e determinística** entre sprites compatíveis para reduzir repetição de pedaços de terra simples, mantendo a aparência coerente com a topologia e material. A mesma entrada deve produzir a mesma escolha quando a reprodução determinística o exigir.

A ausência de uma variante elegível deve produzir um fallback determinístico ou estado sem vencedor visual, mas **nunca deve alterar a geometria de gameplay**.

O auto-tiling é exclusivamente uma decisão de **Presentation**. Nunca pode arredondar, mover, expandir, encolher ou substituir a geometria contínua usada pelo gameplay/collision.

O objectivo de produto é que o autor possa dizer essencialmente **“esta é uma plataforma de terra”** e obter automaticamente a composição pixel-art adequada, em vez de ter de construir manualmente os cantos, bordas, interiores e ligações tile a tile.

## 17. Sprites e licenciamento

Sprites e outros assets visuais são opções curadas. Não existe editor de arte livre no núcleo 1.0.

Todo asset externo precisa de ficha formal de origem/licença no repositório, incluindo recursos CC0.

## 18. Áudio

1.0 inclui música retro simples e livre para redistribuição e efeitos de jogo/editor.

Efeitos não-musicais podem ter vários takes/variações com selecção pseudo-aleatória controlada para reduzir repetição. A música permanece estável durante a reprodução.

Pesquisa aprofundada de música e autoria áudio fica para a fase final de 1.0, não bloqueia agora a arquitectura.

## 19. Partilha e comunidade

A unidade principal de publicação é a **campanha**.

`.lvl` continua a ser uma unidade técnica de import/export e armazenamento do level individual. Não é descartado.

A arquitectura deve poder evoluir para um pacote de campanha superior a `.lvl`, contendo levels, layout, metadata, dificuldade, validação e assets.

Importar/exportar campanhas é obrigatório para 1.0.

## 20. Distribuição

Obrigatório em 1.0:

- Windows x64 standalone/executável;
- site público para partilha de conteúdo e rankings;
- validação de campanhas;
- difficulty tester.

Versão web é desejável, mas não bloqueia 1.0.

## 21. Futuro / 1.1+

A generalização de arquitectura deve favorecer mods, expansões e jogos derivados em direcções arbitrárias sem obrigar 1.0 a pagar o custo de mecânicas que ainda não têm razão de design.

Candidatos incluem perigos, plataformas móveis, triggers, colectáveis obrigatórios, checkpoints, novos objectivos, conteúdos horizontais e outros modelos de jogo. Nada disto altera as regras de 1.0 descritas acima.
