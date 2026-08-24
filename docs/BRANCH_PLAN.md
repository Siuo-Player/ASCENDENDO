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

### Ordem interna

1. Renderizar plataformas, spawn, FLAG e estado selecionado.
2. Converter cursor de janela para espaço lógico e depois para mundo/editor.
3. Hit-test das entidades.
4. STAMP por clique, usando preset MEDIUM por defeito.
5. DRAG para criação de retângulos.
6. Seleção e movimento de entidades existentes.
7. Apagar por botão direito e por `Delete`/`Backspace`.
8. Feedback visual de ferramenta, preset e entidade selecionada.
9. Manter todas as restrições já implementadas pelo modelo determinístico.

## Não entra nesta branch

- save/serialização;
- validação assíncrona;
- import/export de pacotes;
- biblioteca online/site;
- networking;
- campanha/playlist 9.6;
- sistema final de sprites do editor.

## Critérios de aceitação

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

## Próxima branch

Depois de integrar esta branch, criar uma nova branch dedicada a save/serialização + validação assíncrona (9.5). A documentação deverá então marcar a UI 9.4 como concluída e substituir este plano pelo plano de save/validation.