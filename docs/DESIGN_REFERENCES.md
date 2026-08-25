# Referências de design — editores e jogos abertos

Este documento regista referências externas usadas para orientar o desenho do ASCENDENDO. Não são especificações para copiar literalmente: servem para identificar padrões de interface, workflow e composição de níveis que já foram exercitados em projetos reais.

## 1. Tiled — referência para edição espacial e snapping

[Tiled](https://github.com/mapeditor/tiled) é um editor de mapas open source usado também em jogos de plataformas.

### O que aproveitamos

- separar claramente o documento de nível da camada de apresentação;
- tratar snapping como ferramenta de criação, não como detalhe escondido do motor;
- permitir que o editor mostre a estrutura do conteúdo de forma previsível;
- manter o formato do mapa declarativo e legível;
- pensar desde cedo em compatibilidade e evolução do formato.

### O que não copiamos

O ASCENDENDO não precisa da generalidade de Tiled: cada nível é uma tela fixa 640×360, o que elimina layers arbitrários, mapas de tamanho ilimitado e muitos modos de edição.

Fonte: [Tiled — GitHub](https://github.com/mapeditor/tiled).

## 2. Godot — referência para ferramentas modais, viewport e snapping

O editor 2D do [Godot](https://github.com/godotengine/godot) oferece ferramentas distintas para selecionar, mover, escalar, fazer pan, medir e configurar snapping.

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

## 3. SuperTux — referência para editor integrado e comunidade

O [SuperTux](https://github.com/SuperTux/supertux) possui um Level Editor integrado e documentação específica para criação de níveis/worldmaps, incluindo fluxo de partilha como add-ons. A documentação também separa ferramentas de tiles e objetos e distingue níveis individuais do worldmap. citeturn687736search0turn687736search1turn687736search6

### O que aproveitamos

- o editor pertencer ao próprio produto final;
- separação explícita entre editar uma unidade jogável e organizar o conjunto que a contém;
- descoberta gradual através de ferramentas e tutoriais;
- partilha como extensão natural do fluxo de criação;
- formatos de nível versionáveis, porque o formato do SuperTux evoluiu explicitamente entre versões. citeturn687736search4

### Relação com ASCENDENDO

Esta é a referência mais próxima para a ideia de **Level Editor + Campaign Editor** e para a meta de uma comunidade capaz de criar e partilhar conteúdo sem precisar de um editor externo. citeturn687736search0turn687736search8

## 4. Jump King — referência visual, composição e workflow de construção

[Jump King] não é uma referência obrigatória para a física do ASCENDENDO. A inspiração aqui é sobretudo **visual e editorial**: salas com leitura imediata, composição vertical, uso deliberado do espaço, identidade visual forte por zona e construção de sequências em que o posicionamento das plataformas conta para a percepção de risco e progressão.

Existe ainda uma referência particularmente relevante no ecossistema atual do jogo: o workflow oficial de criação passou a incluir **ferramentas de edição de níveis, teste dentro do jogo com um clique, atualização do nível enquanto se faz playtest e pré-visualizações de níveis/skins**. Isso reforça diretamente a nossa decisão de tratar *editar → testar → corrigir → validar* como um ciclo central do editor. citeturn687736search7

Há também documentação comunitária do formato de níveis do Jump King que representa a estrutura por ecrãs e usa um grid auxiliar para manter a edição dentro dos limites de cada screen, uma ideia muito próxima da regra do ASCENDENDO de que cada `.lvl` é uma tela fixa. citeturn687736search2

### O que aproveitamos

- composição visual por tela/zona;
- leitura forte de plataformas e rotas;
- verticalidade como linguagem visual, não apenas como coordenada física;
- identidade visual por conjuntos de telas;
- preview do nível antes de o jogar;
- playtest imediato a partir do editor;
- possibilidade de atualizar/recarregar o conteúdo durante o teste;
- ferramentas que reduzem a distância entre criação e resultado jogável.

### O que não copiamos

- não assumimos que a física de salto do Jump King define a física do ASCENDENDO;
- não copiamos o seu formato de níveis;
- não importamos necessariamente a mesma quantidade de ecrãs ou escala;
- não queremos transformar o editor num clone do workflow do Jump King.

## 5. Padrões comuns que vamos adotar

As referências convergem em alguns princípios que fazem sentido para o ASCENDENDO:

1. **A ação principal deve ser visível.** O utilizador não deve depender de memorizar atalhos para descobrir como começar.
2. **O contexto deve ser explícito.** Um modo de ferramenta deve deixar claro o que um clique/drag fará.
3. **A transformação deve ser previsível.** Snap, limites e coordenadas não podem mudar silenciosamente.
4. **O feedback deve ser imediato.** Preview, seleção, validação e erro devem aparecer no próprio editor.
5. **Testar é parte da criação.** Um editor de jogo deve permitir verificar rapidamente se aquilo que foi colocado funciona no runtime. O workflow do Jump King reforça diretamente este princípio. citeturn687736search7
6. **O documento é separado da apresentação.** A UI pode mudar sem alterar o formato ou o modelo lógico do nível.
7. **A comunidade é uma parte do produto.** Se os mapas serão criados por terceiros, o editor, o formato, a validação e as ferramentas de diagnóstico devem ser tratados como infraestrutura do jogo e não como ferramentas descartáveis.
8. **A complexidade do editor deve ser proporcional ao problema.** O ASCENDENDO deve adotar princípios testados, não toda a complexidade de ferramentas genéricas.

## 6. Arquitetura preparada para conteúdo futuro

Mesmo antes de introduzirmos objetos mais ricos, a base deve reservar espaço para evoluir sem quebrar mapas antigos.

As futuras categorias devem ser tratadas como dados declarativos e versionáveis, por exemplo:

- plataformas e superfícies;
- pontos de spawn/objetivos;
- elementos decorativos;
- perigos;
- objetos interativos;
- elementos móveis;
- triggers/zonas;
- metadados de layout e identidade visual;
- regras específicas de campanha.

A implementação concreta destas categorias fica para fases posteriores do roadmap. O requisito atual é não acoplar o modelo de nível a uma lista fixa de `PLATFORM` + `FLAG` de forma que uma futura expansão obrigue a reescrever o formato inteiro.

## 7. Decisões específicas para o ASCENDENDO

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
| Visual | composição por ecrãs/zonas | inspiração visual de Jump King, sem copiar a física |
| Partilha | export/import | pacote declarativo + validação final no EXE |

## 8. Regra para referências futuras

Quando uma nova decisão de UX, level design ou tooling for necessária:

1. procurar pelo menos um projeto aberto semelhante;
2. identificar o padrão que existe independentemente da implementação;
3. decidir se esse padrão é adequado ao orçamento de complexidade/performance do ASCENDENDO;
4. registar a decisão em `docs/PRODUCT_DECISIONS.md`;
5. atualizar `docs/ROADMAP.md` se a decisão alterar o plano.

As referências são evidência de padrões de design, não uma obrigação de reproduzir uma ferramenta externa.