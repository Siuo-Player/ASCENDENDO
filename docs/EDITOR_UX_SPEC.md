# Especificação UX — Editor

Esta especificação transforma as decisões de produto em critérios verificáveis antes da implementação da 9.5.

## 0. Objetivo do editor

O editor é parte da infraestrutura do produto. Deve ser suficientemente confiável para que mapas criados pela comunidade possam continuar a funcionar quando o jogo evoluir.

Isso implica:

- operações previsíveis;
- feedback claro de erro;
- playtest rápido;
- validação independente;
- formato de nível extensível e versionável;
- preservação de compatibilidade sempre que possível.

Adicionar novos objetos no futuro não deve exigir reconstruir o editor do zero.

## 1. Entrada e descoberta

Ao entrar no editor, o utilizador deve conseguir perceber:

- qual é o modo atual;
- qual é a ferramenta ativa;
- qual é o tamanho/preset ativo;
- como guardar, testar, validar e sair;
- como mudar para o Campaign Editor.

Nenhuma ação crítica pode depender exclusivamente da memória de um atalho.

## 2. Viewport

### Level Editor

O nível inteiro `640x360` deve permanecer visível simultaneamente.

A implementação pode reduzir a escala de apresentação para caber no viewport físico, mas nunca pode:

- cortar a esquerda/direita;
- cortar topo/fundo;
- deformar o rácio;
- esconder a moldura do nível.

### Campaign Editor

A timeline pode ser maior que a área visível e pode fazer scroll vertical. Cada bloco individual, porém, deve estar integralmente enquadrado quando está em foco.

## 3. Layout adaptável

Componentes de UI devem preferir layouts relativos/autoajustáveis em vez de posições fixas.

### Texto

Texto que não caiba deve usar, por ordem de preferência:

1. reduzir o espaço ocupado mantendo legibilidade;
2. quebrar linha de forma controlada quando o contexto permitir;
3. truncar com indicação visual quando uma linha única for obrigatória.

Nunca simplesmente desaparecer pela extremidade do ecrã.

### Menus

O cálculo de largura deve depender do número de opções e do espaço útil disponível.

### Rodapés

O texto contextual deve respeitar margens mínimas e adaptar a sua escala/composição ao espaço restante.

## 4. Ferramentas

O Level Editor deve manter poucos modos e tornar o estado ativo óbvio.

Estado mínimo:

- `STAMP` — criar conteúdo;
- `DRAG` — selecionar/mover conteúdo.

O tamanho do elemento é um preset explícito. O preset `MEDIUM` é o default.

## 5. Snap e grelha

A grelha visual e as regras de composição visual não são necessariamente iguais à posição de autoria.

A posição de uma plataforma no mundo é **pixel-perfect** e não está bloqueada a uma grelha global de `16×16`. O editor não deve impor snap obrigatório de 4 px.

Para a composição automática de pixel art, a **largura e a altura da plataforma devem ser múltiplas de `16 px`**. O editor pode mostrar uma grelha visual de `16 px` como auxílio para dimensões e composição, mas essa grelha não restringe a coordenada inicial da plataforma.

Assim, é válido ter, por exemplo, uma plataforma que começa em `x = 347` e tem `160 px` de largura. A plataforma é uma região contínua; as peças `16×16` usadas pelo renderer são uma decisão de apresentação.

## 6. Auto-tiling e composição visual de plataformas

O autor não deve precisar de colocar manualmente cada tile de uma plataforma.

A operação lógica é criar/seleccionar uma **plataforma contínua de um material** — por exemplo, terra — e editar a sua posição e dimensões. O sistema compõe automaticamente a pixel art dessa região.

```text
plataforma contínua
        ↓
malha visual local 16×16
        ↓
análise de vizinhança/material
        ↓
auto-tiling
        ↓
escolha de sprite/variante
        ↓
render
```

A composição deve escolher automaticamente peças adequadas para:

- topo exposto;
- fundo e interior;
- laterais e extremidades;
- cantos;
- ligações entre regiões;
- fronteiras entre materiais;
- vizinhança cardinal e diagonal;
- variantes de pixel art compatíveis.

Plataformas diferentes podem ligar-se visualmente quando a sua geometria realmente partilha pixels/arestas ou contacto relevante. Essa relação é determinada em world-space, não por uma grelha global.

