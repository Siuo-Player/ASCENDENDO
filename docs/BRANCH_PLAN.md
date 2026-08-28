# Plano da branch atual

**Bloco do roadmap:** `Fase 10 / LevelData semantic validation`

**Work Package:** `Finite LevelData geometry boundary`

**Issue:** `#144`

**Branch de implementação:** `refactor/leveldata-finite-geometry-20260828`

## Contexto

A tranche #142 passou a rejeitar plataformas e flags com largura ou altura não positiva. A arquitetura do ASCENDENDO também estabelece que valores `NaN`/`Inf` não devem entrar na simulação.

## Decisão

Estender a mesma boundary semântica, sem alterar o parser, para exigir coordenadas finitas em `PLATFORM` e `FLAG`:

```text
min.x, min.y, max.x, max.y são finitos
+ width > 0
+ height > 0
```

## Escopo

- `Game/Logic/LevelDataValidator`;
- rejeitar coordenadas não finitas de plataformas e flag;
- testes para `NaN` e `±Inf`;
- preservar assets válidos e comportamento existente.

## Fora de escopo

- schema/versionamento;
- migration;
- bounds policy;
- `SPAWN` validation sem consumidor runtime;
- redesign de `Level`;
- física/collision.

## Validação

```text
validator unit tests                  success
Linux normal/headless Vulkan          success
ASan/UBSan                            success
Windows                               success
source-size/campaign validation       success
```

## Critério de saída

```text
finite + positive geometry accepted
+ NaN/Inf geometry rejected before runtime append
+ no valid asset changes
+ three mandatory CI gates green
+ documentation synchronized
```

## Estado atual

`COMPLETED — PR #145 merged as 34c96b83573add90bd1f3d238f62d8f37ba3c9a9`
