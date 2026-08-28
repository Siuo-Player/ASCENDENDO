# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

O roadmap orienta a ordem do trabalho, mas uma propriedade só muda de estado quando existe evidência suficiente para a alegação correspondente.

## Gate 9.6 — Base Engineering Gate

O Gate está **READY FOR FORMAL CLOSE** nesta tranche. O estado `CLOSED` só será usado depois de esta decisão ser integrada na `main`.

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
- PR #117 — reconciliação canónica do roadmap/tech debt com os Studies e com a `main` real.

### Estado reconciliado de replay

A evidência atual demonstra replay tick-semantic, comparação de estado por tick e o contrato da fronteira de input em que edges são emitidos apenas no primeiro tick de cada frame, enquanto ações held permanecem disponíveis nos ticks derivados do mesmo frame.

Isto **não** demonstra:

- live-input frame-rate independence para diferentes cadências reais de render/eventos;
- terminal/result replay completo de `GameSession`;
- persistence/serialization de replay.

Estas propriedades são distintas. A revisão de produto não encontrou requisito que as torne pré-condições do Gate 9.6; permanecem capacidades futuras explícitas.

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

Este contrato está caracterizado. Um modelo semântico geral de chunks e schema/versioning permanece Fase 10 salvo novo requisito.

### Vulkan — revisão final

A revisão final confirmou:

- `VulkanContext` exige graphics queue, present queue quando há surface, `VK_KHR_swapchain` e API compatível com 1.3;
- `Swapchain` valida suporte da present queue e consulta surface capabilities, formatos e present modes antes da criação;
- graphics e present podem ser a mesma queue family ou famílias distintas;
- #94 cobre falha de `vkDeviceWaitIdle()`;
- #109 estabelece semântica terminal/fail-closed após falha de `vkQueueSubmit`.

Não foi identificado requisito que justifique uma nova camada genérica de fault injection. O resultado é `reviewed / accepted`, não `every VkResult exhaustively tested`.

### Architecture / ownership — revisão final

`GameSession` permanece como fronteira de estado de sessão sem ownership Vulkan/presentation. `GraphicsRuntime` mantém o ownership agregado do stack gráfico e cleanup em ordem inversa de dependências. `PresentationRuntime` mantém recursos de presentation e attachments não-owning ao `RendererFacade`.

`RendererFacade` ainda recebe `logic::Player`/`logic::Level` diretamente em parte do caminho. A fronteira geral de `RenderSnapshot` permanece incompleta e é dívida arquitetural conhecida, mas não foi identificado um requisito de Gate 9.6 que a torne blocker. Não deve existir uma classe `Application` genérica apenas para completar o diagrama.

### Gate disposition

Os restantes itens são explicitamente classificados:

```text
Vulkan capability/queue/synchronization review       ACCEPTED
Architecture/ownership review                        ACCEPTED
Replay terminal/result                               FUTURE / not Gate requirement
Live-input frame-rate independence                   FUTURE / not Gate requirement
Replay persistence                                   FUTURE / not Gate requirement
Collision universal invariance                       NOT CLAIMED
Level semantic validation/schema                     FASE 10
RenderSnapshot general migration                     NEXT ARCHITECTURE BLOCK
```

### Ordem seguinte

```text
formal Gate 9.6 close
→ dedicated RenderSnapshot work package
→ presentation/domain boundary migration
→ Fase 10 semantic Level/schema/versioning
```

Não iniciar a migração geral de `RenderSnapshot` no mesmo WP do fecho do Gate.

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