O sistema pode escolher variantes de terra simples e visualmente compatíveis com **randomização controlada e determinística**, reduzindo a repetição sem introduzir um sprite incompatível com a topologia. Repetir o mesmo conteúdo deve continuar a produzir uma composição reproduzível quando o contexto exige determinismo.

O auto-tiling é exclusivamente visual. Nunca altera a posição, dimensões, colisão ou qualquer outra propriedade autoritativa da plataforma.

Quando não existe uma variante perfeita, deve ser usada uma escolha de fallback determinística ou um estado visual sem vencedor. O sistema nunca deve alterar a geometria para resolver a falta de um asset.

## 7. Feedback de seleção, preview e salto

Uma seleção deve ser visualmente inequívoca.

O preview deve indicar se a operação atual é permitida. Se a posição for inválida, o estado deve ser distinguível de um preview que será aplicado.

Durante o jogo/playtest, a força do salto deve ser visualmente percetível através de uma barra/indicador associado ao personagem. O jogador não deve precisar de inferir a carga exclusivamente pelo tempo de tecla.

O indicador deve comunicar:

- carga atual;
- mínimo/máximo;
- momento em que a ação pode ser libertada;
- possibilidade de cancelar quando a UX/accessibilidade justificar.

A animação exata da barra continua em aberto até à implementação e teste visual.

## 8. Playtest

O playtest deve ser um modo de observação, não uma gravação automática.

Fluxo:

```text
EDITOR
  ↓
PLAYTEST
  ↓
observar Player/física
  ↓
voltar
  ↓
EDITOR com alterações intactas
```

Guardar é sempre explícito.

Durante o playtest, deve ser possível perceber claramente a força do salto, o percurso do jogador e o ponto em que uma tentativa falha.

## 9. Validação

A validação deve produzir pelo menos:

- estado global: válido/inválido;
- indicação da primeira causa útil;
- área/objeto relacionado quando possível;
- percurso de tentativa quando houver uma simulação disponível;
- ponto aproximado da falha;
- atualização suficientemente rápida para acompanhar a edição.

As causas devem preferir mensagens compreensíveis, como:

- alvo demasiado longe;
- ângulo impossível;
- trajetória sem plataforma alcançável;
- colisão lateral;
- transição seguinte inacessível.

No Campaign Editor, runs visuais complementam esta informação mostrando onde uma tentativa está e onde uma transição falha.

## 10. Campanhas

`Começar` passa por uma seleção explícita de campanha mesmo quando existe apenas uma opção.

A UI deve mostrar:

- nome da campanha;
- preview/miniatura quando disponível;
- número de níveis quando disponível;
- estado de validade quando disponível;
- ação clara para iniciar;
- voltar ao menu.

## 11. Campaign Editor

O Campaign Editor deve:

- apresentar níveis verticalmente como miniaturas 16:9;
- permitir scroll vertical;
- permitir seleção de um nível;
- permitir drag/reordenação com snap;
- permitir abrir o Level Editor por mudança explícita de estado;
- mostrar o estado de validação de cada nível;
- mostrar vários agentes/runs em background quando possível;
- mostrar pelo menos uma tentativa por nível quando a capacidade permitir;
- mostrar tentativas que atravessem a fronteira entre níveis para diagnosticar a continuidade da campanha.

## 12. Critérios de aceitação da 9.5

A tranche é considerada UX-completa quando:

- o Level Editor não corta o canvas em fullscreen ou janela redimensionada;
- os comandos principais são descobríveis sem documentação externa;
- `1/2/3` executam guardar/testar/validar;
- `0` abre a consulta de Controlos;
- playtest não grava automaticamente;
- a barra/indicador de força torna a carga do salto claramente visível;
- a validação dá feedback útil durante a edição;
- o diagnóstico mostra o percurso tentado e causa/local de falha quando disponível;
- `Começar` mostra seleção de campanha;
- Campaign Editor permite ver e reorganizar a ordem vertical dos níveis;
- nenhum texto importante ultrapassa as margens do viewport;
- uma plataforma pode ser criada como uma região contínua sem pintura tile-a-tile;
- a composição automática mantém bordas/cantos/ligações coerentes e usa apenas variantes compatíveis;
- nenhuma decisão de auto-tiling altera a geometria de gameplay;
- a introdução futura de conteúdo não exige substituir o modelo de nível já existente.
