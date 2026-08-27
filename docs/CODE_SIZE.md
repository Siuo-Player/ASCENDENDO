# Regra de tamanho dos ficheiros de código

O ASCENDENDO usa o tamanho físico do ficheiro como **critério objetivo para decidir quando uma responsabilidade deve ser repartida**, tendo como finalidade principal tornar a manutenção e o diagnóstico mais localizados. O tamanho, por si só, não determina onde dividir: a divisão tem de seguir coesão, ownership, dependências e responsabilidades reais.

## Política normativa

Aplica-se a ficheiros de código C/C++:

`.c`, `.cc`, `.cpp`, `.cxx`, `.h`, `.hh`, `.hpp`, `.hxx`

A métrica normativa é o **tamanho físico em KiB**. A contagem de linhas pode aparecer como diagnóstico, mas nunca decide o estado do gate.

| Tamanho físico | Estado | Regra |
|---:|---|---|
| `< 40 KiB` | normal | desenvolvimento normal; continuar a avaliar coesão |
| `40–47.99 KiB` | warning | iniciar avaliação explícita de particionamento antes de acumular novas responsabilidades |
| `>= 48 KiB` | error | CI deve bloquear até existir particionamento coerente ou exceção documentada |

Os limiares são bytes físicos (`1 KiB = 1024 bytes`). Medir o peso real do artefacto evita que formatação, comentários ou densidade de linhas distorçam o critério físico.

## Métricas complementares

LOC continua útil para diagnóstico e revisão humana, mas não é uma política de bloqueio. Complexidade, coupling, número de responsabilidades e coesão devem ser avaliados separadamente.

Assim:

```text
peso físico do ficheiro  → KiB → decisão de particionamento
estrutura do código      → revisão arquitetural → onde dividir
complexidade             → métricas próprias
LOC                      → diagnóstico auxiliar
```

O objetivo do limite é **localizar responsabilidades, manutenção e diagnóstico**. Não é minimizar o número de linhas nem produzir ficheiros pequenos por si só.

## Exceções

Uma exceção a `>= 48 KiB` deve documentar antes da implementação/integração:

```text
por que o ficheiro permanece coeso
coupling/dependências relevantes
alternativas de divisão consideradas
custo/risco da divisão agora
condição para reconsiderar
```

A exceção pertence ao work package/PR que mantém o ficheiro grande e deve ter condição explícita de revisão.

## Subdivisão

Dividir por **responsabilidade coesa**, ownership e direção das dependências.

Exemplos aceitáveis:

```text
FontRenderer
├── CPU font preparation / metrics
├── GPU atlas resource ownership
└── text draw submission
```

```text
SpriteRenderer
├── CPU image decode / sprite preparation
├── GPU image resource ownership
└── sprite draw submission
```

```text
main.cpp
├── process lifetime / composition
├── runtime state orchestration
└── delegation to runtime/editor/presentation components
```

Exemplos não aceitáveis:

```text
main1.cpp
main2.cpp
main3.cpp
```

ou qualquer divisão por intervalos arbitrários de linhas apenas para fazer a métrica passar.

## Implementação

`Development/Tools/check_source_sizes.py` é o **checker canónico** usado pelo CI.

Os limiares normativos são:

```text
WARNING_BYTES    = 40 * 1024
HARD_LIMIT_BYTES = 48 * 1024
```

`Development/check_source_size.py` permanece apenas como entry point de compatibilidade e delega para o checker canónico; não existe uma segunda implementação da política.

O CI executa ainda `Tests/Tooling/test_check_source_sizes.py`, que verifica os limiares de 40/48 KiB antes de executar o gate sobre o repositório real.

## Incidente de referência — 2026-08-25/26

A implementação anterior usava linhas como métrica normativa e bloqueou `main.cpp` ao atingir 402 linhas. Isso revelou uma inconsistência de tooling: o repositório já possuía um checker em KiB com limites de 40/48 KiB, mas o CI executava outro checker baseado em LOC.

A correção não é comprimir `main.cpp` para ficar abaixo de um contador arbitrário. A política agora tem uma única unidade física (KiB) e uma única implementação normativa.

## Relação com WBS

Um warning/error de tamanho que exigir mudança estrutural deve produzir um work package com:

```text
Objetivo
Escopo / fora de escopo
Dependências / consumidores
Riscos
Alternativas
Validação
Exit criteria
Dívida criada
Próximo bloco dependente
```

Consultar `docs/DEVELOPMENT_PROTOCOL.md` e `docs/PROJECT_MANAGEMENT.md` antes de iniciar a subdivisão.
