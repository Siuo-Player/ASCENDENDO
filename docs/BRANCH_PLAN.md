# Plano da branch atual

**Branch:** `feat/9-4-editor-visual-integration`

**Base:** `main` após integração da PR #7 (`10cceeb`).

## Estado herdado — concluído

- Fases 1–8.
- 9.1–9.3.
- bootstrap GLFW/Vulkan e cleanup de falhas parciais.
- documentação técnica inicial.
- 9.4 tranche 1: modelo determinístico do editor, grid/snap, plataformas, spawn, FLAG e testes.
- 9.4 tranche 2: `EditorInteractionController`, cursor logical→world, hit-test, STAMP/DRAG, seleção, movimento e delete com testes sem GPU.
- 9.4 tranche 3: robustez de validação, Makefile Windows/Linux, CI headless Vulkan determinístico, validação de campanha e documentação de CI.
- requisito de produto: EXE portable Windows x64, offline-first, import/export e futura biblioteca online com validação local obrigatória.

O plano da branch anterior (`feat/9-4-editor-ui-integration`) fica **concluído**.

## Objetivo desta branch

Ligar o modelo determinístico do editor à UI/renderização real do jogo, mantendo a interação baseada em grid e estados claros, sem duplicar a lógica de edição entre input e renderer.

## Plano desta branch

1. Integrar `LevelEditorDocument` e `EditorInteractionController` no estado `EDITOR` real.
2. Desenhar plataformas/editáveis existentes usando o mesmo espaço lógico do jogo.
3. Implementar cursor de editor e preview do preset ativo.
4. Implementar feedback visual de STAMP, DRAG, seleção e movimento.
5. Integrar mouse e key bindings sem criar conflitos com o estado normal do jogo.
6. Garantir que cliques fora da grid/canvas válida são ignorados ou recusados.
7. Adicionar testes de integração para input→controller→document e regressões de viewport.
8. Validar manualmente a experiência com o jogo real antes do merge.
9. Atualizar documentação e abrir PR.

## Decisões de UX herdadas

- O editor abre no tamanho médio por defeito.
- Presets devem privilegiar tamanhos familiares de jogos de plataforma, restringidos pelo grid.
- Teclado e rato podem alternar entre ferramentas/estados; os bindings devem ser explorados de forma consistente.
- Elementos fora da grid/canvas não são oferecidos como opção editável.
- O FLAG continua reservado ao último nível da campanha.
- O editor é uma mudança de estado explícita, inclusive ao navegar diretamente do editor de campanha para o editor de nível.

## Requisitos de engenharia

- Não introduzir dependência de networking.
- Manter o núcleo leve e otimizado.
- Evitar duplicação de sprites/dados sempre que uma representação procedural ou atlas compacto for suficiente.
- O jogo continua offline-first.
- A futura importação/partilha de mapas deve validar novamente o conteúdo no EXE antes de permitir jogar.

## Não entra nesta branch

- save/serialização final de mapas;
- import/export de pacotes;
- biblioteca online/site;
- networking/WebSockets;
- campanha/playlist 9.6;
- sistema final de sprites/atlas do editor;
- empacotamento final do EXE release.

## Critério de conclusão

A tranche só é considerada concluída quando for possível entrar no editor de nível real e, com mouse/teclado, selecionar, criar, mover e apagar elementos com feedback visual coerente, sem quebrar o estado de jogo normal, e quando os testes automatizados e uma validação manual do fluxo estiverem verdes.
