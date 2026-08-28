# Work Package — Vulkan error-path classification

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsistema:** `Graphics / Vulkan lifecycle`  
**Work Package:** `9.6 Vulkan lower-level failure-path classification`  
**Branch:** `docs/9-6-vulkan-error-path-classification-20260828`  
**PR:** `#104`

## Objetivo

Classificar os restantes caminhos de erro Vulkan e decidir quais requerem evidência executável adicional, sem introduzir uma infraestrutura de mocking artificial.

## Escopo

### Inclui

- inventário dos `VkResult` relevantes em `RendererCore` e `VulkanContext`;
- distinção entre implementação fail-closed e evidência executável;
- identificação do próximo boundary de maior valor para evidência;
- registo da decisão antes de qualquer alteração de produção.

### Não inclui

- mock framework Vulkan;
- substituição global de funções Vulkan;
- redesign do renderer;
- transactional swapchain rebuild;
- `RenderSnapshot`;
- gameplay.

## Dependências

### Depende de

- PR #94 — `vkDeviceWaitIdle()` failure evidence;
- current `RendererCore` / `VulkanContext` implementation;
- `PROJECT-STUDIES/ASCENDENDO` current-state research.

### Produz para

- eventual WP de `vkQueueSubmit` failure contract;
- Gate 9.6 evidence matrix.

### Consumidores afetados

- `RendererCore`;
- `RendererFacade`;
- Gate 9.6 documentation.

### Dependências de validação

- Linux/headless Vulkan;
- ASan/UBSan;
- Windows/Vulkan software driver, se surgir implementação executável.

## Decisões arquiteturais

```text
Problema/contexto:
Os restantes VkResult são explicitamente classificados no código, mas não existe fault injection executável para todos.

Decisão:
Não criar fault injection abrangente. Primeiro investigar qual failure boundary tem consequência de lifecycle/ownership que mereça prova executável.

Alternativas consideradas:
Mock Vulkan global; seams para todas as APIs; manter apenas análise estática.

Consequências:
Menor complexidade e menor superfície de teste; algumas branches permanecem suportadas por evidência estrutural.

Condição de revisão/remoção:
Um counterexample ou requisito explícito que demonstre que a análise estrutural é insuficiente.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| Criar seams sem valor real | média | médio | exigir boundary/invariante observável | mitigado |
| Perder uma falha de sincronização relevante | média | alto | investigar `vkQueueSubmit` após reset de fence | aberto |
| Confundir tratamento de erro com prova comportamental | média | alto | manter estados implementation/evidence separados | mitigado |

## Validação

### Testes automatizados

- revisão dos testes existentes de `RendererCore`;
- nova evidência apenas se um contrato material for identificado.

### Validação manual

- inspeção de `RendererFacade::drawFrame()` e do frame loop de `main.cpp`.

### Profiling / métricas

- não aplicável.

### Failure paths

- `vkQueueSubmit` é o próximo candidato a investigação devido ao reset imediato do fence antes da submissão.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [x] inventário dos paths realizado;
- [x] distinção implementation/evidence documentada;
- [x] próximo boundary de investigação identificado;
- [ ] decisão final sobre `vkQueueSubmit` failure contract;
- [ ] eventual implementação de seam, se justificada;
- [ ] documentação normativa final sincronizada;
- [ ] PR integrada.

## Alterações durante a execução

```text
Descoberta:
O código fail-closes múltiplos VkResult e o wait-idle já possui fault injection mínima.

Impacto:
A próxima tranche deve concentrar-se em semântica pós-submit, não em enumerar todas as APIs Vulkan.

Decisão tomada:
Criar primeiro caracterização documental; implementação fica condicionada a uma propriedade observável.

Documentos atualizados:
AUDIT/2026-08-28-vulkan-error-path-classification.md
```

## Evidência / referências

- `Game/Graphics/RendererCore.cpp`;
- `Game/Graphics/RendererFacade.cpp`;
- `Tests/Integration/test_renderer_core.cpp`;
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_DEEP.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-26-swapchain-recreation-failure-contract.md`.

## Fecho

**Resultado:** `parcialmente concluído`  
**Critério de saída:** `paths classificados e próximo boundary definido sem implementar abstração prematura`  
**Dívida residual:** `decisão sobre evidência executável de vkQueueSubmit`
