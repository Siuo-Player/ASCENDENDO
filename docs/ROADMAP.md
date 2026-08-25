# Roadmap de desenvolvimento

## Estado de referência

`main` contém a base integrada até à 9.4: editor core + migração incremental do renderer. A branch atual `feat/9-5-editor-ux-campaign` é, nesta tranche, **documentação e design**; esta PR não altera código.

## O que já existe

- Fases 1–8: motor, física, campanha, UI, texto TTF, sprites, replay/save e validação.
- 9.1: `GameAction` + `KeyBindings`.
- 9.2: rato, window→logical e menus clicáveis.
- 9.3: `GameState::EDITOR`.
- `LevelEditorDocument`, `EditorInteractionController`, `EditorSession` e `EditorRenderSnapshot`.
- Nova stack de rendering: `RendererCore`, `ShapeRenderer`, `WorldRenderer`, `UiRenderer`, `EditorRenderer`, `RendererFacade` e `RendererFacadeAdapter`.
- Level Editor baseado numa única tela lógica `640x360`.
- Base separada de `CampaignEditor` e `LevelEditorValidator`.
- Test runner Windows e source-size gate.

## Princípio estratégico: comunidade primeiro

O objetivo da base do jogo é permitir que o **editor sobreviva ao próprio desenvolvimento inicial** e continue utilizável pela comunidade.

Portanto, antes de enriquecer o catálogo de objetos, priorizamos:

- confiabilidade do modelo de nível;
- compatibilidade/versionamento;
- playtest rápido;
- validação independente;
- diagnóstico de falhas compreensível;
- layouts previsíveis;
- ferramentas de criação consistentes;
- partilha/importação segura.

Tipos de conteúdo mais ricos entram depois, sobre esta base, sem obrigar a reescrever níveis existentes.

## Princípios de design a partir da 9.5

Ver `docs/DESIGN_REFERENCES.md`.

1. Ferramenta ativa explícita: o editor deve mostrar o que o clique/drag fará.
2. Viewport previsível: limites, escala e área útil devem ser claros.
3. Snapping útil: grid visual e passo de snap podem ser diferentes.
4. Feedback imediato: preview, seleção, validação e erros aparecem no editor.
5. Teste é parte da criação: **editar → testar → corrigir → validar → guardar**.
6. Modelo e apresentação ficam separados.
7. Atalhos aceleram; não podem ser necessários para descobrir o produto.
8. Adotamos padrões comprovados sem transformar o ASCENDENDO num editor genérico.
9. A comunidade é tratada como utilizador de primeira classe do editor.
10. O Jump King é referência sobretudo visual/composicional e de workflow de criação/playtest, não especificação da física.

## Decisões de produto vigentes

Fonte de verdade: `docs/PRODUCT_DECISIONS.md`.

- `.lvl` = uma tela `640x360`.
- Level Editor sem pan; canvas completo sempre visível.
- Campaign Editor separado, vertical, com miniaturas 16:9.
- Níveis no Campaign Editor são blocos arrastáveis e reordenáveis por snap.
- Um nível pode abrir diretamente no Level Editor via mudança explícita de estado.
- Playtest com Player antes de guardar.
- Validação rápida em memória + feedback orientado a tutorial.
- Diagnóstico visual mostra percurso tentado e causa/local de falha quando disponível.
- Campaign Editor pode mostrar vários agentes/runs, incluindo transições entre níveis.
- Key bindings consultáveis no jogo; ações essenciais visíveis no rodapé.
- Funcionalidades importantes do editor não dependem de F-keys.
- UI/texto usam layout adaptável e nunca devem ultrapassar o viewport.
- Fullscreen é preservado; letterbox adapta o espaço lógico.
- `Começar` passa sempre por seleção de campanha, mesmo com uma só campanha.
- A carga do salto deve ter feedback visual claro através de uma barra/indicador de força.
- O modelo de nível deve reservar espaço para conteúdo futuro extensível e versionável.
- Importações/downloads são sempre revalidados pelo EXE.
- Objetivo final: EXE Windows x64 portátil.

## Fase 9 — Edição

### 9.4 ✅ — Editor core + migração incremental do renderer

Fechada e integrada em `main`.

### 9.5.a — UX do Level Editor

