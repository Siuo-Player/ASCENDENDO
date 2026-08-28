# Work Package — vkQueueSubmit post-failure contract

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsistema:** `Graphics / Vulkan lifecycle`
**Work Package:** `9.6 vkQueueSubmit post-failure contract`
**Branch:** `docs/9-6-vk-queue-submit-contract-20260828`

## Objetivo

Determinar o contrato observável após uma falha de `vkQueueSubmit()` e verificar se existe um requisito ainda não provado que justifique fault injection adicional.

## Scope

### Inclui
- sequência `vkResetFences → vkQueueSubmit` em `RendererCore::submitFrame()`;
- retorno `FrameStatus::Fatal` em qualquer `vkQueueSubmit != VK_SUCCESS`;
- propagação para `RendererFacade::drawFrame()`;
- comportamento do loop principal perante `drawFrame() == false`;
- distinção entre contrato de término/fail-closed e eventual recuperação da mesma instância.

### Não inclui
- mock framework Vulkan;
- substituição global das funções Vulkan;
- tentativa de continuar a usar a mesma frame após falha de submit;
- redesign de sincronização;
- gameplay ou `RenderSnapshot`.

## Observação do código

A implementação atual faz:

```text
vkResetFences(inFlightFence)
        ↓
vkQueueSubmit(..., inFlightFence)
        ↓
VK_SUCCESS     → present
qualquer erro   → FrameStatus::Fatal
```

`RendererFacade::drawFrame()` devolve `false` para `FrameStatus::Fatal`. O `main` imprime o erro e executa `break` no loop principal; portanto a aplicação não tenta reutilizar o core como se a frame estivesse recuperável.

## Contrato resultante

A propriedade atualmente suportada pela implementação é **fail-closed terminal no boundary do frame**:

1. o fence pode ser reset antes do submit;
2. uma falha de `vkQueueSubmit()` não é tratada como frame recuperável;
3. o estado de sincronização dessa frame não é reutilizado pelo caller;
4. o erro é propagado até o loop principal;
5. o loop abandona a execução normal e procede para o encerramento.

Este contrato é diferente de afirmar que a API Vulkan torna o fence seguro para reutilização após uma falha de submit. Essa propriedade mais forte não é necessária para o comportamento atual porque o caller encerra a execução em vez de iniciar outra frame com a mesma sincronização.

## Decisão

Não introduzir uma seam de `vkQueueSubmit()` apenas para produzir cobertura artificial. A evidência estática da sequência caller/callee e do boundary terminal estabelece a propriedade relevante para o design atual: **não há caminho de recuperação do frame falhado dentro da mesma execução**.

Uma seam adicional só seria justificada se um requisito futuro exigir recuperação após falha de submit ou se um teste precisar provar uma propriedade inferior ao contrato terminal atual.

## Definition of Done

- [x] sequência de sincronização inspecionada;
- [x] retorno de erro classificado;
- [x] boundary da facade verificado;
- [x] comportamento do loop principal verificado;
- [x] propriedade necessária ao design atual identificada;
- [x] ausência de necessidade atual de broad fault injection justificada;
- [ ] eventual teste executável futuro, apenas se surgir requisito de recuperação.

## Evidência

- `Game/Graphics/RendererCore.cpp`;
- `Game/Graphics/RendererCore.h`;
- `Game/Graphics/RendererFacade.cpp`;
- `main.cpp`;
- PR #94 — `vkDeviceWaitIdle()` executable failure evidence;
- PR #108 — residual Vulkan failure-path classification.

## Resultado

**Status:** `investigated / contract established`

**Residual:** não existe atualmente um requisito de recuperação após `vkQueueSubmit()` failure; manter fail-closed e não adicionar mocking amplo.
