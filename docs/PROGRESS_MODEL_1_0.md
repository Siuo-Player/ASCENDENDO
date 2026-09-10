# ASCENDENDO — Modelo de progresso do produto 1.0

Este documento define como interpretar as percentagens usadas em `docs/ROADMAP.md`.

## Princípio

A percentagem mede **distância ponderada ao produto 1.0**, não volume de código.

Uma área pode estar muito madura tecnicamente e ainda contribuir pouco para o global se houver grandes blocos de produto posteriores por concluir.

## Ordem

```text
Fundação
  ↓
Gameplay
  ↓
Conteúdo
  ↓
Validação
  ↓
Produto
  ↓
Release
```

A existência de trabalho paralelo não altera esta ordem. A fila expressa dependências de produto e o momento em que uma área pode ser declarada estável.

## Métricas de referência

- Fundação: ~95%
- Gameplay: ~85%
- Conteúdo: ~35%
- Validação: ~35%
- Produto: ~50%
- Release: ~10%
- Global: ~55%

Estes valores são aproximações para gestão do projecto. O aumento de uma percentagem exige alteração material de estado, acompanhada da evidência correspondente no roadmap.

## O que não deve inflacionar a percentagem

Não contar duas vezes a mesma implementação entre engine, produto e validação. Não aumentar significativamente o progresso por documentação, refactor interno, fixtures ou protótipos que não façam parte do percurso 1.0. Não tratar “código existente” como sinónimo de “fluxo de utilizador terminado”.

## Critério final

ASCENDENDO chega a 100% apenas quando um utilizador externo consegue percorrer, validar e utilizar o produto suportado desde a entrada no programa até ao conteúdo oficial e à distribuição 1.0, sem depender de tooling interno.
