# Referências de design — editores e jogos abertos

Este documento regista referências externas usadas para orientar o desenho do ASCENDENDO. Não são especificações para copiar literalmente: servem para identificar padrões de interface que já foram exercitados em projetos reais.

## 1. Tiled — referência para edição espacial e snapping

[Tiled](https://github.com/mapeditor/tiled) é um editor de mapas open source usado também em jogos de plataformas. A documentação destaca uma separação clara entre mapa, layers, tiles e objetos e uma ferramenta de criação orientada para edição rápida.

### O que aproveitamos

- separar claramente o documento de nível da camada de apresentação;
- tratar snapping como ferramenta de criação, não como detalhe escondido do motor;
- permitir que o editor mostre a estrutura do conteúdo de forma previsível;
- manter o formato do mapa declarativo e legível;
- pensar desde cedo em compatibilidade e evolução do formato.

### O que não copiamos

O ASCENDENDO não precisa da generalidade de Tiled: cada nível é uma tela fixa 640×360, o que elimina layers arbitrários, mapas de tamanho ilimitado e muitos modos de edição.

Fonte: [Tiled — GitHub](https://github.com/mapeditor/tiled) e [Editing Tile Layers](https://github.com/mapeditor/tiled/blob/master/docs/manual/editing-tile-layers.rst).

## 2. Godot — referência para ferramentas modais, viewport e snapping

O editor 2D do [Godot](https://github.com/godotengine/godot) oferece ferramentas distintas para selecionar, mover, escalar, fazer pan, medir e configurar snapping. A documentação também mostra uma abordagem importante: a interface apresenta as operações e respetivos atalhos no próprio editor.

### O que aproveitamos

- modos de ferramenta explícitos;
- atalhos curtos para ações frequentes;
- modificadores temporários para operações secundárias quando isso melhora a precisão;
- snapping configurável, com distinção entre grelha visual e passo de snap;
- seleção e transformação como conceitos separados;
- viewport com feedback visual suficiente para perceber origem, limites e escala;
- feedback de operações perto da zona de trabalho, em vez de obrigar o utilizador a consultar documentação externa.

### Adaptação ao ASCENDENDO

No Level Editor não adotamos pan livre nem uma world infinita, porque um `.lvl` é uma única tela. O princípio que aproveitamos é o de **ferramenta ativa + viewport previsível + feedback imediato**.

Fonte: [Godot 2D editor documentation](https://github.com/godotengine/godot-docs/blob/master/tutorials/2d/introduction_to_2d.rst).

## 3. SuperTux — referência para editor integrado no próprio jogo

O [SuperTux](https://github.com/SuperTux/supertux) tem um Level Editor integrado no projeto e documentação específica para criar níveis e worldmaps. O editor permite criar conteúdo, testar conceitos do jogo e depois partilhar os resultados.

A documentação mostra ainda uma distinção útil entre edição de tiles e edição de objetos, e entre níveis individuais e worldmap.

### O que aproveitamos

- o editor pertencer ao próprio produto final, em vez de exigir uma ferramenta externa;
- uma separação explícita entre editar uma unidade jogável e organizar o mapa/mundo que contém várias unidades;
- descoberta gradual através de ferramentas e tutoriais;
- partilha tratada como extensão natural do fluxo de criação.

### Relação com ASCENDENDO

Esta é a referência mais próxima para a ideia de **Level Editor + Campaign Editor**: o nosso Level Editor corresponde à unidade jogável, enquanto o Campaign Editor corresponde à organização visual das unidades.

Fontes: [SuperTux Level Editor](https://github.com/SuperTux/supertux/wiki/Level-Editor), [SuperTux Level Editor Guide](https://github.com/SuperTux/supertux/wiki/Level-Editor-Guide), [SuperTux User Manual](https://github.com/SuperTux/supertux/wiki/User-Manual).

## 4. Padrões comuns que vamos adotar

As três referências convergem em alguns princípios que fazem sentido para o ASCENDENDO:

1. **A ação principal deve ser visível.** O utilizador não deve depender de memorizar atalhos para descobrir como começar.
2. **O contexto deve ser explícito.** Um modo de ferramenta deve deixar claro o que um clique/drag fará.
3. **A transformação deve ser previsível.** Snap, limites e coordenadas não podem mudar silenciosamente.
4. **O feedback deve ser imediato.** Preview, seleção, validação e erro devem aparecer no próprio editor.
5. **O documento é separado da apresentação.** A UI pode mudar sem alterar o formato ou o modelo lógico do nível.
6. **Testar é parte da criação.** Um editor de jogo deve permitir verificar rapidamente se aquilo que foi colocado funciona no runtime.
7. **A complexidade do editor deve ser proporcional ao problema.** O ASCENDENDO deve adotar princípios testados, não toda a complexidade de ferramentas genéricas.

## 5. Decisões específicas para o ASCENDENDO

| Problema | Padrão de referência | Decisão ASCENDENDO |
|---|---|---|
| Área de trabalho | viewport previsível | 640×360 fixa no Level Editor |
| Ferramentas | modos explícitos | STAMP / DRAG e futuras ferramentas discretas |
| Snap | configurável/visível | grid visual 16 px + snap fino 4 px |
| Organização de níveis | worldmap/timeline | Campaign Editor vertical |
| Reordenação | manipulação direta | blocos arrastáveis + snap |
| Teste | feedback rápido | Playtest antes de guardar |
| Validação | feedback contextual | validação em memória + explicação visual |
| Atalhos | teclas curtas e visíveis | teclas acessíveis + painel Controlos |
| Layout | viewport adapta-se | UI/texto autoajustáveis dentro do viewport |
| Partilha | export/import | pacote declarativo + validação final no EXE |

## 6. Regra para referências futuras

Quando uma nova decisão de UX for necessária:

1. procurar pelo menos um projeto aberto semelhante;
2. identificar o padrão que existe independentemente da implementação;
3. decidir se esse padrão é adequado ao orçamento de complexidade/performance do ASCENDENDO;
4. registar a decisão em `docs/PRODUCT_DECISIONS.md`;
5. atualizar `docs/ROADMAP.md` se a decisão alterar o plano.

As referências são evidência de padrões de design, não uma obrigação de reproduzir uma ferramenta externa.
