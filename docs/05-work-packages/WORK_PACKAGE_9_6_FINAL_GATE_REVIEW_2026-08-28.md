# Work Package — final Gate 9.6 review

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsistema:** `Infrastructure / Runtime / Presentation`
**Work Package:** `9.6 final capability, ownership and Gate disposition review`
**Branch:** `docs/9-6-final-gate-review-20260828`
**PR:** `#118`

## Objetivo

Revisar as evidências já integradas e determinar, com critérios explícitos, se o Gate 9.6 ainda possui blockers reais ou se as incertezas restantes são decisões/capacidades futuras fora do Gate.

## Escopo

### Inclui

- revisão de capability/queue/synchronization assumptions de Vulkan;
- revisão de ownership e fronteiras arquiteturais atuais;
- disposição explícita das propriedades residuais de replay;
- confirmação da matriz mínima de CI e da política `Protect main`;
- decisão formal sobre o estado do Gate 9.6.

### Não inclui

- migração geral de `RenderSnapshot`;
- nova fault-injection framework Vulkan;
- implementação de replay persistence;
- implementação de live-input frame-rate independence;
- schema/versioning ou semantic validation de `LevelData`;
- otimização de performance sem baseline medida.

## Dependências

### Depende de

- PRs #76, #81, #85, #87, #88, #90, #92, #94, #95, #99, #100, #101, #102, #105, #108, #109, #113, #114, #115, #116 e #117;
- `PROJECT-STUDIES/ASCENDENDO` como fonte consultiva;
- `docs/ROADMAP.md`, `docs/TECH_DEBT.md`, `docs/ARCHITECTURE.md` e `docs/PRODUCT_DECISIONS.md`.

### Produz para

- decisão de fecho do Gate 9.6;
- próximos blocos de modularidade e presentation.

### Consumidores afetados

- roadmap;
- technical debt;
- architecture;
- future modularity work packages.

### Dependências de validação

- Linux normal;
- Linux ASan/UBSan;
- Windows/Clang;
- observação dos contratos Vulkan e ownership no código atual.

## Decisões arquiteturais

```text
Problema/contexto:
A arquitetura de presentation não possuía uma fronteira RenderSnapshot completa, enquanto documentos históricos podiam sugerir que essa ausência era um blocker automático do Gate.

Decisão:
O Gate 9.6 foi fechado depois de demonstrados os seus requisitos. RenderSnapshot e outras capacidades futuras ficaram como blocos independentes, cada um com evidência própria.

Alternativas consideradas:
1. adiar o Gate até completar as refatorações;
2. fechar o Gate e tratar as refatorações como dívida explícita.

Consequências:
A progressão fica baseada no contrato real do Gate, evitando transformar arquitetura futura em requisito implícito.

Condição de revisão/remoção:
Reabrir o finding apenas se surgir requisito concreto que torne uma capacidade posterior necessária para uma claim do Gate.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| Snapshot/documento histórico fica stale | média | médio | fonte GitHub atual + audit dated | mitigado |
| Claim de replay é sobredeclarada | média | alto | separar propriedades | mitigado |
| Coverage Vulkan vira checkbox de fault injection | média | médio | revisar contratos, não branches | mitigado |
| RenderSnapshot vira blocker artificial | baixa | alto | disposição explícita no Gate | mitigado |

## Validação

### Testes automatizados

Os testes já integrados nas PRs referenciadas constituem a evidência executável do Gate. Para o head de PR #118, os três workflows obrigatórios terminaram com sucesso:

- `Linux / Clang / C++20 / Headless Vulkan`;
- `Linux / Clang / ASan + UBSan / Headless Vulkan`;
- `Windows / Clang / C++20`.

### Validação manual

Foi revista a ownership graph de `main.cpp`, `GraphicsRuntime`, `VulkanContext`, `Swapchain`, `PresentationRuntime` e `RendererFacade`, juntamente com a documentação canónica atual.

### Profiling / métricas

Nenhuma otimização foi proposta nesta tranche; profiling não é critério de saída.

### Failure paths

Foram revistos explicitamente:

- `vkDeviceWaitIdle()`;
- `vkQueueSubmit()`;
- queue-family/present support;
- swapchain surface capabilities;
- GraphicsRuntime aggregate rollback.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [x] capability/queue/synchronization assumptions revistas;
- [x] ownership/architecture revista;
- [x] claims de replay explicitamente classificadas como Gate ou futuras;
- [x] roadmap/tech debt atualizados;
- [x] evidência final documentada;
- [x] decisão de Gate demonstrada;
- [x] PR #118 mergeada sem trabalho essencial oculto;
- [x] posterior RenderSnapshot/editor boundary já integrada sem reabrir o Gate.

## Alterações durante a execução

```text
Descoberta:
Os Studies datados de 2026-08-28 ficaram stale relativamente ao upstream posterior.

Impacto:
A leitura dos Studies não pode substituir a reconciliação com a main atual.

Decisão tomada:
Este WP usa o GitHub atual como estado operacional e os Studies como fonte consultiva/metodológica.

Resultado adicional:
A main integrou PR #129 (world/player RenderSnapshot), PR #132 (remoção do acoplamento RendererFacade → EditorSession) e respetiva documentação.

Documentos atualizados:
ROADMAP / TECH_DEBT / ARCHITECTURE / DEVELOPMENT_PROTOCOL / BRANCH_PLAN
```

## Evidência / referências

- `Siuo-Player/Siuo-Player-PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_LATEST.md` — snapshot consultivo, agora parcialmente stale;
- `ROADMAP`, `TECH_DEBT`, `ARCHITECTURE`, `DEVELOPMENT_PROTOCOL` atuais;
- PR #118, #119, #129 e #132;
- workflows obrigatórios dos heads validados.

## Fecho

**Resultado:** `CLOSED`
**Critério de saída:** todas as claims do Gate têm estado explícito e nenhuma capacidade posterior é tratada como blocker sem requisito.
**Dívida residual:** replay persistence/live-input independence, LevelData semantic/schema work e modularidade de infraestrutura não necessária para o Gate.
**Próximo bloco:** `D.0 — shared Vulkan image upload primitive (#23)`