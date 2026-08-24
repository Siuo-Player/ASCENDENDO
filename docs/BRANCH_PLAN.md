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

## Implementado até agora

- `EditorSession` para orquestrar `InputManager`, `Camera`, `KeyBindings` e `EditorInteractionController`.
- `G` alterna entre `STAMP` e `DRAG` como ferramenta persistente.
- `[` / `]` alteram `SMALL` / `MEDIUM` / `LARGE`, começando em `MEDIUM`.
- Clique esquerdo cria ou move; clique direito cancela; `Delete`/`Backspace` apaga a seleção.
- Preview determinístico (`EditorPreview`) separado da renderização, incluindo recusa fora do canvas lógico.
- `EditorRenderSnapshot` separa os dados de apresentação do documento de edição; o renderer não deve conhecer `LevelEditorDocument`.
- Integração do ciclo `EDITOR` no `main.cpp`, sem executar física enquanto o editor está ativo.
- Testes unitários de input → controller → documento, geometria do preview e conteúdo do snapshot.
- `docs/ARCHITECTURE.md` atualizado com a direção arquitetural alvo.
- `docs/TECH_DEBT.md` criado com dívida técnica classificada e critérios de saída.
- `docs/ROADMAP.md` revisto para introduzir o Gate 9.4.5 antes de Save/Import.

## Pendente nesta tranche

1. Fazer `Renderer` consumir `EditorRenderSnapshot` sem voltar a acoplar-se ao documento.
2. Renderizar plataformas editáveis, seleção e preview no espaço de mundo.
3. Renderizar cursor/feedback de ferramenta de forma económica, sem novo sistema de sprites.
4. Fazer o renderer respeitar exatamente o mesmo canvas/grid usado pelo hit-test.
5. Adicionar o HUD de ferramenta/tamanho sem sobrecarregar a tela.
6. Validar visualmente o fluxo real no jogo.
7. Atualizar documentação final e fechar a PR.

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
- empacotamento final do EXE release;
- Gate 9.4.5 de consolidação arquitetural.

## Critério de conclusão

A tranche só é considerada concluída quando for possível entrar no editor de nível real e, com mouse/teclado, selecionar, criar, mover e apagar elementos com feedback visual coerente, sem quebrar o estado de jogo normal, e quando os testes automatizados e uma validação manual do fluxo estiverem verdes.
