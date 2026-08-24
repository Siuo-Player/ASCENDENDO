# Plano da branch atual

**Branch:** `feat/9-4-model-grid-snap`

**Base:** `main` após integração da documentação no PR #3 (`ce522c2`).

## Estado herdado — concluído

- Fases 1–8 do roadmap.
- 9.1: `GameAction` + `KeyBindings`.
- 9.2: input de rato + viewport lógico + menus clicáveis.
- 9.3: `GameState::EDITOR`, câmera livre e grelha visual.
- PR #1: bootstrap GLFW/Vulkan e cleanup de falhas parciais.
- PR #3: documentação técnica, contrato de níveis, design do editor, testes e workflow.

O plano da branch anterior de documentação fica concluído.

## Objetivo desta branch — concluído

Construir a camada determinística da 9.4 antes de ligar interação gráfica.

### Implementado

- `LevelEditorDocument` em `Game/Logic/LevelEditor.*`;
- plataformas em memória com adicionar/mover/remover;
- snap único baseado em `config::EDITOR_GRID_SNAP`;
- rejeição de pedidos fora do canvas antes do snap;
- rejeição de dimensões inválidas;
- spawn com Y fixo no topo do chão inicial e X limitado à faixa segura do player;
- FLAG condicionada ao nível final da campanha;
- presets de tamanho pequeno/médio/grande;
- `GameAction`/`KeyBindings` para STAMP/DRAG, tamanho e apagar;
- nomes persistidos das novas teclas (`G`, `[`, `]`, `DELETE`, `BACKSPACE`);
- testes unitários do documento e dos bindings sem Vulkan.

## Não entrou nesta branch — deliberado

- UI de clique/arrasto no `Renderer`;
- gravação para disco;
- validação assíncrona;
- playlist/campaign editor;
- atlas/sistema final de sprites.

## Critérios de aceitação

Todos os critérios da tranche foram implementados na camada determinística. A execução completa de `make tests-verbose -j8` não foi possível neste ambiente porque o checkout remoto não pôde ser clonado por falta de resolução de rede; por isso o estado da PR deve continuar a indicar que a validação local em Windows/Clang é necessária.

## Próxima branch

A próxima branch deve começar a UI da 9.4 sobre este modelo testado: STAMP/DRAG, seleção, mover, apagar e bindings. A primeira tarefa deve ser renderizar o documento editável dentro do `EDITOR` e ligar `windowToLogical()` + snap + hit-testing ao modelo, sem introduzir save/async validation ainda.