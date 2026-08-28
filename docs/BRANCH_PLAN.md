# Plano da branch atual

**Bloco do roadmap:** `Fase 10 / LevelData semantic validation`

**Work Package:** `Semantic LevelData geometry validation`

**Issue:** `#142`

**Branch de implementação:** `refactor/leveldata-semantic-validation-20260828`

## Contexto

A auditoria encontrou um gap entre parsing sintático e semântica de geometria: `LevelDataIO` aceitava plataformas/flags com largura ou altura zero/negativa.

## Decisão

Introduzir `LevelDataValidator` como boundary semântico independente:

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

A única regra nova desta tranche é extensão estritamente positiva para plataformas e flag.

## Escopo

- `Game/Logic/LevelDataValidator.h/.cpp`;
- validação após `LevelDataIO::load()` em `CampaignRuntime`;
- testes unitários das invariantes;
- testes de integração para entrada inicial/chunk inválido não consumido;
- documentação Fase 10.

## Fora de escopo

- schema/versionamento;
- migration;
- política de bounds do nível;
- física/colisão;
- redesign de `Level`;
- mudança do formato válido existente.

## Validação

```text
validator unit tests
→ malformed campaign-runtime tests
→ Linux normal/headless Vulkan
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
invalid geometry rejected before runtime append
+ valid campaign preserved
+ no progress consumed on rejected chunk
+ three mandatory CI gates green
+ documentation synchronized
```

## Estado atual

`IMPLEMENTED — pending PR/CI validation`

## Próxima decisão

Após o #142, continuar a Fase 10 apenas para invariantes semanticamente demonstráveis. Schema/versioning fica reservado para um requisito real de compatibilidade/importação.
