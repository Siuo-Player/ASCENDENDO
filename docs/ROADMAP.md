# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

O roadmap orienta a ordem do trabalho, mas uma propriedade só muda de estado quando existe evidência suficiente para a alegação correspondente.

## Gate 9.6 — Base Engineering Gate

O Gate está **CLOSED** a partir da integração da revisão final do Gate (PR #118) e da confirmação de fecho formal (PR #119).

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
- PR #101 — documentação do resultado observado de collision-order.
- PR #102 — reconciliação canónica do Gate após #100/#101.
- PR #105 — evidência executável de independência do current working directory para `RuntimePaths::fromProcess(nullptr)` nos ambientes suportados.
- PR #108 — classificação dos failure/error paths Vulkan residuais.
- PR #109 — contrato pós-`vkQueueSubmit`: semântica terminal/fail-closed no design atual.
- PR #113 — caracterização documental do contrato `LevelData → Level → CampaignRuntime`.
- PR #114 — characterization tests desse contrato, sem alterar semântica de produção.
- PR #115 — isolamento dos residuais de replay/input.
- PR #116 — characterization executável da fronteira frame → `TickInput`.
- PR #117 — reconciliação canónica do roadmap/tech debt com os Studies e com a `main` real.
- PR #118 — revisão final de Vulkan capability/queue/synchronization, architecture/ownership e disposição dos claims residuais de replay.
- PR #119 — fecho formal do Gate 9.6.

### Estado reconciliado de replay

A evidência atual demonstra replay tick-semantic, comparação de estado por tick e o contrato da fronteira de input em que edges são emitidos apenas no primeiro tick de cada frame, enquanto ações held permanecem disponíveis nos ticks derivados do mesmo frame.

Isto **não** demonstra:

- live-input frame-rate independence para diferentes cadências reais de render/eventos;
- terminal/result replay completo de `GameSession`;
- persistence/serialization de replay.

Estas são capacidades futuras explícitas e não requisitos do Gate 9.6 segundo a revisão final integrada.

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

A revisão final confirmou as assumptions de queue/capability/synchronization e os contratos de failure relevantes. Não foi identificado requisito que justifique uma camada genérica de fault injection.

### Architecture / ownership — revisão final

`GameSession` permanece como fronteira de estado de sessão sem ownership Vulkan/presentation. `GraphicsRuntime` mantém o ownership agregado do stack gráfico e cleanup em ordem inversa de dependências. `PresentationRuntime` mantém recursos de presentation e attachments não-owning ao `RendererFacade`.

## Next Architecture Block — RenderSnapshot

**Issue:** #122  
**WP:** `docs/05-work-packages/WORK_PACKAGE_RENDERSNAPSHOT_BOUNDARY_2026-08-28.md`  
**Estado:** **FIRST TRANCHE CONCLUÍDA (PR #129)**

Objetivo da tranche: separar dados de presentation do domínio no world/player path. O snapshot contém somente dados necessários para os passes migrados, sem `logic::Player`, `logic::Level`, `logic::Vec2`, `Camera`, recursos Vulkan ou ownership.

A sequência executada foi:

```text
inventariar consumidores
→ definir contrato mínimo
→ construir snapshot explícito
→ migrar WorldRenderer
→ adaptar RendererFacade
→ caracterizar/testar
→ validar CI
→ concluir primeira tranche
```

### Evidência da primeira tranche

- `Game/Graphics/RenderSnapshot.h` existente foi reutilizado como único contrato.
- `RenderSnapshotBuilder` passou a constituir a fronteira explícita runtime → presentation.
- `WorldRenderer` passou a consumir `RenderSnapshot` no world/player path, sem consultar `Player`/`Level` mutáveis.
- `RendererFacade` passou a receber o snapshot nesse caminho.
- `main.cpp` constrói o snapshot na composição do frame.
- Testes cobrem composição, geometria visual, direção visual e independência perante alterações posteriores do runtime.
- PR #129 foi integrado após os três workflows obrigatórios passarem.

### Contrato atual da tranche

```text
RenderSnapshot
├── player { bounds, facingDirection }
├── platforms[] { x, y, width, height }
└── flag { visible, x, y, width, height }
```

A `Camera` permanece separada como objeto de presentation; não faz parte do snapshot.

### Fora de escopo

- replay;
- schema/versioning;
- physics/gameplay;
- `Application` genérica;
- migração automática de todos os passes;
- otimização de baixo nível sem baseline.

## Next Modularity Block — Shared Vulkan image upload

**Issue:** #23  
**Branch:** `refactor/shared-vulkan-image-upload-20260828`  
**Estado:** **READY FOR IMPLEMENTATION**

### Descoberta

`FontRendererGpu.cpp` e `SpriteRendererGpu.cpp` duplicam a mesma sequência de staging, memory allocation, image creation/binding, one-time command submission, image layout transitions, copy-to-image, image view, sampler e cleanup. A duplicação existe mesmo sem o ficheiro atingir o hard limit de 48 KiB.

### Decisão

Escolhida a opção **B — primitive estreito de upload/creation de imagem Vulkan**. O primitive não será um `TextureManager`, não terá cache global e não absorverá descriptor policy.

```text
FontRendererGpu ─┐
                 ├→ shared Vulkan image upload primitive
SpriteRendererGpu┘
```

`format` e `filter` permanecem parâmetros explícitos. O consumer continua proprietário dos recursos retornados e do descriptor setup específico.

### Dependências

- `VulkanContext`;
- `FontRendererGpu`;
- `SpriteRendererGpu`;
- #22 `FontRenderer decomposition` como dependente posterior;
- documentação de modularidade e source-size.

### Validação

```text
baseline / comportamento atual
→ primitive + failure cleanup
→ FontRendererGpu
→ SpriteRendererGpu
→ game build + tests
→ sanitizers
→ Windows
→ documentação final
```

A abstração deve ser abandonada/revista caso comece a acumular políticas específicas não demonstradas pelos dois consumidores atuais.

## Fase 10

Semantic `LevelData` validation/schema/versioning permanece separado do bloco de presentation e só avança quando a evidência/requisito correspondente o justificar.

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