# Decisões de produto — ASCENDENDO

Este documento regista decisões funcionais tomadas durante o desenvolvimento. A implementação futura deve respeitar estas regras; quando uma decisão mudar, atualiza-se este documento e o roadmap na mesma tranche.

## 1. Editores

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

## 2. Teste antes de guardar

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

## 3. Validação ao vivo

O editor deve apresentar o resultado da validação enquanto o nível está a ser construído.

A validação rápida deve funcionar em memória e não executar um processo Python a cada frame.

O feedback deve explicar a primeira causa útil de falha e servir como tutorial visual. O indicador de “inválido” sozinho não é suficiente.

No Campaign Editor, vários agentes/runs podem ser mostrados em background. Idealmente cada nível tem pelo menos um agente ativo e pelo menos um agente atravessa a fronteira entre níveis para mostrar a fluidez da campanha.

Esses agentes são ferramentas de visualização/debug e não substituem a validação final do EXE.

## 4. Key bindings e descobribilidade

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

## 5. Layout e viewport

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

## 6. Seleção de campanha

`Começar` não inicia diretamente uma campanha por acidente.

O fluxo é:

`MENU → Seleção de campanha → campanha selecionada → PLAYING`

Mesmo que exista apenas uma campanha, a seleção deve existir como conceito e UI. Isso evita ter de alterar o fluxo quando forem adicionadas outras campanhas.

## 7. Princípios de design adotados de projetos abertos

As referências detalhadas encontram-se em `docs/DESIGN_REFERENCES.md`.

Adotamos destes projetos sobretudo:

- ferramentas explícitas e separadas por modo;
- snapping visível e previsível;
- viewport com limites/escala claros;
- feedback imediato;
- teste integrado no processo de criação;
- distinção entre unidade jogável e organização de múltiplas unidades.

Não copiamos a generalidade nem a complexidade de editores como Tiled/Godot, porque o problema do ASCENDENDO é intencionalmente menor e mais restrito.

## 8. Portabilidade

O objetivo final continua a ser um executável Windows x64 portátil, acompanhado dos recursos necessários, sem depender do ambiente de desenvolvimento.

## 9. Mapas importados/partilhados

Qualquer mapa obtido por importação, download ou partilha deve ser validado novamente pelo próprio EXE antes de poder ser jogado.

O servidor nunca é a autoridade final de validade do mapa.
