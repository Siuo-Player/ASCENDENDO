# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

O roadmap orienta a ordem do trabalho, mas uma propriedade só muda de estado quando existe evidência suficiente para a alegação correspondente.

## Gate 9.6 — Base Engineering Gate

O Gate permanece **OPEN**.

### Evidência integrada

- PR #81 — rollback agregado de `GraphicsRuntime` em falha de init.
- PR #87 — `VulkanContext::reconfigureForSurface()` com contrato terminal e estado invalidado após falha.
- PR #85 — Windows build/test evidence em runner real com Vulkan software driver.
- PR #88 — `TickInput` como unidade semântica de input da simulação.
- PR #90 — `ReplayManager` alinhado a `TickInput` para sequência explícita de ticks.
- PR #91 — malformed syntax da gramática atual coberta por testes.

### Gaps ainda bloqueadores

1. Vulkan lower-level failure/error evidence e queue/capability evidence adicional.
2. Deterministic simulation/replay além da representação TickInput; live input sampling e persistence permanecem separadas.
3. Paths/runtime-root independence.
4. Level world/chunk metadata contract.
5. Collision-order determinism.
6. Architecture/ownership final review.

### Ordem de execução

```text
Vulkan/error evidence
→ determinism/replay review
→ paths/runtime roots
→ semantic Level validation (Fase 10)
→ architecture/ownership review
→ Gate decision
→ RenderSnapshot generalization
```

Não avançar para a migração geral de `RenderSnapshot` antes do fecho formal do Gate 9.6.

## Princípios de execução

```text
investigar
→ documentar
→ atualizar roadmap/architecture/tech-debt/WP
→ implementar
→ testar/validar
→ documentar resultado, falhas e próxima decisão
```

A decomposição de `main.cpp` é incremental e baseada em ownership/responsabilidade/testabilidade; não criar uma `Application` genérica apenas para reduzir linhas.
