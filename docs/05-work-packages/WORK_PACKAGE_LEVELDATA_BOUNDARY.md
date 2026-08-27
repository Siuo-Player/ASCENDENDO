# Work Package — LevelData boundary

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Runtime / Editor / Level IO`
**Work Package:** `9.6 LevelData boundary`
**Branch:** `refactor/9-6-leveldata-boundary`
**PR:** `#75`

## Objetivo

Unificar a representação declarativa de um nível entre parser/IO, editor e runtime através de um `LevelData` independente de Vulkan/GLFW.

## Contexto / descoberta

A investigação do estado atual encontrou duas representações concorrentes do mesmo conteúdo: `logic::Level` fazia parsing textual diretamente, enquanto `logic::LevelEditorDocument` mantinha um modelo de edição separado. O serializer do editor também não persistia spawn. Isto criava risco de divergência.

## Decisão arquitetural

Criar `logic::LevelData` como modelo declarativo local de um único nível, independente de Vulkan/GLFW e sem estado de streaming/runtime.

`LevelData` contém nome, plataformas, posição de spawn opcional e flag opcional. Offsets Y de campaign/streaming permanecem responsabilidade de `CampaignRuntime`/`Level`.

```text
.lvl
  ↓
LevelData
  ├── Level runtime view
  └── LevelEditorDocument view
```

## Alternativas rejeitadas

- manter `Level` como parser principal;
- usar `LevelEditorDocument` como modelo comum;
- introduzir framework genérico de serialization;
- persistir `offsetY` no modelo declarativo.

## Resultado implementado

- `LevelData` introduzido;
- `LevelDataIO` passou a concentrar parsing/serialização;
- `Level` consome `LevelData` e mantém composição em world coordinates;
- editor exporta para `LevelData` antes de serializar;
- formato histórico `NAME/PLATFORM/FLAG` permanece compatível;
- `SPAWN` é opcional nesta tranche;
- versionamento explícito (`VERSION`) permanece na Fase 10.

## Invariantes

- `LevelData` não depende de Vulkan/GLFW;
- dados persistidos usam coordenadas locais;
- streaming offset não é persistido;
- ausência de `SPAWN` é distinta de `SPAWN 0 0`;
- regras editoriais permanecem no editor;
- runtime conserva o avanço fixo de `LOGICAL_HEIGHT` por chunk.

## Validação

Foram adicionados testes de compatibilidade histórica, `SPAWN 0 0`, round-trip, conversão editor → `LevelData` e composição runtime com offsets.

Evidência CI final:

- **Tests #840 / run `33028268190` — success**: build game, testes, headless Vulkan e campaign validation;
- **Sanitizers / run `33028268183` — success**: ASan + UBSan, headless Vulkan e campaign validation;
- source-size checks passaram nos dois workflows.

Durante a implementação houve duas falhas de compilação reais e corrigidas:

1. include excessivo em `LevelEditor.h`, causando tipos `EditorToolMode`/`EditorSizePreset` desconhecidos;
2. `Tests/Unit/test_level_file_loading.cpp` ainda dependia de `Level::appendFromFile` depois da mudança de API.

Ambas foram diagnosticadas pelos logs dos runners e corrigidas antes do head final.

## Definition of Done

- [x] `LevelData` é o modelo comum declarativo;
- [x] parser/serializer deixaram de depender diretamente do runtime/editor model;
- [x] runtime mantém comportamento de streaming;
- [x] editor mantém invariantes existentes;
- [x] round-trip e fixtures passam;
- [x] normal + ASan/UBSan verdes;
- [x] PR #75 validada;
- [x] PR #75 integrada;
- [x] versionamento do formato explicitamente mantido para a Fase 10.

## Fecho

**Estado:** concluído.

A fronteira `LevelData` está estabelecida. O próximo trabalho do roadmap volta ao `9.6 Base Engineering Gate`: decomposição restante de `main.cpp`, revisão dos P0 Vulkan/runtime e preparação do contrato de validação/versionamento antes do avanço para as fases seguintes.
