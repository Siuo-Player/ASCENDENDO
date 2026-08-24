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
FLAG x y width height
```

Linhas vazias e linhas iniciadas por `#` são ignoradas.

### `PLATFORM`

É uma AABB sólida. O editor deve produzi-la alinhada ao `EDITOR_GRID_SNAP`.

### `FLAG`

É uma área de conclusão. Não é uma plataforma física independente; o final da campanha é detetado por overlap entre o player e `flagBounds`.

Regra de design: há **uma única FLAG por campanha** e ela pertence ao nível que estiver na última posição de `campaign.txt`. Isto evita colocar uma bandeira em cada tela e mantém a semântica de "fim da campanha" simples.

## `campaign.txt`

É uma lista ordenada de nomes de ficheiros `.lvl`, relativa a `Game/Assets/Levels/`:

```text
inicio.lvl
zigzag.lvl
precipicio.lvl
```

A ordem é significativa. É a ordem em que o motor faz streaming e também define qual nível é o último.

## Invariantes

Um nível novo deve:

- caber horizontalmente em `[0, 640]`;
- respeitar a altura local de uma página de `360` unidades;
- usar dimensões positivas;
- usar valores representáveis pelo grid do editor;
- não introduzir entidades desconhecidas no parser do motor;
- para a campanha, ser fisicamente alcançável segundo o validador.

O editor não deve oferecer uma posição impossível quando a restrição puder ser aplicada diretamente na UI. A pasta `Game/Assets/Levels/NaoValidados/` existe para níveis que estão em construção ou que falham a validação depois de guardados.

## Streaming

`Level::appendFromFile()` posiciona o conteúdo no `offsetY` atual e avança exatamente `LOGICAL_HEIGHT` por chunk. O conteúdo que ultrapassa uma página deve ser considerado inválido pelo editor/validador, não silenciosamente espalhado por vários chunks.

## Compatibilidade

O formato é texto simples de propósito: é legível, fácil de versionar e barato em armazenamento. O editor deve escrever o mesmo formato que o motor já lê; não deve criar um formato intermédio proprietário.