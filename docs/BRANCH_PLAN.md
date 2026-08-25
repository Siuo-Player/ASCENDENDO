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

Ligar o modelo determinístico do editor à UI/renderização real do jogo e, em paralelo, substituir gradualmente o renderer monolítico por passes pequenos, mantendo o renderer antigo intacto como referência até existir paridade comprovada.

## Implementado até agora

- `EditorSession` para orquestrar `InputManager`, `Camera`, `KeyBindings` e `EditorInteractionController`.
- `G` alterna entre `STAMP` e `DRAG` como ferramenta persistente.
- `[` / `]` alteram `SMALL` / `MEDIUM` / `LARGE`, começando em `MEDIUM`.
- Clique esquerdo cria ou move; clique direito cancela; `Delete`/`Backspace` apaga a seleção.
- Preview determinístico (`EditorPreview`) separado da renderização, incluindo recusa fora do canvas lógico.
- `EditorRenderSnapshot` separa os dados de apresentação do documento de edição; os passes gráficos não devem conhecer `LevelEditorDocument`.
- Integração do ciclo `EDITOR` no `main.cpp`, sem executar física enquanto o editor está ativo.
- Testes unitários de input → controller → documento, geometria do preview e conteúdo do snapshot.
- `docs/ARCHITECTURE.md` atualizado com a direção arquitetural alvo.
- `docs/TECH_DEBT.md` atualizado com a dívida técnica e a regra de tamanho de código.
- `docs/CODE_SIZE.md` criado com a política de tamanho físico.
- `Development/Tools/check_source_sizes.py` criado e ligado ao CI.
- Regra oficial de código: `<30 KiB` normal, `30–36 KiB` warning, `>36 KiB` bloqueado para novo código.
- Nova stack de renderer construída em paralelo: `RendererCore`, `ShapeRenderer`, `EditorRenderer`, `WorldRenderer`, `UiRenderer` e `RendererFacade`.
- `RendererFacadeAdapter` criado como ponte reversível entre a API antiga e a nova fachada.
- `docs/RENDERER_MIGRATION.md` criado com a matriz de paridade e os critérios de remoção do renderer antigo.
- Workflow de CI passou também a construir o binário `game` além dos testes.

## Estratégia atual para o `Renderer.cpp`

O `Renderer.cpp` antigo **não será desmontado diretamente**. Em vez disso:

```text
Renderer.cpp legado
      │
      ├── referência de comportamento
      └── fallback temporário

Nova stack
  ├── RendererCore
  ├── ShapeRenderer
  ├── EditorRenderer
  ├── WorldRenderer
  ├── UiRenderer
  └── RendererFacade
```

Cada pass novo é comparado contra o bloco correspondente do renderer legado. Quando a paridade for demonstrada, o caminho antigo daquele estado deixa de ser executado. Só no fim removemos o arquivo legado ou o reduzimos a uma casca compatível, consoante o que produzir a arquitetura mais simples.

O `Renderer.cpp` tem aproximadamente 34 KiB e está na zona de atenção da política de tamanho. A solução escolhida evita acrescentar responsabilidades a esse arquivo; o objetivo é **substituí-lo**, não torná-lo ainda maior.

## Matriz de migração

| Área | Novo componente | Estado |
|---|---|---|
| recursos Vulkan/frame lifecycle | `RendererCore` | ✅ construído |
| primitives / push constants | `ShapeRenderer` | ✅ construído |
| editor | `EditorRenderer` | ✅ construído |
| mundo / plataformas / FLAG / jogador | `WorldRenderer` | ✅ construído |
| timer / MENU / PAUSED / CREDITS | `UiRenderer` | ✅ construído |
| orquestração | `RendererFacade` | ✅ construída |
| adapter compatível | `RendererFacadeAdapter` | ✅ construído |
| integração runtime do `EDITOR` | fachada nova | ⏳ próximo |
| paridade visual/runtime | todos os passes | ⏳ |
| remoção do legado | `Renderer.cpp` | ⏳ fim da migração |

## Próximos passos desta tranche

1. Fazer o CI reconhecer e compilar a nova stack (`game` + testes).
2. Fazer o primeiro cut-over apenas do estado `EDITOR`, mantendo os restantes estados no renderer legado.
3. Validar visualmente `grelha → plataformas → seleção → preview → cursor → HUD`.
4. Comparar `PLAYING/PAUSED` e migrar o bloco de mundo para `WorldRenderer`.
5. Migrar `MENU/CREDITS` para `UiRenderer`.
6. Trocar `main.cpp` definitivamente para a fachada nova quando todos os estados tiverem paridade.
7. Remover qualquer consumidor do `Renderer.cpp` legado.
8. Só então eliminar o arquivo legado ou mantê-lo como fachada mínima, escolhendo a opção com menor complexidade.

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
- Nenhum novo ficheiro C/C++ deve ultrapassar 36 KiB; a partir de 30 KiB não deve receber novas responsabilidades sem um plano de subdivisão.

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

A tranche só é considerada concluída quando for possível entrar no editor de nível real e, com mouse/teclado, selecionar, criar, mover e apagar elementos com feedback visual coerente, sem quebrar o estado de jogo normal, quando os testes automatizados e a validação manual do fluxo estiverem verdes, e quando o renderer legado já não for necessário para o editor.
