# Especificação UX — Editor

Esta especificação transforma as decisões de produto em critérios verificáveis antes da implementação da 9.5.

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

A grelha visual e o snap não são necessariamente iguais.

Configuração atual:

- snap: 4 px;
- grelha visual: 16 px;
- linhas principais: 64 px.

A grelha deve ajudar a alinhar blocos e não competir visualmente com eles.

## 6. Feedback de seleção e preview

Uma seleção deve ser visualmente inequívoca.

O preview deve indicar se a operação atual é permitida. Se a posição for inválida, o estado deve ser distinguível de um preview que será aplicado.

## 7. Playtest

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

## 8. Validação

A validação deve produzir pelo menos:

- estado global: válido/inválido;
- indicação da primeira causa útil;
- área/objeto relacionado quando possível;
- atualização suficientemente rápida para acompanhar a edição.

No Campaign Editor, runs visuais complementam esta informação mostrando onde uma tentativa está e onde uma transição falha.

## 9. Campanhas

`Começar` passa por uma seleção explícita de campanha mesmo quando existe apenas uma opção.

A UI deve mostrar:

- nome da campanha;
- número de níveis quando disponível;
- estado de validade quando disponível;
- ação clara para iniciar;
- voltar ao menu.

## 10. Critérios de aceitação da 9.5

A tranche é considerada UX-completa quando:

- o Level Editor não corta o canvas em fullscreen ou janela redimensionada;
- os comandos principais são descobríveis sem documentação externa;
- `1/2/3` executam guardar/testar/validar;
- `0` abre a consulta de Controlos;
- playtest não grava automaticamente;
- a validação dá feedback útil durante a edição;
- `Começar` mostra seleção de campanha;
- Campaign Editor permite ver e reorganizar a ordem vertical dos níveis;
- nenhum texto importante ultrapassa as margens do viewport.
