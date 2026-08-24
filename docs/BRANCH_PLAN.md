# Plano da branch atual

**Branch:** `feat/9-4-editor-ui-integration`

**Base:** `main` após integração da PR #6 (`55e4a0a`).

## Estado herdado — concluído

- Fases 1–8.
- 9.1–9.3.
- bootstrap GLFW/Vulkan e cleanup de falhas parciais.
- documentação técnica inicial.
- 9.4 tranche 1: modelo determinístico do editor, grid/snap, plataformas, spawn, FLAG e testes.
- 9.4 tranche 2: `EditorInteractionController`, cursor logical→world, hit-test, STAMP/DRAG, seleção, movimento e delete com testes sem GPU.
- requisito de produto: EXE portable Windows x64, offline-first, import/export e futura biblioteca online com validação local obrigatória.

O plano da branch anterior (`feat/9-4-editor-ui-v2`) fica **concluído**.

## Objetivo desta branch

Integrar a lógica do editor no runtime e, antes disso, garantir que a infraestrutura de build/testes funciona de forma consistente no Windows e no CI.

## Implementado até agora nesta branch

- correção da regra de dimensão mínima de plataformas: dimensão inválida é rejeitada antes do snap;
- Makefile com comandos de criação/remoção de diretórios adequados ao Windows e Linux;
- targets `tests`, `tests-fast` e `tests-verbose` sem depender de `./`/`cat` no Windows;
- separação dos comandos de execução/leitura/erro por plataforma;
- GitHub Actions para compilar e executar os testes em Ubuntu;
- GitHub Actions para validar toda a campanha ativa com `ai_validator.py --campaign`.

## Resultado da validação local conhecida

A primeira execução local encontrou corretamente o bug da plataforma mínima (`3x2` aceite indevidamente). Depois da correção, a execução seguinte ainda não passou porque o Makefile antigo usava comandos POSIX (`./`, `cat`, `mkdir -p`, `rm -rf`) sob o shell Windows. Esta branch substitui essas receitas por variantes conscientes da plataforma.

A validação final dos testes desta correção deve ser feita pelo GitHub Actions e, opcionalmente, por uma execução local Windows.

## Ordem interna restante

1. Confirmar CI verde para build + testes + validação da campanha.
2. Integrar `LevelEditorDocument` ao `GameState::EDITOR`.
3. Renderizar plataformas, spawn, FLAG e estado selecionado.
4. Ligar eventos reais do `InputManager` ao controller.
5. STAMP por clique, preset MEDIUM por defeito.
6. DRAG com preview quantizado.
7. Seleção e movimento.
8. Apagar por botão direito e `Delete`/`Backspace`.
9. Feedback visual de ferramenta, preset e seleção.
10. Atualizar documentação da 9.4 e preparar merge.

## Não entra nesta branch

- save/serialização;
- validação assíncrona do editor;
- import/export de pacotes;
- biblioteca online/site;
- networking;
- campanha/playlist 9.6;
- sistema final de sprites do editor.

## Critérios de aceitação da branch completa

- CI verde nos testes;
- o nível em edição é visível dentro do editor;
- clique em vazio cria no grid quando em STAMP;
- drag cria uma área quantizada quando em DRAG;
- clicar numa entidade seleciona-a em vez de duplicá-la;
- arrastar entidade move-a dentro dos bounds;
- botão direito apaga a entidade alvo;
- `Delete` e `Backspace` fazem a mesma ação;
- spawn e FLAG respeitam as invariantes da camada lógica;
- não aparecem posições inválidas como resultado de uma operação aceite;
- testes da lógica permanecem independentes de Vulkan.

## Próxima branch após esta

Depois de a integração da UI 9.4 estar concluída e integrada, criar uma nova branch para **9.5 — save/serialização + validação assíncrona**. A documentação deverá marcar 9.4 como concluída e substituir este plano pelo plano de save/validation.