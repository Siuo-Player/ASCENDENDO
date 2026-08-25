# Decisões de produto — ASCENDENDO

Este documento regista decisões funcionais tomadas durante o desenvolvimento. Estas decisões devem ser refletidas no roadmap e na implementação futura.

## Editores

### Level Editor = uma única tela

Um ficheiro `.lvl` representa exatamente uma tela lógica de `640x360`.

O **Level Editor** edita apenas essa tela. Não existe navegação/pan vertical do nível. O canvas inteiro deve estar visível e os limites físicos da tela devem ser imediatamente perceptíveis.

A grelha visual deve servir a construção dos blocos e não simplesmente desenhar linhas em excesso. O snap de edição pode ser mais fino do que a grelha visual.

### Campaign Editor = linha do tempo vertical

O **Campaign Editor** é uma ferramenta diferente do Level Editor.

As telas são apresentadas como blocos/miniaturas compactados verticalmente, com proporção `16:9`. A câmara pode deslocar-se verticalmente neste editor.

Cada nível é um bloco arrastável. A movimentação pode usar snap para facilitar a troca de posição na campanha.

A ordem visual dos blocos representa a ordem da campanha.

### Entrada entre editores

É permitido abrir o Level Editor diretamente a partir do Campaign Editor, mas a mudança deve ocorrer através de uma mudança explícita de estado.

## Teste antes de guardar

Um nível em construção deve poder ser testado antes de ser guardado definitivamente.

O objetivo é permitir:

1. editar;
2. testar imediatamente com o Player;
3. observar a física e o percurso;
4. voltar ao editor;
5. só depois guardar/validar.

## Validação ao vivo

O editor deve apresentar o resultado do validador enquanto o nível está a ser construído.

A validação deve ser suficientemente rápida para funcionar em memória, sem executar um processo Python a cada frame.

Além do estado final válido/inválido, a interface deve servir como tutorial visual do que falta corrigir.

## Simulação da campanha no editor

O Campaign Editor deve conseguir mostrar agentes/runs de validação em background sobre as miniaturas da campanha.

Devem existir vários agentes suficientes para que, idealmente, cada nível tenha pelo menos um agente ativo. Deve existir também pelo menos um agente capaz de mostrar a transição entre níveis, permitindo observar a fluidez da campanha.

Esses agentes são uma visualização do processo de validação/debug e não substituem a validação final do EXE.

## Key bindings

As teclas importantes devem ser descobríveis sem consultar documentação externa.

O menu principal deve conter uma área/opção **Controlos / Key Bindings** onde o jogador possa consultar as ações e respetivas teclas.

Não é requisito atual permitir rebind arbitrário no jogo; consultar e perceber as teclas é obrigatório.

Todos os menus devem mostrar no rodapé pelo menos as ações simples necessárias naquele estado, especialmente:

- navegar;
- confirmar;
- voltar/sair.

## Portabilidade

O objetivo de distribuição continua a ser um executável Windows x64 portátil, acompanhado dos recursos necessários, sem depender do ambiente de desenvolvimento.

## Mapas importados/partilhados

Qualquer mapa obtido por importação, download ou partilha deve ser validado novamente pelo próprio EXE antes de poder ser jogado.

O servidor nunca é a autoridade final de validade do mapa.
