# Approval form — preenchimento humano

Decisão humana registada em `2026-09-04`: aprovar por defeito candidatos com tamanho verificável e compatível com a escala do projecto; colocar em `REWORK` candidatos sem tamanho verificável ou fora da grelha adoptada.

```text
ART-001: APPROVE — 32×32 px
ART-002: APPROVE — 16×16 px
ART-003: APPROVE — 16×16 px
ART-004: REWORK — requer 16×16 px; 18×18 não integrar
ART-005: APPROVE — 16×16 px para tiles; múltiplos de 16 px para composições
ART-006: REWORK — candidato concreto necessário, unidade base 16×16 px
ART-007: REWORK — candidato concreto necessário, módulos base 16×16 px; peças maiores em múltiplos de 16 px
```

## Regra aplicada

- Player actual: `32×32 px`.
- Tiles/plataformas: `16×16 px`.
- Elementos compostos: dimensões em múltiplos inteiros de `16 px` por eixo.
- Sem tamanho verificável: `REWORK`.
- `18×18 px`: `REWORK` para alinhar com a grelha `16×16 px`.

## Nota

A aprovação/rework acima é uma decisão de revisão visual e de escala. Não altera automaticamente o jogo. A integração de assets aprovados requer ainda o ficheiro exacto, provenance e os restantes gates técnicos.
