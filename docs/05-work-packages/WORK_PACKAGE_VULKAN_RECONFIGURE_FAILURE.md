# Work Package — Vulkan surface reconfiguration failure contract

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Vulkan / lifecycle`
**Work Package:** `9.6 Vulkan reconfigure terminal-state contract`
**Branch:** `fix/9-6-vulkan-reconfigure-terminal-state`
**Base:** `main` após PR #86 (`31a9c69d084970ef054d9a86f3b144fca6772238`)
**Classificação:** `FIX + EVIDENCE`

## Finding confirmado

`VulkanContext::reconfigureForSurface()` podia destruir o logical device anterior, falhar ao criar o novo device e deixar `m_initialized == true`.

Isso criava um estado observável inconsistente:

```text
isInitialized() == true
m_device == VK_NULL_HANDLE
```

A mesma fronteira também ignorava o resultado de `vkDeviceWaitIdle()` antes de destruir o device anterior.

## Decisão

Adotar a política **terminal failure** para `VulkanContext` durante surface reconfiguration:

```text
reconfigure failure
→ surface cleanup
→ old device cleanup, se ainda existir
→ queues invalidated
→ isInitialized() == false
→ caller must perform full shutdown() + init()
```

Não é introduzido rollback transacional do device anterior.

## Alteração implementada

- `vkDeviceWaitIdle()` é observado antes da destruição do device durante reconfiguração;
- falha de `vkDeviceWaitIdle()` é tratada como terminal;
- qualquer falha de criação do novo logical device deixa o contexto não inicializado;
- uma falha precoce, anterior à substituição do device, continua a destruir o device antigo no caller antes de normalizar o estado;
- queues são limpas sempre que o device é destruído;
- `shutdown()` continua a ser a operação de recuperação suportada.

## Evidência

O teste unitário cobre a pré-condição `VK_NULL_HANDLE` para `createSurface()` e confirma que uma entrada inválida não invalida um contexto já inicializado.

A falha real de `vkCreateDevice()`/queue acquisition não possui atualmente seam determinístico de injeção no wrapper Vulkan. Não se adicionou um mock global ou um test hook artificial apenas para fabricar essa falha.

Portanto, a propriedade é suportada por:

```text
code inspection
+ explicit terminal-state logic
+ normal Vulkan integration tests
+ sanitizer coverage
```

mas a matriz ainda não considera uma simulação determinística de cada código de erro Vulkan encerrada.

## Não incluído

- redesign de VulkanContext;
- Vulkan abstraction layer;
- mock framework;
- swapchain redesign;
- RenderSnapshot;
- alterações de gameplay.

## Definition of Done

- [x] finding reproduzível por inspeção do estado;
- [x] política terminal documentada;
- [x] wait-idle result observado;
- [x] old-device leak path evitado;
- [x] queues normalizadas após falha;
- [x] teste de precondition invalid surface;
- [ ] fault injection determinístico para cada VkResult, como trabalho futuro de maior cobertura.

## Relação com o Gate 9.6

Este WP reduz a dívida de **Vulkan lifecycle** mas não fecha o Gate sozinho. Continuam exigidos os restantes failure paths, replay/determinism, malformed/error-path evidence e a revisão final do Gate.
