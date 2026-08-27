# Work Package — VulkanContext failure-state invariant

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `VulkanContext / Vulkan lifecycle`  
**Work Package:** `9.6 VulkanContext failure-state invariant`  
**Branch:** `fix/9-6-vulkan-context-state-invariant`  
**Base:** `main` after PR #81

## Objetivo

Garantir que `VulkanContext` não publique um estado `initialized` incompatível com os seus recursos depois de uma reconfiguração de surface que destrói o device anterior e falha ao reconstruí-lo.

## Descoberta / evidência

PROJECT-STUDIES PR #8 identificou um possível leak quando a aquisição das queues falha após `vkCreateDevice()`. A inspeção do código atual contradiz essa parte do diagnóstico: `createLogicalDevice()` já chama `vkDestroyDevice(device, nullptr)` quando os handles das queues são inválidos.

O gap confirmado é inferior e específico: `reconfigureForSurface()` destrói o device existente e depois chama `createLogicalDevice()`. Se a recriação falhar, `m_initialized` não era explicitamente normalizado para o novo estado sem device.

## Decisão

A partir do momento em que o device antigo é destruído durante `reconfigureForSurface()`:

```text
m_initialized == false
m_device == VK_NULL_HANDLE
m_graphicsQueue == VK_NULL_HANDLE
m_presentQueue == VK_NULL_HANDLE
```

Após `createLogicalDevice()` ter sucesso:

```text
m_initialized == true
m_device != VK_NULL_HANDLE
m_graphicsQueue != VK_NULL_HANDLE
m_presentQueue != VK_NULL_HANDLE
```

Não alterar a política de seleção de devices/queues nem introduzir um framework transacional.

## Inclui

- normalização explícita do estado durante surface reconfiguration;
- preservação do rollback existente de `createLogicalDevice()`;
- testes de lifecycle que sejam reproduzíveis no backend real sem mocks de Vulkan;
- documentação da distinção entre finding do estudo e evidência de código atual;
- validação Linux/headless e ASan/UBSan.

## Não inclui

- redesign de queue selection;
- novo sistema de injeção/mock de Vulkan;
- swapchain recreation redesign;
- Windows CI;
- `Application` abstraction.

## Dependências / consumidores

```text
GraphicsRuntime
    ↓
VulkanContext
    ├── Surface
    ├── PhysicalDevice
    └── LogicalDevice + queues
```

Consumers: `GraphicsRuntime`, `Swapchain`, renderer core/presentation e testes gráficos.

## Validação

- build normal Linux/headless;
- ASan/UBSan;
- confirmar que o caminho atual de queue-handle failure mantém o `VkDevice` destruído;
- confirmar estado neutro durante/apos falha de surface reconfiguration onde o backend permite indução segura;
- documentar explicitamente qualquer failure path que não possa ser induzido sem mocks.

## Definition of Ready

- [x] Study PR #8 consultada;
- [x] finding revalidado contra o código atual;
- [x] distinção leak já corrigido vs state invariant ainda aberto documentada;
- [x] dependências Vulkan inspecionadas;
- [x] scope negativo definido.

## Definition of Done

- [ ] invariant implementada;
- [ ] evidence/tests executados;
- [ ] ASan/UBSan verdes;
- [ ] documentação canónica sincronizada;
- [ ] PR integrada;
- [ ] remaining Vulkan recovery gaps mantidos explícitos.
