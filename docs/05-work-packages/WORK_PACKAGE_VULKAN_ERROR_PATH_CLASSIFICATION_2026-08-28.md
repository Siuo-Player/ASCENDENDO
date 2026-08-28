# Work Package — Vulkan error-path classification

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsistema:** `Graphics / Vulkan lifecycle`  
**Work Package:** `9.6 Vulkan lower-level failure-path classification`  
**Branch:** `docs/9-6-vulkan-error-path-classification-20260828-v2`  
**PR:** `#107`

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

## Decisão arquitetural

Não criar fault injection abrangente. Primeiro determinar se `vkQueueSubmit` tem uma propriedade pós-falha de lifecycle/synchronization que justifique uma seam mínima.

## Riscos

- criar seams sem valor real;
- confundir tratamento estrutural com prova comportamental;
- perder uma ambiguidade de sincronização após falha de submit.

## Validação

- inspeção dos paths atuais;
- futura evidência executável apenas se a investigação encontrar um invariant material.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentação dos Studies consultada;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida.

## Definition of Done

- [x] paths classificados;
- [x] implementation/evidence separados;
- [x] próximo boundary identificado;
- [ ] decisão final sobre `vkQueueSubmit`;
- [ ] eventual seam, se justificada;
- [ ] documentação final integrada.

## Evidência / referências

- `Game/Graphics/RendererCore.cpp` at `main` `bec96c3...`;
- `Game/Graphics/RendererFacade.cpp`;
- `Tests/Integration/test_renderer_core.cpp`;
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_1329.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-26-swapchain-recreation-failure-contract.md`.

## Fecho

**Resultado:** `investigated / decision pending`  
**Dívida residual:** `vkQueueSubmit failure contract`
