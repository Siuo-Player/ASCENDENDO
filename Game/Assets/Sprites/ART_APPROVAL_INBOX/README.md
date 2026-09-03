# ASCENDENDO — Pixel-Art Approval Inbox

**Estado:** `PENDING HUMAN APPROVAL`

Esta pasta é a fronteira entre **candidato visual** e **arte aprovada para o produto**.

Nada nesta pasta é considerado aprovado. Nenhum candidato deve ser promovido para o runtime, para o registry canónico ou para uma composição final apenas por estar aqui listado.

## Como usar

Revê cada ID e decide:

```text
APPROVE — pode avançar para integração após as verificações técnicas.
REJECT  — não usar no produto.
REWORK  — a ideia é útil, mas requer alterações antes de nova avaliação.
LIMIT   — usar apenas numa função/contexto explicitamente indicado.
```

A decisão humana visual é obrigatória. Licença, provenance, resolução ou passagem de testes técnicos não substituem essa decisão.

## Candidatos actualmente identificados

| ID | Candidato | O que é | Para que serviria | Estado |
|---|---|---|---|---|
| `ART-001` | `personagem.png` / `Source/personagem.pixil` | Sprite actual do personagem | Player/avatar principal | `PENDING` |
| `ART-002` | `kenney.pixel-line-platformer` | Tileset pixel-art 16×16 | Vocabulário visual de plataformas | `PENDING` |
| `ART-003` | `hdst.platformer-pack-16x16` | Tileset pixel-art 16×16 | Alternativa visual para plataformas | `PENDING` |
| `ART-004` | `kenney.pixel-platformer` | Tileset pixel-art 18×18 | Referência/comparação; não corresponde ainda ao módulo canónico 16×16 | `PENDING / REFERENCE` |
| `ART-005` | `generic.platformer-tileset-16x16` | Tileset + background 16×16 | Alternativa para terreno/ambiente | `PENDING` |
| `ART-006` | `FLAG` | Arte necessária ainda não seleccionada | Goal/flag de nível final | `NO CANDIDATE SELECTED` |
| `ART-007` | `PROPS` | Conjunto ambiental ainda não seleccionado | Decoração/ambiente sem alterar colisões | `NO CANDIDATE SELECTED` |

## Evidência e provenance

Os candidatos externos `ART-002` a `ART-005` são apenas referências já registadas no `PLATFORM_ASSET_REGISTRY.md`. A pasta não descarrega nem importa esses packs automaticamente.

Para qualquer promoção futura, devem permanecer disponíveis:

```text
source URL
creator
licence
asset/version identifier
retrieval date
local modification status
```

E também devem ser avaliados no produto:

```text
escala pixel-art
legibilidade da personagem
contacto das plataformas
silhueta
seams e continuidade
coerência entre materiais
compatibilidade com a apresentação 16×16
captura determinística
revisão visual humana
```

## Regra de promoção

Só depois de uma decisão humana positiva é permitido criar uma alteração de integração. A promoção deve referenciar **o ficheiro exacto** aprovado e conservar a provenance; candidatos rejeitados não devem ser substituídos silenciosamente por outros.

`NO WINNER` é um resultado válido.
