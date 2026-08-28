# Work Package — LevelData finite geometry

**Issue:** #144

## Contexto

A validação semântica #142 passou a exigir extensões estritamente positivas para plataformas e flag. A revisão seguinte encontrou a propriedade já exigida pela arquitetura da simulação: nenhum `NaN`/`Inf` deve atravessar a fronteira de dados para o runtime.

## Decisão

Estender a boundary `LevelDataValidator` com uma regra mínima e independente do parser:

```text
all AABB coordinates are finite
+ width > 0
+ height > 0
```

Isto cobre plataformas e flag.

## Fora de escopo

- schema/versionamento;
- migration;
- bounds policy;
- redesign de `Level`;
- physics/collision changes;
- mudanças no grammar parser.

## Critério de saída

```text
non-finite geometry rejected before runtime append
+ valid campaign unchanged
+ focused semantic tests
+ Linux normal / ASan-UBSan / Windows green
+ campaign validation green
```
