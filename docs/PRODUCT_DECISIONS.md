# Decisões de produto — ASCENDENDO

Este documento regista decisões funcionais tomadas durante o desenvolvimento. A implementação futura deve respeitar estas regras; quando uma decisão mudar, atualiza-se este documento e o roadmap na mesma tranche.

## 1. Objetivo do editor

O objetivo principal não é apenas fornecer uma ferramenta interna para criar os níveis iniciais. O **Level Editor é infraestrutura do produto**: depois da base do jogo estar pronta, a comunidade deve conseguir criar, testar, diagnosticar e partilhar mapas durante muitos anos sem depender da equipa original para corrigir manualmente cada mapa.

Por isso, confiabilidade, compatibilidade de formato, diagnósticos claros, playtest e validação são prioridades superiores a adicionar rapidamente muitos tipos de objetos.

O roadmap pode introduzir tipos de conteúdo mais ricos mais tarde, mas a arquitetura deve deixar espaço para os suportar sem quebrar níveis existentes.

## 2. Editores

### Level Editor = uma única tela

Um ficheiro `.lvl` representa exatamente uma tela lógica de `640x360`.

O **Level Editor** edita apenas essa tela. Não existe navegação/pan da tela no editor. O canvas completo deve estar visível e os limites físicos devem ser imediatamente perceptíveis.

A grelha visual serve a construção dos blocos e não deve transformar o canvas numa malha de linhas excessiva. O snap pode ser mais fino do que a grelha visual.

### Campaign Editor = linha do tempo vertical

O **Campaign Editor** é uma ferramenta diferente do Level Editor.

As telas são apresentadas como blocos/miniaturas compactados verticalmente, mantendo a proporção `16:9`. Aqui a câmara pode deslocar-se verticalmente.

Cada nível é um bloco arrastável. A movimentação pode usar snap para facilitar a troca de posição e a ordem visual representa a ordem da campanha.

### Entrada entre editores

É permitido abrir o Level Editor diretamente a partir do Campaign Editor, mas a mudança deve ocorrer através de uma mudança explícita de estado.

## 3. Teste antes de guardar

Um nível em construção deve poder ser testado antes de ser guardado definitivamente.

Fluxo esperado:

1. editar;
2. testar imediatamente com o Player;
3. observar a física e o percurso;
4. voltar ao editor sem perder o documento editado;
5. corrigir;
6. validar;
7. guardar.

O playtest não deve transformar alterações experimentais em estado persistido automaticamente.

Esta abordagem é coerente com editores integrados em jogos comunitários e com o workflow de criação/playtest usado pelo Jump King. citeturn687736search0turn687736search7

## 4. Validação ao vivo e diagnóstico

O editor deve apresentar o resultado da validação enquanto o nível está a ser construído.

A validação rápida deve funcionar em memória e não executar um processo Python a cada frame.

O feedback deve explicar a primeira causa útil de falha e servir como tutorial visual. O indicador de “inválido” sozinho não é suficiente.

No Campaign Editor, vários agentes/runs podem ser mostrados em background. Idealmente cada nível tem pelo menos um agente ativo e pelo menos um agente atravessa a fronteira entre níveis para mostrar a fluidez da campanha.

Esses agentes são ferramentas de visualização/debug e não substituem a validação final do EXE.

### Diagnóstico do percurso

Quando um agente falhar, a UI deve mostrar:

- o percurso tentado;
- o ponto aproximado onde a tentativa deixou de ser viável;
- uma explicação curta quando possível, por exemplo: **alvo demasiado longe**, **ângulo impossível**, **plataforma fora da janela alcançável**, **colisão lateral**, ou **sequência seguinte inacessível**.

No Campaign Editor, o diagnóstico deve tornar evidente se o problema é local ao nível ou surge na transição entre dois níveis.

## 5. Feedback visual do salto

A mecânica do salto continua a ser uma decisão de gameplay própria do ASCENDENDO; o Jump King é referência sobretudo para visual/composição e workflow de criação, não uma obrigação de copiar a sua física.

O carregamento do salto deve ser **visível**. O jogador não deve ter de inferir a força apenas pelo tempo em que manteve uma tecla pressionada.

A interface deve ter uma barra/indicador de força associada ao personagem que:

