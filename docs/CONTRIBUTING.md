# Workflow de desenvolvimento

## Branches

`main` deve permanecer integrado e utilizável.

Cada passo de trabalho começa numa branch nova criada a partir do `main` já atualizado. O nome deve indicar o objetivo:

```text
fix/<problema>
feat/<fase>-<objetivo>
docs/<objetivo>
test/<objetivo>
```

A branch do passo anterior deixa de ser a branch de trabalho depois de a sua PR ser integrada. Quando a ferramenta permitir apagar refs, a branch antiga deve ser eliminada; caso contrário, fica abandonada e não volta a receber alterações.

## PR obrigatório por passo

Cada passo tem uma PR própria ligada à Issue canónica correspondente. A PR deve conter:

- objetivo;
- comportamento esperado;
- ficheiros principais;
- testes realizados;
- limitações conhecidas;
- referência aos ADRs ou registos de sessão aplicáveis.

Não misturar uma feature de uma fase com trabalho não relacionado de outra fase.

## Memória contínua e documentação viva

O repositório utiliza o padrão `ai-memory` (`docs/memory/`) para continuidade operacional entre sessões e agentes:

- **Decisões de arquitetura**: Registar ADRs em `docs/memory/decisions/` (ADR-XXXX) para decisões técnicas e arquiteturais estáveis.
- **Armadilhas e gotchas**: Registar gotchas de compilação, bugs subtis de SO ou física em `docs/memory/gotchas/`.
- **Sessões de trabalho**: Registar o resumo da sessão em `docs/memory/sessions/YYYY-MM-DD-<descricao>.md`, indicando tarefas concluídas, testes executados e pendências.
- **Sem snapshots datados**: Não criar snapshots datados `STATUS_*`, `CURRENT_STATUS_*` ou `ROADMAP_*`; `docs/ROADMAP.md` é a única fonte viva de planeamento.

## TDD e testes

Para lógica determinística, escrever primeiro os testes. Features gráficas devem separar lógica verificável sem GPU da integração Vulkan.

Uma PR não deve ser considerada concluída por apenas compilar. Deve ter os testes relevantes executados e, quando aplicável, a campanha validada.

## Merge

Antes de começar o passo seguinte:

1. terminar a branch corrente;
2. validar localmente (testes C++ e suites Python);
3. abrir PR ligada à Issue canónica;
4. aguardar validação verde de todos os workflows do CI;
5. integrar PR em `main`;
6. registar resumo em `docs/memory/sessions/`;
7. criar a nova branch a partir do commit integrado em `main`.

Este processo mantém cada PR pequena, reversível e auditável.
