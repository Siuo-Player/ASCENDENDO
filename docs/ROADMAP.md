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
- PR #108 — classificação dos failure/error paths Vulkan residuais.
- PR #109 — contrato pós-`vkQueueSubmit`: semântica terminal/fail-closed no design atual.
- PR #113 — caracterização documental do contrato `LevelData → Level → CampaignRuntime`.
- PR #114 — characterization tests desse contrato, sem alterar semântica de produção.
- PR #115 — isolamento dos residuais de replay/input.
- PR #116 — characterization executável da fronteira frame → `TickInput`.

### Estado reconciliado de replay

A evidência atual demonstra replay tick-semantic, comparação de estado por tick e o contrato da fronteira de input em que edges são emitidos apenas no primeiro tick de cada frame, enquanto ações held permanecem disponíveis nos ticks derivados do mesmo frame.

Isto **não** demonstra:

- live-input frame-rate independence para diferentes cadências reais de render/eventos;
- terminal/result replay completo de `GameSession`;
- persistence/serialization de replay.

Estas são capacidades/propriedades distintas e não devem ser colapsadas em uma única claim de “replay completo”.

### Collision-order — estado correto

A implementação de produção não foi alterada. O teste de PR #100 executou o mesmo conjunto de contactos sobrepostos em duas ordens de armazenamento e não observou divergência em posição, velocidade ou estado `grounded`, em Linux normal, ASan/UBSan e Windows.

Esta evidência fecha apenas o **cenário exercitado**. Não estabelece invariância universal para todas as combinações de contactos possíveis. Um requisito mais forte ou um counterexample adicional justificaria um novo WP.

### Runtime-root independence — estado correto

PR #105 adicionou teste que executa `RuntimePaths::fromProcess(nullptr)` a partir de dois current working directories temporários diferentes e compara o executable root e os caminhos derivados de assets/levels/sprite. A evidência foi executada através dos três checks obrigatórios.

Esta tranche demonstra a independência de CWD para o caminho de resolução de root do processo atualmente exercitado nos targets suportados. Não define uma nova política de packaging, bundles ou deployment.

### World/chunk metadata — estado correto

PRs #113/#114 já caracterizaram e protegeram o comportamento atual:

- `LevelData` representa dados locais do chunk;
- `offsetY` é aplicado durante a composição;
- a geometria acumula em `Level`;
- a metadata `flag` é singular e pode ser sobrescrita pelo chunk seguinte;
- `spawnPosition` é lido no `LevelData`, mas não é materializado como metadata persistente de `Level`.

Este contrato está agora **caracterizado**, mas não é um modelo semântico geral de chunks. Schema/versioning e um modelo mais forte de metadata permanecem Fase 10 salvo novo requisito.

### Vulkan — estado reconciliado

O bloco residual já não deve ser tratado como “todos os `VkResult` ainda precisam de fault injection”.

- #94 fornece evidência executável para `vkDeviceWaitIdle()`.
- #108 classificou os restantes paths por contrato.
- #109 estabeleceu que falha de `vkQueueSubmit` após reset do fence é terminal/fail-closed no design atual: o frame falhado não é recuperado em processo e o runtime abandona o loop.

Permanece necessária uma **revisão final de capability/queue/synchronization assumptions**, não uma cobertura artificial de cada branch de erro.

### Gaps ainda bloqueadores do Gate

1. Revisão final de Vulkan capability/queue/synchronization assumptions e limites da evidência atual.
2. Revisão final de architecture/ownership após as mudanças já integradas.
3. Disposição explícita das propriedades de replay que são ou não requisitos do Gate: terminal/result e live-input frame-rate independence.
4. Decisão formal de fecho do Gate 9.6.

Ficam deliberadamente fora do Gate, salvo requisito novo:

- semantic Level validation/schema/versioning (Fase 10);
- replay persistence/serialization;
- generalização de `RenderSnapshot` antes do fecho do Gate.

### Ordem de execução

```text
revisão final Vulkan capability/queue/synchronization
→ revisão final architecture/ownership
→ disposição explícita dos residuais de replay
→ Gate 9.6 decision
→ RenderSnapshot generalization
→ Fase 10 semantic Level/schema/versioning
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
