# Formato de níveis

## Coordenadas

O espaço lógico é `640 × 360`.

- origem: canto inferior esquerdo;
- X cresce para a direita;
- Y cresce para cima;
- cada ficheiro `.lvl` descreve uma página vertical de `360` unidades;
- ao fazer streaming, o motor soma `offsetY` ao Y local.

Por isso uma plataforma `PLATFORM 100 40 80 20` no segundo chunk é colocada no mundo em Y `400` quando `offsetY = 360`.

## Sintaxe

```text
NAME Nome humano-legível
PLATFORM x y width height
SPAWN x y
```

Linhas vazias e linhas iniciadas por `#` são ignoradas.

### `PLATFORM`

É uma AABB sólida. O editor deve produzi-la alinhada ao `EDITOR_GRID_SNAP`.

### `SPAWN`

É a posição inicial do jogador no nível, quando explicitamente definida. A posição só pertence ao primeiro chunk efetivamente carregado numa run; o streaming mantém o spawn inicial.

### Objetivo final da campanha

`FLAG` **não faz parte do formato `.lvl`** e não pode ser authored pelo editor nem persistido em ficheiro.

O objectivo é derivado pelo runtime a partir da **plataforma mais alta do último nível da campanha**. A área de conclusão é colocada automaticamente imediatamente acima dessa plataforma, usando a mesma largura e uma altura fixa de `40` unidades.

Consequentemente, alterar a ordem dos níveis em `campaign.txt` altera automaticamente qual nível recebe o objectivo final. Adicionar, remover ou mover a plataforma mais alta do último nível também altera automaticamente a posição do objectivo. Não existe uma `FLAG` escondida num ficheiro que possa ficar dessincronizada.

## `campaign.txt`

É uma lista ordenada de nomes de ficheiros `.lvl`, relativa a `Game/Assets/Levels/`:

```text
inicio.lvl
zigzag.lvl
precipicio.lvl
```

A ordem é significativa. É a ordem em que o motor faz streaming e também define qual nível é o último e, portanto, qual nível recebe automaticamente o objectivo final.

## Invariantes

Um nível novo deve:

- caber horizontalmente em `[0, 640]`;
- respeitar a altura local de uma página de `360` unidades;
- usar dimensões positivas;
- usar valores representáveis pelo grid do editor;
- não introduzir entidades desconhecidas no parser do motor;
- não conter `FLAG`;
- para a campanha, ser fisicamente alcançável segundo o validador.

O editor não deve oferecer uma posição impossível quando a restrição puder ser aplicada diretamente na UI. A pasta `Game/Assets/Levels/NaoValidados/` existe para níveis que estão em construção ou que falham a validação depois de guardados.

## Streaming

`Level::appendFromFile()` posiciona o conteúdo no `offsetY` atual e avança exatamente `LOGICAL_HEIGHT` por chunk. O conteúdo que ultrapassa uma página deve ser considerado inválido pelo editor/validador, não silenciosamente espalhado por vários chunks.

## Compatibilidade

O formato é texto simples de propósito: é legível, fácil de versionar e barato em armazenamento. O editor deve escrever o mesmo formato que o motor já lê; não deve criar um formato intermédio proprietário.