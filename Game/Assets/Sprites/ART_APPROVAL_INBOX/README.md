# ASCENDENDO — Pixel-Art Approval Inbox

**Estado:** `APPROVED BY USER WITH SIZE GATE`

Esta pasta é a fronteira entre **candidato visual** e **arte aprovada para o produto**.

A decisão humana registada nesta revisão é aplicar aprovação por defeito quando o candidato tem um tamanho compatível com a escala já usada pelo projecto. Candidatos sem tamanho verificável ficam em `REWORK` até existir evidência concreta do tamanho correcto.

## Regra de tamanhos adoptada

| Função | Tamanho de referência |
|---|---|
| Player actual (`ART-001`) | `32×32 px` |
| Tiles/plataformas modulares | `16×16 px` |
| Elementos que ocupem várias células | múltiplos inteiros de `16 px` por eixo |
| Flag / goal | `16×16 px` por unidade de tile, podendo compor múltiplos de `16 px` |
| Props ambientais | módulos base de `16×16 px`; objectos maiores em múltiplos de `16 px` |

## Decisões actuais

| ID | Candidato | O que é | Para que serviria | Estado | Tamanho exigido |
|---|---|---|---|---|---|
| `ART-001` | `personagem.png` / `Source/personagem.pixil` | Sprite actual do personagem | Player/avatar principal | `APPROVE` | `32×32 px` |
| `ART-002` | `kenney.pixel-line-platformer` | Tileset pixel-art 16×16 | Vocabulário visual de plataformas | `APPROVE` | `16×16 px` |
| `ART-003` | `hdst.platformer-pack-16x16` | Tileset pixel-art 16×16 | Alternativa visual para plataformas | `APPROVE` | `16×16 px` |
| `ART-004` | `kenney.pixel-platformer` | Tileset pixel-art 18×18 | Referência/comparação | `REWORK` | converter/substituir por `16×16 px`; não integrar a 18×18 |
| `ART-005` | `generic.platformer-tileset-16x16` | Tileset + background 16×16 | Alternativa para terreno/ambiente | `APPROVE` | `16×16 px` para tiles; elementos compostos em múltiplos de `16 px` |
| `ART-006` | `FLAG` | Arte necessária ainda não seleccionada | Goal/flag de nível final | `REWORK` | candidato concreto com unidade base `16×16 px` |
| `ART-007` | `PROPS` | Conjunto ambiental ainda não seleccionado | Decoração/ambiente sem alterar colisões | `REWORK` | módulos base `16×16 px`; peças maiores em múltiplos de `16 px` |

## Regra de aprovação aplicada

`APPROVE` por defeito quando há tamanho verificável e compatível (`32×32 px` para o player actual; `16×16 px` para a grelha modular e seus múltiplos).

`REWORK` quando o tamanho não está definido/verificável ou quando não corresponde à grelha adoptada.

Esta aprovação é uma decisão visual humana explícita e não promove automaticamente nenhum asset para o runtime. A integração continua dependente das verificações técnicas, provenance e do ficheiro exacto a integrar.

## Evidência e provenance

Os candidatos externos `ART-002` a `ART-005` permanecem sujeitos às referências registadas no `PLATFORM_ASSET_REGISTRY.md`. Para qualquer promoção futura devem permanecer disponíveis:

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

Só depois desta decisão humana positiva e das verificações técnicas/provenance é permitido criar uma alteração de integração. A promoção deve referenciar **o ficheiro exacto** aprovado; candidatos em `REWORK` não devem ser substituídos silenciosamente por outros.

`NO WINNER` continua a ser um resultado válido noutras revisões.

## Regra de evidência concreta

Um ID só é accionável como candidato visual quando existe um ficheiro ou preview concreto que possa ser inspeccionado. Referências bibliográficas, nomes de pacotes ou linhas de registry, por si só, não contam como arte apresentada para aprovação.
