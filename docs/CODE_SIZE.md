# Regra de tamanho dos ficheiros de código

O ASCENDENDO usa tamanho de ficheiro como **sinal de manutenção e arquitetura**, não como objetivo de produtividade. Um tamanho elevado desencadeia análise de coesão, responsabilidades e coupling; não justifica divisão artificial.

## Política normativa

Aplica-se a ficheiros de código C/C++:

`.c`, `.cc`, `.cpp`, `.cxx`, `.h`, `.hh`, `.hpp`, `.hxx`

| Linhas | Estado | Regra |
|---:|---|---|
| `< 300` | normal | desenvolvimento normal; continuar a avaliar coesão |
| `300–399` | warning | não adicionar responsabilidades sem plano de subdivisão |
| `>= 400` | error | CI deve bloquear até subdivisão ou exceção documentada |

A métrica é uma heurística. Um ficheiro com 399 linhas não é automaticamente bom e um ficheiro com 401 linhas não é automaticamente mal arquitetado.

## Exceções

Uma exceção a `>= 400` deve documentar antes da implementação/integração:

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
└── delegation to application/runtime/editor/presentation components
```

Exemplos não aceitáveis:

```text
main1.cpp
main2.cpp
main3.cpp
```

ou qualquer divisão por intervalos arbitrários de linhas apenas para fazer a métrica passar.

## Estado da implementação

A política normativa está agora alinhada com o checker de `main` por meio de uma mudança na branch `chore/9-6-base-engineering-gate`:

- a unidade de medida é **linhas físicas de código**, não KiB;
- `main.cpp` é explicitamente incluído;
- os mesmos limiares `<300`, `300–399`, `>=400` são usados pelo checker e por esta documentação;
- CI continua a executar `Development/Tools/check_source_sizes.py` antes do restante da validação.

Esta branch transforma uma inconsistência de tooling em uma regra executável. O resultado concreto de cada ficheiro deve ser obtido no CI da branch; avisos não equivalem a falhas e devem permanecer acompanhados por work packages quando exigirem intervenção estrutural.

## Incidente de referência — 2026-08-25

A execução histórica #251 identificou:

```text
ERROR   430 linhas  Game/Graphics/FontRenderer.cpp
WARNING 332 linhas  Game/Graphics/SpriteRenderer.cpp
WARNING 305 linhas  Tests/Unit/test_keybindings.cpp
WARNING 326 linhas  Tests/Unit/test_level.cpp
WARNING 330 linhas  main.cpp
```

Os dois primeiros alvos já foram investigados e decompostos por fronteira de responsabilidade coesa. Os avisos restantes passam a ser tratados como trabalho de modularidade/testabilidade, não como motivo para alterar a métrica.

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
