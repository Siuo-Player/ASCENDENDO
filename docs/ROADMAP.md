# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

O roadmap orienta a ordem do trabalho, mas uma propriedade só muda de estado quando existe evidência suficiente para a alegação correspondente.

## Gate 9.6 — Base Engineering Gate

O Gate permanece **OPEN**.

### Evidência integrada

- PR #81 — rollback agregado de `GraphicsRuntime` em falha de init.
- PR #87 — `VulkanContext::reconfigureForSurface()` com contrato terminal e estado invalidado após falha.
- PR #85 — Windows build/test/runtime evidence em runner real com Vulkan software driver.
- PR #88 — `TickInput` como unidade semântica de input da simulação.
- PR #90 — `ReplayManager` alinhado a `TickInput`; reprodução por sequência explícita de ticks.
- PR #91 — evidência de malformed syntax para a gramática atual de `LevelDataIO`.

### Gaps restantes

```text
Vulkan lower-level failure/error evidence
queue/capability evidence adicional
live input sampling / replay boundary review
paths/runtime roots
Level world/chunk metadata
collision-order determinism
architecture/ownership final review
```

### Ordem

```text
Vulkan/error evidence
→ determinism/replay review
→ paths/runtime roots
→ semantic Level validation (Fase 10)
→ architecture/ownership review
→ Gate decision
→ RenderSnapshot generalization
```

Não iniciar a migração geral de `RenderSnapshot` antes do fecho formal do Gate.

## Regras permanentes

```text
investigar
→ documentar
→ atualizar arquitetura/roadmap/tech debt/WP
→ implementar
→ testar/validar
→ documentar resultado, falhas e próxima decisão
```

A decomposição de `main.cpp` é incremental e baseada em ownership/responsabilidade/testabilidade; não criar `Application` genérica apenas para reduzir tamanho.

`RuntimeBootstrap` é composição de startup.
`LevelDataIO` é parser/serializer até existir schema e semantic validation explícitos.
`ReplayManager` opera sobre `TickInput`; isso não prova independência do live input face ao frame rate.
