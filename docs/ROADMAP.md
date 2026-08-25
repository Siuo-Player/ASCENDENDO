# Roadmap de desenvolvimento

## Estado de referência

`main` contém a base integrada até à 9.4 anterior. A branch atual, `feat/9-4-editor-visual-integration`, desenvolve a integração visual e o modelo dos editores.

Concluído:

- Fases 1–8: motor, física, campanha, UI, texto TTF, sprites, replay/save e validação.
- 9.1: `GameAction` + `KeyBindings`.
- 9.2: rato + conversão window/logical + menus clicáveis.
- 9.3: `GameState::EDITOR` e infraestrutura inicial do editor.
- bootstrap GLFW/Vulkan robusto + cleanup de falhas parciais.
- editor determinístico com grid/snap, plataformas, spawn, FLAG e testes.
- `EditorInteractionController` + `EditorSession`.
- nova stack de rendering construída em paralelo: `RendererCore`, `ShapeRenderer`, `WorldRenderer`, `UiRenderer`, `EditorRenderer`, `RendererFacade`.
- `EDITOR` já faz cut-over através de `RendererFacadeAdapter`.
- Level Editor agora assume uma única tela lógica fixa de `640x360`.
- base separada de `CampaignEditor` com representação vertical das telas.
- `LevelEditorValidator` para validação em memória.
- ações preparadas para guardar/testar/validar.

## Decisões de produto vigentes

A fonte de verdade destas decisões é `docs/PRODUCT_DECISIONS.md`.

- Um `.lvl` corresponde a uma única tela de `640x360`.
- No Level Editor não existe deslocação vertical/horizontal da tela; o canvas completo é editado no mesmo espaço.
- O Campaign Editor é uma ferramenta distinta, com timeline vertical e miniaturas 16:9 compactadas.
- Os níveis são blocos arrastáveis no Campaign Editor e podem usar snap para reordenar.
- Um nível pode ser aberto diretamente do Campaign Editor através de uma mudança explícita de estado.
- O nível deve poder ser testado com o Player antes de ser guardado definitivamente.
- O validador deve fornecer feedback ao vivo durante a construção.
- O Campaign Editor deve conseguir mostrar vários agentes/runs de validação em background, incluindo agentes que atravessem fronteiras entre níveis para visualizar a fluidez da campanha.
- Os key bindings devem ser consultáveis num local próprio do menu e os menus devem mostrar no ecrã as ações essenciais do estado atual, pelo menos navegar/confirmar/voltar ou sair.
- Mapas importados ou descarregados são sempre validados novamente pelo próprio EXE.
- O objetivo final continua a ser um executável Windows x64 portátil.

## Fase 9 — Edição

### 9.4 — Editor de níveis e renderização

#### 9.4.a ✅ — Modelo e interação

Concluído:

- `LevelEditorDocument`;
- snap e bounds;
- spawn e FLAG;
- `EditorInteractionController`;
- `EditorSession`;
- STAMP/DRAG;
- seleção/movimento/delete;
- bindings base;
- testes determinísticos.

#### 9.4.b ▶ — Editor visual real

Em curso:

- canvas fixo 640x360;
- moldura física visível;
- grelha visual coerente com a escala do jogador/blocos;
- preview/seleção;
- HUD de ferramentas;
- `F2` guardar;
- `F5` testar;
- `F6` validar;
- validação ao vivo;
- testar sem guardar;
- estado seguro para voltar ao nível/campanha.

**Critério de conclusão:** editar → testar com Player → voltar ao editor → validar ao vivo → guardar sem regressões.

### 9.4.c ▶ — Descobribilidade de controlos

- menu/overlay dedicado a consultar todos os key bindings relevantes;
- rodapé consistente nos menus com navegar/confirmar/voltar ou sair;
- não é necessário suportar rebind completo nesta fase.

### 9.4.d ▶ — Editor de campanha

- timeline vertical;
- miniaturas compactadas mantendo 16:9;
- níveis como blocos arrastáveis;
- snap/reordenação;
- abrir Level Editor por mudança de estado;
- pré-visualização dos níveis em sequência;
- agentes/runs de validação em background;
- pelo menos um agente por nível quando possível;
- pelo menos um agente capaz de mostrar transição entre níveis.

## Gate 9.4.5 — Consolidação arquitetural antes de Save

Só inicia depois de 9.4 estar realmente funcional e integrado.

Prioridade:

1. reduzir responsabilidades do `main.cpp`;
2. fechar `Game/Editor → RenderSnapshot → Renderer`;
3. migrar gameplay restante para `GameAction`/`KeyBindings`;
4. limitar catch-up excessivo do fixed timestep;
5. consolidar `LevelData` comum entre runtime/editor/parser;
6. tornar paths independentes do current working directory;
7. validar graphics/present queues e capacidades Vulkan;
8. Windows build/tests no CI;
9. ASan/UBSan no CI;
10. `make game` no CI;
11. invariantes/property tests relevantes;
12. limpar placeholders e documentação histórica redundante.

## Fase 9.5 — Guardar + validar

- versão explícita no `.lvl`;
- `LevelData` declarativo;
- serialização;
- guardar em área apropriada;
- validação em background;
- mapas inválidos nunca entram na campanha jogável;
- qualquer mapa importado/descarregado é novamente validado pelo EXE.

## Fase 10 — Hardening e ferramentas

- CI Windows completo;
- build/link do jogo no CI;
- ASan/UBSan;
- validação Vulkan robusta;
- ownership/RAII;
- AssetResolver quando justificar;
- configuração separada por domínio;
- undo/redo transacional;
- testes de parser malformado, viewport, física e editor;
- property-based tests onde façam sentido;
- limpeza de `.gitkeep`, placeholders e runtime data versionados.

## Fase 11 — Partilha e biblioteca de mapas

### 11.1 — Export/import local

- Exportar/Partilhar;
- Importar;
- pacote compacto declarativo;
- extração controlada;
- validação obrigatória pelo EXE.

### 11.2 — Biblioteca online

Site para upload/download de pacotes de mapas. HTTP(S) é suficiente inicialmente; WebSockets não são requisito inicial.

### 11.3 — Partilha direta

Só adicionar comunicação bidirecional (WebSockets/WebRTC/etc.) quando houver necessidade concreta.

## Fase 12 — Release / Portable Build

Objetivo final:

> um executável Windows x64 que possa ser copiado para outro computador dentro dos requisitos mínimos e executado sem instalar o ambiente de desenvolvimento.

Requisitos:

- `.exe` portable;
- assets/DLLs necessárias incluídas;
- sem dependência do current working directory;
- sem downloads obrigatórios;
- mensagem amigável para requisitos ausentes;
- suporte visado ao hardware mais fraco que suporte Vulkan corretamente.

## Regra de progressão

Nenhuma fase seguinte começa enquanto a anterior não tiver:

- implementação coerente;
- testes relevantes;
- documentação atualizada;
- branch própria;
- PR aberta;
- PR integrada em `main`.

Antes de cada novo passo: integrar a PR anterior, abandonar a branch anterior e criar uma branch nova a partir de `main` atualizado.
