# Plano da branch atual

**Branch:** `chore/ci-concise-logs`

**Bloco do roadmap:** 9.6 Base Engineering Gate — CI observability / evidence discipline

**Work Package:** CI output reduction + documentation of new difficulty research

## Objetivo

1. Reduzir drasticamente o ruído normal do workflow de CI sem esconder informação útil quando ocorre uma falha.
2. Registar imediatamente a nova evidência científica sobre dificuldade automática e player modelling.
3. Atualizar roadmap, decisões de produto e índice de investigação antes de qualquer implementação futura de difficulty analysis.

## Escopo

- workflow GitHub Actions;
- política de logs de sucesso/falha;
- `SCIENTIFIC_REFERENCES.md`;
- `RESEARCH_INDEX.md`;
- `PRODUCT_DECISIONS.md`;
- `ROADMAP.md`;
- este plano de branch.

## Fora de escopo

- implementar um difficulty score;
- alterar a física;
- alterar a geração ou geometria dos níveis;
- implementar adaptive difficulty;
- alterar `RenderSnapshot`;
- subdividir `SpriteRenderer`/`main.cpp`.

## Evidência nova

**Francillette et al. (2025)** — *A Comprehensive Model of Automated Evaluation of Difficulty in Platformer Games*, DOI `10.1145/3705013`.

A contribuição relevante é separar dificuldade game-centric/executiva de experiência do jogador e propor avaliação automática através de zonas de perigo estáticas e perigos dinâmicos. É evidência para uma camada de análise do conteúdo, não prova de que as fórmulas/thresholds sejam válidos para o ASCENDENDO.

**Madineni (2025)** — *Analyzing Player Difficulty Perception in Platformers Through Procedural Level Generation*.

A dissertação estuda ajuste de dificuldade em tempo real e encontrou desempenho menos consistente para jogadores menos confortáveis com platformers. É evidência preliminar para a futura fase de player modelling, não uma política universal de adaptive difficulty.

## Dependências

**Depende de:**

- CI atual validado em `main`;
- física determinística;
- validação simulada existente;
- modelo de nível e replay/telemetria futuros.

**Produz para:**

- futura análise de dificuldade executiva;
- player modelling;
- eventual adaptive difficulty;
- documentação científica do projeto.

## Decisão arquitetural preliminar

```text
collision geometry + deterministic physics
                    ↓
             physics validity
                    ↓
       motor/executive difficulty
                    ↓
          player performance
                    ↓
        perceived difficulty
```

A primeira camada nova deve ser de **análise/diagnóstico**. Não deve alterar silenciosamente a física ou o conteúdo authored.

## Política de logs do CI

Sucesso normal deve produzir apenas:

- progresso de alto nível por fase;
- confirmação do source-size gate;
- confirmação do Vulkan driver;
- resumo do número de testes/assertions;
- resultado da validação da campanha.

Em falha, o workflow deve imprimir o **tail do log da operação que falhou**, preservando o diagnóstico do compilador/test runner sem imprimir dezenas de milhares de linhas de output normal.

O run #302 validou a base depois da troca de `tests-verbose` para `tests`: source-size ✅, Vulkan ✅, build/tests ✅, campanha ✅; 167 testes e 901 assertions passaram.

## Riscos

- esconder demasiado output e dificultar diagnóstico;
- tratar uma métrica académica como contrato do jogo sem calibração;
- confundir dificuldade executiva com dificuldade percebida;
- iniciar player modelling antes de existir telemetria suficientemente rica.

## Definition of Ready

- [x] objetivo definido;
- [x] escopo incluído/excluído definido;
- [x] dependências conhecidas;
- [x] riscos identificados;
- [x] evidência científica investigada;
- [x] limitações da evidência registadas;
- [x] documentação de processo consultada;
- [x] base `main` validada antes de abrir a branch.

## Critério de saída

```text
CI normal substantially shorter
+ failures retain actionable diagnostics
+ new research is documented with provenance and limitations
+ PRODUCT_DECISIONS reflects the product boundary
+ ROADMAP contains WBS/dependencies for the future difficulty axis
+ no gameplay/difficulty implementation has been smuggled into this tranche
+ workflow passes after the change
```

## Próximo work package

Só depois de esta branch integrar em `main` e os testes passarem novamente:

**9.6 Base Engineering Gate — primeiro work package ainda pendente**, conforme o estado real do roadmap.

O novo eixo de difficulty fica documentado como trabalho futuro; não está autorizado a saltar diretamente para implementação antes de cumprir a Definition of Ready própria.
