# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

O roadmap orienta a ordem do trabalho, mas uma propriedade só muda de estado quando existe evidência suficiente para a alegação correspondente.

## Gate 9.6 — Base Engineering Gate

O Gate permanece **OPEN**.

### Evidência integrada

- PR #76 — `RuntimeBootstrap` como composição de startup, sem `Application` genérica.
- PR #81 — rollback agregado de `GraphicsRuntime` em falha de init.
- PR #87 — `VulkanContext::reconfigureForSurface()` com contrato terminal e estado invalidado após falha.
- PR #85 — Windows build/test evidence em runner real com Vulkan software driver.
- PR #88 — `TickInput` como unidade semântica de input da simulação.
- PR #90 — `ReplayManager` alinhado a `TickInput`.
- PR #92 — malformed syntax da gramática atual coberta por testes; PR #91 foi superseded.
- PR #94 — failure-path evidence específica para `vkDeviceWaitIdle()` em `RendererCore`.
- PR #95 — reconciliação do replay contra o `main` atual.
- PR #99 — licença MIT do projeto e fronteiras de licenciamento de terceiros.
- PR #100 — caracterização por permutação da ordem de contactos de colisão.
- PR #101 — documentação do resultado observado da caracterização de collision-order.
- PR #102 — reconciliação canónica do Gate após #100/#101.
- PR #105 — evidência executável de independência do current working directory para `RuntimePaths::fromProcess(nullptr)` nos ambientes suportados.

### Estado reconciliado de replay

O teste atual demonstra replay tick-semantic e comparação de estado por tick, incluindo invariância da mesma sequência `TickInput` sob agrupamento externo diferente. Isto **não** demonstra live-input frame-rate independence, terminal/result replay completo de `GameSession` ou persistence.

### Collision-order — estado correto

A implementação de produção não foi alterada. O teste de PR #100 executou o mesmo conjunto de contactos sobrepostos em duas ordens de armazenamento e não observou divergência em posição, velocidade ou estado `grounded`, em Linux normal, ASan/UBSan e Windows.

Esta evidência fecha apenas o **cenário exercitado**. Não estabelece invariância universal para todas as combinações de contactos possíveis. Um requisito mais forte ou um counterexample adicional justificaria um novo WP.

### Runtime-root independence — estado correto

PR #105 adicionou teste que executa `RuntimePaths::fromProcess(nullptr)` a partir de dois current working directories temporários diferentes e compara o executable root e os caminhos derivados de assets/levels/sprite. A evidência foi executada através dos três checks obrigatórios.

Esta tranche demonstra a independência de CWD para o comportamento atual de resolução de root do processo nos targets suportados. Não define uma nova política de packaging, bundles ou deployment.

### Gaps ainda bloqueadores

1. Vulkan lower-level failure/error evidence e queue/capability evidence adicional além dos caminhos agora demonstrados.
2. Level world/chunk metadata contract.
3. Revisão final de collision-order policy apenas se surgir requisito/counterexample além do cenário já exercitado.
4. Semantic Level validation/schema/versioning (Fase 10).
5. Architecture/ownership final review.
6. Propriedades de replay separadas que não pertencem ao WP atual: terminal/result replay, persistence e live-input frame-rate independence.

### Ordem de execução

```text
Vulkan/error evidence residual
→ replay review residual
→ world/chunk metadata contract
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
