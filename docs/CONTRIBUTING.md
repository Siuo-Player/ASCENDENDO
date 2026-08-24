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

Cada passo tem uma PR própria. A PR deve conter:

- objetivo;
- comportamento esperado;
- ficheiros principais;
- testes realizados;
- limitações conhecidas;
- ligação para a documentação da branch.

Não misturar uma feature de uma fase com trabalho não relacionado de outra fase.

## Documentação por branch

Antes da implementação principal, atualizar `docs/BRANCH_PLAN.md` na própria branch com:

- base commit/estado atual;
- o que a branch herdou da anterior;
- plano concreto desta branch;
- critérios de aceitação;
- trabalho explicitamente deixado para depois.

Ao abrir a branch seguinte, considerar o plano anterior encerrado e escrever o novo estado real, evitando transportar "pendentes" antigos sem os classificar.

## TDD e testes

Para lógica determinística, escrever primeiro os testes. Features gráficas devem separar lógica verificável sem GPU da integração Vulkan.

Uma PR não deve ser considerada concluída por apenas compilar. Deve ter os testes relevantes executados e, quando aplicável, a campanha validada.

## Merge

Antes de começar o passo seguinte:

1. terminar a branch corrente;
2. validar;
3. abrir PR;
4. integrar PR em `main`;
5. criar a nova branch a partir do commit integrado;
6. atualizar a documentação da nova branch;
7. só depois implementar.

Este processo mantém cada PR pequena, reversível e auditável.