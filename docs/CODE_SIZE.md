# Regra de tamanho dos ficheiros de código

O ASCENDENDO usa **tamanho físico do ficheiro** como sinal de manutenção e arquitetura. Um tamanho elevado desencadeia análise de coesão, responsabilidades e coupling; não justifica divisão artificial.

## Política normativa

Aplica-se a ficheiros de código C/C++:

`.c`, `.cc`, `.cpp`, `.cxx`, `.h`, `.hh`, `.hpp`, `.hxx`

| Tamanho | Estado | Regra |
|---:|---|---|
| `< 30 KiB` | normal | desenvolvimento normal; continuar a avaliar coesão |
| `30–36 KiB` | warning | não adicionar responsabilidades sem plano de subdivisão |
| `> 36 KiB` | error | CI deve bloquear até subdivisão ou exceção documentada |

A métrica é uma heurística. Um ficheiro de 35 KiB não é automaticamente mau e um ficheiro de 37 KiB não é automaticamente mal arquitetado.

A política usa bytes/KiB em vez de linhas para manter o gate ligado ao tamanho físico que o checker mede e para evitar que comentários ou formatação alterem artificialmente a classificação.

## Exceções

Uma exceção acima de `36 KiB` deve documentar antes da implementação/integração:

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
├── CPU font baking / glyph preparation
├── Vulkan atlas resource management
└── text draw submission / layout
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

A política normativa acima está alinhada com o checker atual: `Development/Tools/check_source_sizes.py` verifica os ficheiros C/C++ sob `Game/` e `Tests/` com limites de `30/36 KiB`.

`main.cpp` só entra no gate se estiver dentro das raízes verificadas pelo checker ou se o checker for explicitamente ampliado numa tranche posterior; a documentação não deve afirmar cobertura que o código ainda não possui.

## Incidente de referência — 2026-08-25

A execução histórica #251 identificou:

```text
ERROR   20.21 KiB   430 linhas  Game/Graphics/FontRenderer.cpp
WARNING 14.36 KiB   332 linhas  Game/Graphics/SpriteRenderer.cpp
WARNING 11.37 KiB   305 linhas  Tests/Unit/test_keybindings.cpp
WARNING 12.61 KiB   326 linhas  Tests/Unit/test_level.cpp
WARNING 13.83 KiB   330 linhas  main.cpp
```

Os valores acima foram reportados pelo workflow histórico; o primeiro alvo de decomposição continua a ser `FontRenderer.cpp`. A ordem pode mudar apenas por decisão documentada caso nova evidência mude o risco.

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
