# Work Package — Malformed level syntax evidence

## Identificação

**Roadmap:** 9.6 — Evidência transversal  
**Subsistema:** Runtime / Levels  
**Work Package:** 9.6 — Malformed level syntax evidence  
**Branch:** `fix/9-6-vulkan-reconfigure-terminal-contract-20260827` (historical implementation branch)  
**PR:** #92 (replacement for superseded PR #91)

## Objetivo

Demonstrar a política estrutural atualmente implementada por `LevelDataIO`: sintaxe desconhecida, números inválidos, campos truncados e trailing tokens são rejeitados de forma determinística.

## Não inclui

- schema/versioning;
- validação semântica de domínio;
- alteração de warnings de geometria para errors;
- import/export ou UGC trust boundary.

## Decisão

`LevelDataIO` permanece parser/serializer estrito da gramática atual. A validação semântica continua uma decisão posterior da Fase 10.

```text
malformed syntax → reject
valid syntax → LevelData
semantic validity → Fase 10
```

## Validação

O teste cobre token desconhecido, número inválido, campo truncado, trailing token em `PLATFORM`, `FLAG` e `SPAWN`, e uma entrada válida da gramática atual.

Os ficheiros temporários são únicos por execução e removidos no destrutor do fixture.

A evidência foi integrada pela PR #92 sobre a `main` atual. A PR #91 foi superseded por divergência com a documentação/base entretanto atualizada e não contém a evidência final integrada.

## Critério de saída

Suite normal, ASan/UBSan e Windows devem passar no mesmo commit. O resultado fecha apenas a evidência de malformed syntax; semantic validation permanece aberta.

## Estado

`concluído — evidência integrada em #92`

## Evidência residual

Não existe, nesta tranche, qualquer afirmação de schema authority, semantic validation ou canonicalization.
