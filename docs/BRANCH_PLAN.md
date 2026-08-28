# Plano da branch atual

**Bloco do roadmap:** `Fase 10 / LevelData semantic validation`

**Work Package concluído:** `Semantic LevelData geometry validation`

**Issue:** `#142`

**PR:** `#143`

**Branch de implementação:** `refactor/leveldata-semantic-validation-20260828`

## Resultado

A tranche introduziu `LevelDataValidator` como boundary semântico independente do parser. `CampaignRuntime` agora valida o documento imediatamente após `LevelDataIO::load()` e antes de fazer `appendFromData()`.

A regra semântica introduzida é deliberadamente mínima:

```text
PLATFORM width  > 0
PLATFORM height > 0
FLAG     width  > 0
FLAG     height > 0
```

Assim, o fluxo passa a ser:

```text
LevelDataIO
  parse
    ↓
LevelDataValidator
  validate
    ↓
CampaignRuntime
  append/use
```

## Validação

- validator unit tests: **success**
- malformed campaign-runtime tests: **success**
- Linux / Clang / C++20 / Headless Vulkan: **success**
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**
- Windows / Clang / C++20: **success**
- source-size: **success**
- campaign validation: **success**

**Merge:** `2ef4c1b4c25bbfe862ad8c05edad8f8438741835`

## Fora de escopo

- schema/versionamento;
- migration;
- política geral de bounds do nível;
- física/colisão;
- redesign de `Level`;
- mudança do formato válido existente.

## Próxima decisão

Continuar a Fase 10 apenas quando existir outra invariável semântica demonstrável. Schema/versioning permanece separado e só avança quando houver requisito real de compatibilidade/importação.
