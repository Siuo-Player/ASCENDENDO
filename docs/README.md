# Documentação técnica do ASCENDENDO

Esta pasta é organizada por **função documental**, não por uma coleção de fases ou branches históricas.

## Fontes normativas

- [Roadmap](ROADMAP.md) — **fonte única da ordem de execução**.
- [00-meta](00-meta/README.md) — processo, gestão, decisões de produto e continuidade.
- [01-architecture](01-architecture/README.md) — arquitetura, ownership, dívida e fronteiras.
- [02-engineering](02-engineering/README.md) — CI, testes e políticas de manutenção.

## Domínio do jogo

- [03-gameplay-editor](03-gameplay-editor/README.md) — níveis, editor, campanhas e UX.
- [04-research](04-research/README.md) — papers, referências técnicas e investigação.
- [05-work-packages](05-work-packages/README.md) — contratos de implementação por tranche.

## Histórico

- [99-history](99-history/README.md) — snapshots, changelogs e evidência histórica específica.

## Regra de leitura antes de uma branch

1. `ROADMAP.md`
2. `docs/00-meta/` — processo e decisões
3. `docs/01-architecture/` — fronteiras e dívida
4. `docs/04-research/` + `Siuo-Player/Siuo-Player-PROJECT-STUDIES/ASCENDENDO` — evidência externa/consultiva
5. work package relevante

## Regra de manutenção

Os ficheiros que ainda estão no topo de `docs/` não foram renomeados nesta tranche: o objetivo foi criar uma classificação estável **sem quebrar links existentes nem alterar conteúdo**. A migração física dos ficheiros será feita depois, em bloco mecânico e verificável.

O `PROJECT-STUDIES/ASCENDENDO` é **somente leitura** a partir deste projeto.

Cada descoberta material deve ser documentada antes da implementação, a validação deve produzir evidência observável e a documentação deve refletir o resultado real.