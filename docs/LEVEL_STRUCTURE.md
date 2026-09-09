# Estrutura espacial de levels — ASCENDENDO 1.0

## 1. Contrato fixo

A largura é uma constante absoluta de 1.0:

```text
640 px
```

Nunca existe largura configurável de level ou screen em 1.0.

Uma screen lógica é:

```text
640 × 360 px
```

Um level é uma sequência vertical de `N` screens:

```text
height = N × 360 px
width  = 640 px
```

`N` não é fixo. Exemplos válidos incluem 1, 3, 5 e 10; o formato deve aceitar valores maiores.

## 2. Porque existe o nível multi-screen

As regras de 1.0 são intencionalmente poucas: plataformas estáticas, spawn/jogador, FLAG, movimento horizontal, salto comprometido e ausência de air control. Uma única screen tem, por isso, um espaço de combinações limitado e muitas telas começam a ficar visualmente semelhantes.

O level multi-screen aumenta a variedade e a escala sem aumentar a largura nem adicionar mecânicas. Cada screen continua legível e utilizável como unidade local de análise; o level ganha contexto, sequência e progressão.

## 3. Direcção de progressão

A rota do level é sempre ascendente.

```text
SCREEN 0
   ↑
SCREEN 1
   ↑
SCREEN 2
   ↑
...
SCREEN N-1
```

O jogador pode cair para uma screen inferior. Essa queda representa regressão física; não existe uma rota de progressão deliberadamente descendente em 1.0.

A FLAG de um level final deve ficar na última screen.

## 4. Validação

O desenho permite separar validação local e validação de composição:

```text
screen válida
  + transição possível
  + screen seguinte válida
  = progressão de level válida
```

Isto permite diagnósticos do tipo `level 27 / screen 4 / transição 4→5`, em vez de apenas `level inválido`.

A validação pode analisar cada screen isoladamente quando a pergunta é local e depois analisar estados de saída/entrada entre screens quando a pergunta é de progressão. O modelo ascendente reduz o espaço de navegação arbitrária.

## 5. Runtime e streaming

A apresentação deve ser contínua. A fronteira entre screens não deve produzir um teletransporte.

O runtime deve manter a zona actual e zonas vizinhas necessárias, pré-carregar a próxima zona de subida e manter zonas anteriores facilmente recuperáveis durante quedas. O cache pode manter mais contexto quando isso reduzir jitter, desde que não altere a simulação.

Streaming é infraestrutura de carregamento; não altera a ordem lógica das screens nem a autoridade do replay.

## 6. Editor

O Level Editor edita o level vertical inteiro, com navegação vertical.

A screen `640×360` continua a ser a unidade de leitura e composição visual. O editor deve permitir saltar rapidamente entre screens e navegar verticalmente por teclado.

Dois modos são equivalentes do ponto de vista de capacidade:

```text
teclado apenas
teclado + rato
```

Mouse/drag acelera operações, mas nenhuma operação essencial pode depender dele.

## 7. Formato `.lvl`

O `.lvl` mantém-se como unidade técnica de um level.

Para compatibilidade, ausência de `SCREENS` implica `SCREENS 1`.

A forma canónica para novos ficheiros inclui:

```text
NAME <nome>
SCREENS <N>
SPAWN <x> <y>
PLATFORM <x> <y> <w> <h>
...
FLAG <x> <y> <w> <h>
```

O campo `SCREENS` descreve a dimensão vertical lógica; não altera a largura.

## 8. Relação com campanhas

A hierarquia deliberada de 1.0 é:

```text
screen = unidade espacial 640×360
level  = unidade jogável vertical de N screens
campaign = sequência de levels
```

A campanha continua a ser a unidade pública principal de publicação/import/export. `.lvl` permanece a unidade técnica de storage e transporte do level.

## 9. Futuro

A generalização do modelo deve permitir posteriormente objectos letais, colectáveis obrigatórios, checkpoints, moving platforms, triggers e outros sistemas sem exigir que 1.0 os use.

Possíveis jogos derivados podem ainda colocar levels horizontalmente, mas isso não faz parte do contrato de ASCENDENDO 1.0 e não deve contaminar a API actual.