- mostra continuamente a carga;
- comunica claramente o estado mínimo/máximo;
- acompanha visualmente o momento em que a ação pode ser libertada;
- pode ter indicação de cancelamento/aborto quando isso melhorar acessibilidade ou evitar uma ação acidental.

A forma exata da animação da barra (preencher, voltar ao início, ciclos, etc.) fica como decisão de UX a testar; a exigência é a **visibilidade clara do estado de carga**.

## 6. Key bindings e descobribilidade

As teclas importantes devem ser descobríveis sem consultar documentação externa.

O menu principal terá uma área **Controlos / Key Bindings** para consultar as ações e respetivas teclas.

Não é requisito atual permitir rebind arbitrário; consultar e perceber as teclas é obrigatório.

Todos os menus devem mostrar no rodapé as ações essenciais do estado atual, pelo menos navegar, confirmar e voltar/sair.

### Teclas acessíveis

Não devemos depender de F2/F5/F6 para funcionalidades importantes do editor.

Defaults previstos:

- `1` — guardar;
- `2` — testar;
- `3` — validar;
- `E` — editor de nível;
- `C` — editor de campanha;
- `G` — alternar STAMP/DRAG;
- `[` / `]` — diminuir/aumentar preset;
- `Delete` / `Backspace` — apagar seleção;
- `Esc` — voltar/sair;
- `Q` — sair/abandonar quando aplicável;
- `0` — área de consulta de Controlos.

Os atalhos são aceleradores. As ações essenciais devem continuar acessíveis visualmente.

## 7. Layout e viewport

O jogo mantém o espaço lógico `640x360` e o letterboxing para preservar o rácio.

**Nenhum texto ou componente de UI pode depender de coordenadas fixas que façam o conteúdo desaparecer fora da área visível.**

Usaremos layouts autoajustáveis para:

- texto que possa ter comprimentos diferentes;
- menus com diferentes números de opções;
- rodapés/contextual help;
- HUD do editor;
- painéis de controlos;
- seleção de campanhas.

O fullscreen deve ser preservado. O tamanho da janela física pode mudar, mas o espaço lógico não é deformado nem cortado.

O Level Editor deve conseguir mostrar a tela `640x360` completa dentro do viewport disponível. O Campaign Editor pode ocupar uma área vertical maior e usar scroll nessa área.

## 8. Seleção de campanha

`Começar` não inicia diretamente uma campanha por acidente.

O fluxo é:

`MENU → Seleção de campanha → campanha selecionada → PLAYING`

Mesmo que exista apenas uma campanha, a seleção deve existir como conceito e UI. Isso evita ter de alterar o fluxo quando forem adicionadas outras campanhas.

A seleção deve apresentar informação suficiente para escolher conscientemente, incluindo pelo menos:

- nome;
- preview/miniatura quando existir;
- número de níveis;
- estado de validade;
- ação iniciar;
- voltar.

## 9. Conteúdo futuro e compatibilidade

Os primeiros tipos de conteúdo podem permanecer simples, mas o modelo de nível deve ser extensível e versionável.

Categorias futuras previstas, sem obrigação de implementação imediata:

- superfícies e variantes de plataforma;
- perigos;
- elementos móveis;
- objetos interativos;
- triggers/zonas;
- decoração;
- metadados de identidade visual;
- regras e eventos específicos da campanha.

A introdução destes tipos deve ser aditiva e compatível com níveis antigos sempre que possível.

## 10. Princípios de design adotados de projetos abertos

As referências detalhadas encontram-se em `docs/DESIGN_REFERENCES.md`.

Adotamos destes projetos sobretudo:

- ferramentas explícitas e separadas por modo;
- snapping visível e previsível;
- viewport com limites/escala claros;
- feedback imediato;
- teste integrado no processo de criação;
- distinção entre unidade jogável e organização de múltiplas unidades;
- comunidade e distribuição como parte do produto, não como afterthought.

Não copiamos a generalidade nem a complexidade de editores como Tiled/Godot, porque o problema do ASCENDENDO é intencionalmente menor e mais restrito.

## 11. Portabilidade

O objetivo final continua a ser um executável Windows x64 portátil, acompanhado dos recursos necessários, sem depender do ambiente de desenvolvimento.

## 12. Mapas importados/partilhados

Qualquer mapa obtido por importação, download ou partilha deve ser validado novamente pelo próprio EXE antes de poder ser jogado.

O servidor nunca é a autoridade final de validade do mapa.