- teclas acessíveis (`1/2/3` em vez de F-keys);
- painel próprio de Controlos;
- rodapé contextual;
- layout de texto/UI autoajustável;
- fullscreen + letterboxing corretos;
- canvas `640x360` integralmente visível;
- guardar, playtest e validar integrados;
- feedback de validação orientado a tutorial;
- diagnóstico do percurso tentado;
- retorno seguro entre editor e jogo;
- indicador visual de força do salto.

**Critério:** um utilizador entra, percebe os comandos sem documentação externa, constrói, testa, diagnostica falhas e regressa ao jogo sem elementos cortados.

### 9.5.b — Seleção de campanhas

- `Começar` abre seleção de campanha;
- modelo preparado para várias campanhas;
- metadata de campanha separada da lista de níveis quando necessário;
- campanha selecionada determina explicitamente o conjunto jogado;
- preview e informação suficiente para escolher conscientemente.

### 9.5.c — Campaign Editor

- timeline vertical;
- miniaturas 16:9 compactadas;
- blocos arrastáveis;
- snap/reordenação;
- abrir Level Editor por mudança de estado;
- pré-visualização sequencial;
- agentes/runs de validação em background;
- pelo menos um agente por nível quando possível;
- pelo menos um agente de transição entre níveis;
- indicação visual da causa/local de falhas.

### 9.5.d — Playtest e validação

- playtest sem obrigar a guardar;
- estado editado separado do persistido;
- reset simples após playtest;
- validação rápida em memória;
- erro explicado pela primeira causa útil;
- percurso tentado mostrado quando uma simulação está disponível;
- validação final obrigatória antes de uma campanha jogável.

### 9.5.e — Base para conteúdo futuro

Sem ainda enriquecer o jogo com todos os objetos, a base do editor e do formato deve reservar espaço para:

- superfícies e variantes de plataforma;
- perigos;
- elementos móveis;
- objetos interativos;
- triggers/zonas;
- decoração;
- metadados de identidade visual;
- regras/eventos de campanha.

A prioridade é compatibilidade aditiva: novos elementos devem poder ser introduzidos sem invalidar mapas antigos sempre que tecnicamente possível.

## Gate 9.5.5 — Consolidação arquitetural

Só começa depois da 9.5 integrada:

1. reduzir responsabilidades do `main.cpp`;
2. fechar `Editor → RenderSnapshot → Renderer`;
3. migrar gameplay restante para `GameAction`/`KeyBindings`;
4. limitar catch-up excessivo do fixed timestep;
5. consolidar `LevelData` entre runtime/editor/parser;
6. tornar paths independentes do current working directory;
7. validar graphics/present queues e capacidades Vulkan;
8. Windows build/tests no CI;
9. ASan/UBSan no CI;
10. `make game` no CI;
11. invariantes/property tests relevantes;
12. limpar placeholders e documentação histórica redundante.

## Fase 10 — Guardar + validar

- versão explícita do `.lvl`;
- `LevelData` declarativo;
- serialização;
- área de dados apropriada;
- validação em background;
- mapas inválidos nunca entram na campanha;
- mapas importados/descarregados revalidados pelo EXE.

## Fase 11 — Partilha e biblioteca

### 11.1 — Export/import local

Pacote compacto declarativo, extração controlada e validação obrigatória pelo EXE.

### 11.2 — Biblioteca online

Site para upload/download de pacotes. HTTP(S) é suficiente inicialmente; WebSockets não são requisito.

### 11.3 — Partilha direta

Só adicionar comunicação bidirecional quando existir uma necessidade concreta.

## Fase 12 — Release / Portable Build

Objetivo: um executável Windows x64 que possa ser copiado para outro computador dentro dos requisitos mínimos e executado sem o ambiente de desenvolvimento.

Requisitos: EXE + DLLs/assets necessárias, sem dependência do current working directory, sem downloads obrigatórios e com mensagem amigável para requisitos ausentes.

## Regra de progressão

Nenhuma fase seguinte começa enquanto a anterior não tiver implementação coerente, testes relevantes, documentação atualizada, branch própria, PR aberta e PR integrada em `main`.

Antes de cada novo passo: integrar a PR anterior, abandonar a branch anterior e criar uma branch nova a partir do `main` atualizado.
