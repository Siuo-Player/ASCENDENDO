# Campaign Editor — modelo de interação

## Regra fundamental

Um `.lvl` representa exatamente uma tela lógica de `640x360`. O **Level Editor** nunca permite deslocar essa tela no eixo Y nem explorar uma world infinita.

A deslocação vertical existe apenas no **Campaign Editor**, onde os níveis são representados como miniaturas proporcionais e empilhados verticalmente, à semelhança de uma timeline de vídeo.

## Level Editor

- Canvas fixo: `640x360`.
- Limites do canvas sempre visíveis.
- Snap lógico: 4 px.
- Grelha visual principal: 16 px.
- Grelha major: 64 px.
- Plataformas e objetivos são editados dentro do canvas.
- F2: guardar.
- F5: executar playtest com o jogador antes de guardar/validar.
- F6: validar o nível em memória.
- C: abrir Campaign Editor.

## Campaign Editor

Cada nível é um bloco arrastável. O bloco usa uma miniatura 16:9 (`192x108`) para conservar a proporção visual da tela real.

A ordem da campanha continua explícita no índice do bloco. Ao arrastar um bloco para a posição de outro, a ordem faz snap e os níveis trocam de posição.

O Campaign Editor pode:

- deslocar verticalmente a timeline;
- selecionar um nível;
- arrastar/reordenar níveis;
- abrir o Level Editor do nível selecionado;
- mostrar o percurso dos validadores em todos os níveis em simultâneo;
- evidenciar transições entre o final de um nível e o início do seguinte.

## Validação ao vivo

O Level Editor deve validar o documento em memória sempre que a geometria muda. O resultado deve ser apresentado como feedback visual imediato, não como uma execução externa de Python por frame.

No Campaign Editor haverá pelo menos uma simulação de validação por nível em simultâneo. Quando um agente chega ao final de um nível, a sua representação pode continuar para o próximo nível para tornar visível a continuidade da campanha.

## Arquitetura

```text
LevelEditorDocument
    ↓
EditorSession
    ↓
EditorRenderSnapshot
    ↓
EditorRenderer

CampaignEditorDocument
    ↓
CampaignEditorRenderSnapshot
    ↓
CampaignEditorRenderer
```

Os dois modelos não partilham uma câmara. O Level Editor é fixo; a timeline da campanha é deslocável.
