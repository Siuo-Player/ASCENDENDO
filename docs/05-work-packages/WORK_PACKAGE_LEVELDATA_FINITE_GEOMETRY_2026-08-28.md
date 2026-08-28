# Work Package — LevelData finite geometry validation

## Identificação

**Roadmap:** `Fase 10 / LevelData semantic validation`
**Work Package:** `Finite LevelData geometry boundary`
**Issue:** `#144`
**Implementation branch:** `refactor/leveldata-finite-geometry-20260828`

## Contexto

A tranche #142 passou a rejeitar plataformas e flags com largura ou altura não positiva. A arquitetura do ASCENDENDO também estabelece que valores `NaN`/`Inf` não devem entrar na simulação.

## Decisão

Estender a mesma boundary semântica, sem alterar o parser, para exigir coordenadas finitas em `PLATFORM` e `FLAG`:

```text
min.x, min.y, max.x, max.y são finitos
+ width > 0
+ height > 0
```

O fluxo permanece:

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

## Escopo

- usar `std::isfinite` no `LevelDataValidator`;
- rejeitar coordenadas não finitas de plataformas e flag;
- adicionar testes para `NaN` e `±Inf`;
- preservar assets válidos e a semântica existente.

## Fora de escopo

- schema versioning;
- migration;
- bounds policy;
- `SPAWN` validation enquanto não houver consumidor runtime;
- redesign de `Level`;
- alterações de física/collision.

## Critério de saída

```text
non-finite geometry rejected before runtime append
+ positive finite geometry remains valid
+ no valid asset changes
+ mandatory CI gates green
+ documentation synchronized
```
