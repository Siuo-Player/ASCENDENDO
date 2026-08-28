# Work Package — EditorRenderer concrete Camera include

**Tracking:** PR #136 follow-up CI finding

## Context

PR #136 expôs uma falha de compilação em `Game/Graphics/EditorRenderer.cpp`: a implementação instancia `Camera`, enquanto `ShapeRenderer.h` fornece apenas uma forward declaration.

## Evidence

O compilador reportou tipo incompleto em:

```text
const Camera fixedCamera{};
```

A definição concreta deve ser incluída diretamente pelo ficheiro que instancia o tipo.

## Decision

Incluir `Graphics/Camera.h` diretamente em `EditorRenderer.cpp`.

## Scope

- adicionar o include direto;
- nenhuma alteração de comportamento;
- nenhuma alteração a `Camera` ou ownership do renderer.

## Validation

Executar os três workflows obrigatórios Linux normal, Linux ASan/UBSan e Windows.

## Nota de integração

A correção foi aplicada na própria branch do #136 porque o erro bloqueava a validação dessa tranche e o change é mecanicamente independente da alteração de Logic. O finding permanece explicitamente registado como CI follow-up, sem criar um tracker inexistente.
