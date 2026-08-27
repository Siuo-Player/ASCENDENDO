# Work Package — LevelData boundary

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Runtime / Editor / Level IO`
**Work Package:** `9.6 LevelData boundary`
**Branch:** `refactor/9-6-leveldata-boundary`
**PR:** `<to be created>`

## Objetivo

Unificar a representação declarativa de um nível entre parser/IO, editor e runtime através de um `LevelData` independente de Vulkan/GLFW.

## Contexto / descoberta

A investigação do estado atual encontrou duas representações concorrentes do mesmo conteúdo:

- `logic::Level` guarda plataformas e flag e também contém o parser textual em `appendFromFile()`;
- `logic::LevelEditorDocument` guarda plataformas, spawn e flag para edição;
- `saveEditorLevel()` serializa diretamente `LevelEditorDocument` para `.lvl`;
- `CampaignRuntime` manda `Level` interpretar diretamente cada `.lvl`.

A diferença não é apenas de API: o editor possui estado declarativo de spawn que o serializer atualmente não persiste, enquanto o runtime recebe apenas plataformas/flag através de parsing direto. Isto cria risco de divergência sem uma representação intermediária única.

## Decisão arquitetural

Criar `logic::LevelData` como modelo declarativo local de um único nível, independente de Vulkan/GLFW e sem estado de streaming/runtime.

`LevelData` conterá apenas dados que pertencem ao conteúdo de um nível: nome, plataformas, posição de spawn e flag opcional.

A arquitetura será:

```text
.lvl
  ↓
LevelData
  ├── Level runtime view
  └── LevelEditorDocument view
```

`Level` continuará responsável pela geometria runtime e pela composição de chunks no mundo, mas deixará de ser o parser principal do formato. `LevelEditorDocument` continuará responsável pelas regras de edição (snap, limites, regra de flag final), usando `LevelData` como representação de conteúdo e não como substituto das invariantes de edição.

Offsets Y de campaign/streaming **não** fazem parte de `LevelData`; são responsabilidade de `CampaignRuntime`/`Level` ao compor níveis locais no mundo.

## Alternativas consideradas

1. Manter `Level` como parser e converter para editor quando necessário — rejeitado porque mantém duas representações concorrentes e preserva parsing acoplado ao runtime.
2. Fazer `LevelEditorDocument` ser o modelo comum — rejeitado porque contém regras/estado próprios do editor e depende de invariantes de interação que não pertencem ao runtime.
3. Introduzir um framework genérico de serialization/schema — rejeitado; o formato `.lvl` é simples e específico do projeto e não justifica uma camada genérica.
4. Incluir `offsetY`/streaming em `LevelData` — rejeitado; isso mistura conteúdo persistido com estado de composição runtime.

## Inclui

- introduzir `LevelData` declarativo;
- separar parsing/serialização do modelo runtime/editor;
- adaptar `Level` para consumir `LevelData` e continuar aplicando offset de streaming;
- adaptar `LevelEditorDocument` para exportar/receber dados declarativos preservando as invariantes de edição;
- preservar o formato textual atual nesta tranche, sem introduzir versioning ainda;
- testar round-trip semântico do modelo e os consumidores atuais;
- sincronizar arquitetura, roadmap e dívida.

## Não inclui

- `VERSION` no formato `.lvl`;
- migrações de schema;
- campanha metadata;
- undo/redo;
- import/export web;
- RenderSnapshot;
- alterações de física/collision semantics.

## Dependências

```text
LevelData
 ├── Level parser/serializer
 ├── Level runtime
 └── LevelEditorDocument

CampaignRuntime
 └── Level + local LevelData composition
```

**Consumidores afetados:** `Level`, `CampaignRuntime`, `EditorLevelIO`, `LevelEditorDocument`, editor tests e level loading tests.

**Validação:** testes unitários sem GPU, testes de file loading/round-trip e suite Linux/headless existente.

## Invariantes

- `LevelData` não inclui tipos ou ownership de Vulkan/GLFW;
- dados persistidos descrevem um único nível em coordenadas locais;
- streaming offset nunca é persistido no `LevelData`;
- carregar e serializar preservam semanticamente nome, plataformas, spawn e flag suportados;
- `Level` mantém as regras atuais de composição em world coordinates;
- regras editoriais continuam pertencentes ao editor, não ao modelo de transporte.

## Riscos

| Risco | Impacto | Mitigação | Estado |
|---|---|---|---|
| Alterar parsing muda níveis existentes | alto | manter grammar atual e adicionar fixtures de round-trip | aberto |
| Spawn existente não aparece nos `.lvl` atuais | médio | preservar fallback/default atual e explicitar diferença na documentação | aberto |
| Mistura entre modelo declarativo e regras editoriais | médio | manter `LevelData` neutro e validação no editor/runtime | aberto |
| Streaming offset regressa | alto | testes explícitos de composição de chunks com offsets distintos | aberto |

## Validation

- unit tests de `LevelData`/parser/serialization;
- fixtures `.lvl` existentes;
- round-trip semântico;
- composição runtime com `offsetY`;
- `make tests` Linux/headless;
- ASan/UBSan;
- campaign validation;
- revisão do dependency map e da ownership graph.

## Definition of Ready

- [x] código atual de `Level`, `LevelEditorDocument`, `EditorLevelIO` e `CampaignRuntime` investigado;
- [x] consumidores e diferenças semânticas identificados;
- [x] decisão de fronteira registrada;
- [x] offsets de streaming explicitamente fora do modelo;
- [x] validação definida.

## Definition of Done

- [ ] `LevelData` é o modelo comum declarativo;
- [ ] parser/serializer deixam de depender diretamente do runtime/editor model;
- [ ] runtime mantém comportamento de streaming;
- [ ] editor mantém invariantes existentes;
- [ ] round-trip e fixtures passam;
- [ ] normal + ASan/UBSan verdes;
- [ ] architecture/roadmap/TECH_DEBT sincronizados;
- [ ] PR integrada.

## Alterações durante execução

Registar aqui qualquer descoberta que altere a decisão, especialmente diferenças semânticas entre `.lvl` histórico e o estado editável atual.

## Fecho

**Estado:** investigação concluída, implementação por iniciar.

**Próxima decisão:** implementar `LevelData` com a menor superfície possível e adaptar primeiro IO/parser, depois runtime/editor consumers.
