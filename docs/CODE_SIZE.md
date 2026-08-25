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
├── font resource/lifecycle
├── glyph/layout preparation
└── text draw submission
```

```text
main.cpp
├── process lifetime / composition
├── application setup/orchestration
└── delegação para Runtime / Editor / Presentation
```

Exemplos não aceitáveis:

```text
main1.cpp
main2.cpp
main3.cpp
```

ou qualquer divisão por intervalos arbitrários de linhas apenas para fazer a métrica passar.

## Estado da implementação

A política acima é a decisão normativa deste processo. **Ela ainda não corresponde ao checker integrado em `main`**.

Em `main` (`1573e21c518620188ac47568b99d23327f80a279`), `Development/Tools/check_source_sizes.py` usa ainda limites em KiB (`30/36 KiB`) e apenas percorre `Game/` e `Tests/`. `main.cpp` não é atualmente incluído por esse checker.

Logo, a transição deve ser tratada como um work package de tooling/CI. A documentação não deve afirmar que a nova política já está automaticamente imposta até o checker, workflow e validação serem atualizados.

## Incidente de referência — 2026-08-25

A execução histórica #251 identificou:

```text
ERROR   430 linhas  Game/Graphics/FontRenderer.cpp
WARNING 332 linhas  Game/Graphics/SpriteRenderer.cpp
WARNING 305 linhas  Tests/Unit/test_keybindings.cpp
WARNING 326 linhas  Tests/Unit/test_level.cpp
WARNING 330 linhas  main.cpp
```

O primeiro alvo de decomposição é `FontRenderer.cpp`, seguido por investigação de `SpriteRenderer.cpp`, `main.cpp` e os dois ficheiros de testes. A ordem pode mudar apenas por decisão documentada caso uma nova evidência mude o risco.

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
