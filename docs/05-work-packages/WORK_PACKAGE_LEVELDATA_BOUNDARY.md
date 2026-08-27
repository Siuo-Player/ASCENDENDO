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

- `logic::Level` guarda plataformas e flag e também continha o parser textual em `appendFromFile()`;
- `logic::LevelEditorDocument` guarda plataformas, spawn e flag para edição;
- `saveEditorLevel()` serializava diretamente `LevelEditorDocument` para `.lvl`;
- `CampaignRuntime` mandava `Level` interpretar diretamente cada `.lvl`.

A diferença não é apenas de API: o editor possui estado declarativo de spawn que o serializer não persistia, enquanto o runtime recebia apenas plataformas/flag através de parsing direto. Isto cria risco de divergência sem uma representação intermediária única.

## Decisão arquitetural

Criar `logic::LevelData` como modelo declarativo local de um único nível, independente de Vulkan/GLFW e sem estado de streaming/runtime.

`LevelData` contém apenas dados que pertencem ao conteúdo de um nível: nome, plataformas, posição de spawn opcional e flag opcional.

A arquitetura é:

```text
.lvl
  ↓
LevelData
  ├── Level runtime view
  └── LevelEditorDocument view
```

`Level` continua responsável pela geometria runtime e pela composição de chunks no mundo, mas deixa de ser o parser principal do formato. `LevelEditorDocument` continua responsável pelas regras de edição (snap, limites, regra de flag final), expondo uma conversão para `LevelData` sem transferir essas regras ao modelo declarativo.

Offsets Y de campaign/streaming **não** fazem parte de `LevelData`; são responsabilidade de `CampaignRuntime`/`Level` ao compor níveis locais no mundo.

## Alternativas consideradas

1. Manter `Level` como parser e converter para editor quando necessário — rejeitado porque mantém parsing acoplado ao runtime.
2. Fazer `LevelEditorDocument` ser o modelo comum — rejeitado porque contém regras/estado próprios do editor.
3. Introduzir um framework genérico de serialization/schema — rejeitado; o formato `.lvl` é simples e específico do projeto.
4. Incluir `offsetY`/streaming em `LevelData` — rejeitado; mistura conteúdo persistido com estado runtime.

## Inclui

- introduzir `LevelData` declarativo;
- separar parsing/serialização do modelo runtime/editor;
- adaptar `Level` para consumir `LevelData` e continuar aplicando offset de streaming;
- adaptar `LevelEditorDocument` para exportar dados declarativos;
- preservar o formato textual histórico nesta tranche, com `SPAWN` opcional e sem `VERSION`;
- testar round-trip semântico e consumidores atuais;
- sincronizar documentação após validação.

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
 ├── LevelDataIO
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
- ausência de `SPAWN` é distinta de `SPAWN 0 0`;
- carregar e serializar preservam semanticamente nome, plataformas, spawn e flag suportados;
- `Level` mantém as regras atuais de composição em world coordinates;
- regras editoriais continuam pertencentes ao editor, não ao modelo de transporte.

## Riscos

| Risco | Impacto | Mitigação | Estado |
|---|---|---|---|
| Alterar parsing muda níveis existentes | alto | manter grammar atual e adicionar fixtures de round-trip | mitigado; teste adicionado |
| Spawn existente não aparece nos `.lvl` atuais | médio | `SPAWN` opcional; ficheiros históricos permanecem sem spawn | resolvido por contrato |
| Mistura entre modelo declarativo e regras editoriais | médio | `LevelData` neutro e validação no editor/runtime | resolvido estruturalmente |
| Streaming offset regressa | alto | teste explícito de composição com offset | coberto |
| Mudança inadvertida no header do editor | médio | preservar includes mínimos e CI de compilação | encontrada e corrigida |

## Validation

- unit tests de `LevelData`/parser/serialization;
- fixtures `.lvl` existentes;
- round-trip semântico;
- composição runtime com `offsetY`;
- `make tests` Linux/headless;
- ASan/UBSan;
- campaign validation;
- revisão do dependency map e ownership graph.

### Falha CI observada e corrigida

**Run:** `33028119249`  
**Commit:** `f675965b6a73f4cdc71064ce1eff4baa5215cc07`  
**Failing step:** `Build and run tests in virtual X display`  
**Classificação:** erro de compilação introduzido pela branch.  
**Causa confirmada:** `LevelEditor.h` passou temporariamente a incluir `EditorRenderSnapshot.h`, que por sua vez dependia dos tipos `EditorToolMode`/`EditorSizePreset` definidos no mesmo header de editor. Isso criou uma ordem de inclusão circular/indefinida.  
**Correção:** restaurados os includes mínimos de `LevelEditor.h`: apenas `Physics.h` e `LevelData.h`, mais headers STL necessários.  
**Próxima validação:** nova execução normal + ASan/UBSan a partir do estado corrigido.

Não foram feitas alterações atribuídas a infraestrutura; o log mostrou compilação falhando em `EditorInteraction.cpp`/`EditorLevelIO.cpp` com tipos desconhecidos concretos.

## Definition of Ready

- [x] código atual de `Level`, `LevelEditorDocument`, `EditorLevelIO` e `CampaignRuntime` investigado;
- [x] consumidores e diferenças semânticas identificados;
- [x] decisão de fronteira registrada;
- [x] offsets de streaming explicitamente fora do modelo;
- [x] validação definida.

## Definition of Done

- [x] `LevelData` é o modelo comum declarativo;
- [x] parser/serializer deixaram de depender diretamente do runtime/editor model;
- [x] runtime mantém comportamento de streaming;
- [x] editor mantém invariantes existentes;
- [x] round-trip e fixtures foram adicionados;
- [ ] normal + ASan/UBSan verdes após a correção final;
- [ ] architecture/roadmap/TECH_DEBT sincronizados;
- [ ] PR integrada.

## Alterações durante execução

```text
1. `LevelEditor.h` recebeu includes de apresentação por engano.
   Evidência: build CI falhou com tipos EditorToolMode/EditorSizePreset desconhecidos.
   Correção: restaurado header mínimo e desacoplado.

2. `spawnPosition` foi inicialmente modelado como Vec2 obrigatório.
   Problema: `SPAWN 0 0` seria indistinguível de ausência.
   Correção: `std::optional<Vec2>` para preservar a semântica.

3. `LevelDataIO` inicialmente rejeitava documentos sem plataformas.
   Compatibilidade: o parser histórico permitia essa forma.
   Correção: remoção dessa restrição para não introduzir uma regra nova nesta tranche.

4. `LevelEditorDocument` originalmente serializava implicitamente o chão inicial.
   Compatibilidade: `toLevelData()` materializa novamente esse chão.
```

## Fecho

**Estado:** implementação concluída, em revalidação após correção de compilação.

**Próxima decisão:** confirmar normal + ASan/UBSan. Se ambos passarem, sincronizar os documentos normativos e abrir/integrar a PR. O versionamento do formato permanece separado para a Fase 10.
