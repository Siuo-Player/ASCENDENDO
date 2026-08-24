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

O plano da branch anterior de documentação fica, portanto, **concluído**.

## Objetivo desta branch

Construir a camada determinística da 9.4 antes de ligar interação gráfica:

1. introduzir um documento de nível editável em memória;
2. separar entidades editáveis das estruturas de física/render já existentes;
3. implementar snap ao `EDITOR_GRID_SNAP`;
4. implementar bounds/validação de colocação;
5. suportar operações primitivas de adicionar/mover/remover plataforma;
6. definir spawn e FLAG como entidades/regras especiais;
7. cobrir a lógica com testes unitários sem Vulkan.

## Não entra nesta branch

- UI de clique/arrasto no `Renderer`;
- gravação para disco;
- validação assíncrona;
- playlist/campaign editor;
- atlas/sistema final de sprites.

## Critérios de aceitação

- uma coordenada nunca fica fora do espaço legal depois de uma operação do editor;
- snap é determinístico e centralizado em `Config.h`;
- mover uma entidade não altera outra entidade;
- remover é determinístico;
- o spawn só aceita X dentro da faixa segura do primeiro chão;
- FLAG só pode ser representada no último nível da campanha;
- as regras são testáveis sem abrir uma janela ou inicializar Vulkan.

## Próximo passo

Depois de esta branch ser integrada, abrir uma branch nova para a UI da 9.4 sobre o modelo já testado: STAMP/DRAG, seleção, mover, apagar e bindings.