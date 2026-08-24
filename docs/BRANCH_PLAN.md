# Plano da branch atual

**Branch:** `feat/9-4-editor-ui-v2`

**Base:** `main` após integração das PRs #4 (`38406c5`) e #5 (`1971da2`).

## Estado herdado — concluído

- Fases 1–8.
- 9.1–9.3.
- bootstrap GLFW/Vulkan e cleanup de falhas parciais.
- documentação técnica inicial.
- 9.4 tranche 1: modelo determinístico do editor, grid/snap, plataformas, spawn, FLAG e testes.
- requisito de produto: EXE portable Windows x64, offline-first, import/export e futura biblioteca online com validação local obrigatória.

O plano da branch anterior de documentação de release/partilha fica **concluído**.

## Objetivo desta branch

Ligar o `LevelEditorDocument` testado à UI real do `GameState::EDITOR`.

### Implementado até agora nesta branch

- núcleo `EditorInteractionController` separado de GLFW/Vulkan;
- conversão determinística `logical cursor -> world` com o offset da `Camera`;
- hit-test das plataformas com prioridade para a entidade mais recente;
- STAMP com preset MEDIUM por defeito e suporte a SMALL/MEDIUM/LARGE;
- DRAG com normalização dos cantos e delegação de snap/validação ao documento;
- seleção e movimento preservando o offset do cursor;
- apagar por alvo espacial, com correção do índice selecionado;
- testes unitários do controller sem GPU.

## Ordem interna restante

1. Renderizar plataformas, spawn, FLAG e estado selecionado.
2. Ligar eventos reais do `InputManager` ao controller.
3. STAMP por clique, usando preset MEDIUM por defeito.
4. DRAG para criação de retângulos com preview.
5. Seleção e movimento de entidades existentes.
6. Apagar por botão direito e por `Delete`/`Backspace`.
7. Feedback visual de ferramenta, preset e entidade selecionada.
8. Manter todas as restrições já implementadas pelo modelo determinístico.

## Não entra nesta branch

- save/serialização;
- validação assíncrona;
- import/export de pacotes;
- biblioteca online/site;
- networking;
- campanha/playlist 9.6;
- sistema final de sprites do editor.

## Critérios de aceitação da branch completa

- o nível em edição é visível dentro do editor;
- clique em vazio cria no grid quando em STAMP;
- drag cria uma área quantizada quando em DRAG;
- clicar numa entidade seleciona-a em vez de duplicá-la;
- arrastar entidade move-a dentro dos bounds;
- botão direito apaga a entidade alvo;
- `Delete` e `Backspace` fazem a mesma ação;
- spawn e FLAG respeitam as invariantes da camada lógica;
- não aparecem posições inválidas como resultado de uma operação aceite;
- não há dependência de Vulkan nos testes da lógica.

## Próxima branch após esta

Depois de esta tranche/UI estar concluída e integrada, criar uma nova branch para **9.5 — save/serialização + validação assíncrona**. A documentação deverá marcar 9.4 como concluída e substituir este plano pelo plano de save/validation.