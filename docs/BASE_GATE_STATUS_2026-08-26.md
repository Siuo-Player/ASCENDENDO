# Base Engineering Gate — estado real — 2026-08-26

## Fonte de estado

Este documento é uma fotografia operacional da `main` em 2026-08-26 e não substitui `docs/ROADMAP.md`. Existe para impedir que o estado histórico das branches seja confundido com o estado atual durante esta tranche.

## Estado confirmado

`main` já integrou os blocos de hardening e modularidade seguintes:

- input por ações e fixed timestep defensivo;
- Level Editor com contrato de viewport `640x360`;
- lifecycle/recriação de swapchain e distinção de graphics/present queue;
- remoção do renderer legado e do adapter de migração;
- contrato inicial de `RenderSnapshot`;
- decomposição de `FontRenderer`;
- decomposição de `SpriteRenderer`.

## Fragilidades que permanecem relevantes

### P0/P1 de engenharia

1. A política de source-size documentada por linhas não correspondia ao checker real, que usava KiB e omitia `main.cpp`.
2. A composição em `main.cpp` continua concentrando inicialização, state machine, simulação, campanha, editor e presentation.
3. Alguns ficheiros de testes são grandes e devem ser avaliados por coesão/testabilidade antes de qualquer nova responsabilidade.
4. A workflow atual demonstra Linux/Clang/headless Vulkan e valida campanha, mas ainda não constitui a matriz de Windows + sanitizers + hardware prevista no roadmap.
5. A apresentação de gameplay ainda depende diretamente de modelos de domínio; o `RenderSnapshot` geral permanece intencionalmente bloqueado pelo Gate.
6. Paths de assets e user data continuam parcialmente ancorados na source tree e devem ser separados antes da portabilidade/distribuição.

### Qualidade do conteúdo / validação

7. O validator formal não equivale a avaliação de execução prática: reachability analítica e dificuldade/robustez motora são propriedades diferentes.
8. Qualidade, diversidade e controllability da geração devem permanecer métricas observáveis separadamente de qualquer score agregado.
9. Um único agente automático não deve ser tratado como substituto universal do jogador humano.
10. Dificuldade percebida e experiência do jogador ainda não são propriedades validadas empiricamente pelo projeto.

## Decisão desta tranche

Antes de continuar `RenderSnapshot`, corrigir a inconsistência do source-size tooling e transformar o Gate em um processo verificável:

```text
property
→ test/evidence
→ environment
→ result
→ artifact
```

Não alterar a política apenas para fazer os ficheiros passarem.

## Ordem imediata

```text
base-gate tooling
    ↓
large test files: cohesion/testability review
    ↓
main.cpp decomposition by responsibility
    ↓
Gate review
    ↓
RenderSnapshot general migration
```

O aparecimento de uma falha factual em qualquer etapa antecipa a sua correção antes do trabalho seguinte; uma nova abstração não deve ser introduzida apenas porque aparece no diagrama do roadmap.

## Relação com Project Studies

A auditoria de `Siuo-Player/Siuo-Player-PROJECT-STUDIES/ASCENDENDO` é usada como fonte consultiva somente. As conclusões relevantes para esta tranche são: separar feature completion de robustness verification, exigir differential evidence entre C++/Python quando ambos são usados para validação, tratar CI cross-platform como evidência ainda incompleta e não confundir validade formal com execução prática.
