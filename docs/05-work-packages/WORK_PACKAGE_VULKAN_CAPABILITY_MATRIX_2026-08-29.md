# Work Package — Vulkan capability matrix semantics

## Identificação

**Fase:** Vulkan capability/error semantics  
**Issue:** #162  
**Implementation:** PR #164  
**Merge:** `95419bf25adb2a2c7227b0a72b3a23d714aeaf30`  
**Estado:** **DONE**

## Finding

`Tests/Integration/test_vulkan_init.cpp` exigia `VK_KHR_swapchain` e uma graphics queue para cada dispositivo Vulkan 1.3+ encontrado.

O runtime não possui essa política. `VulkanContext::selectPhysicalDevice()` filtra dispositivos inadequados e procura uma candidatura válida.

## Decisão

O teste deve modelar a política do runtime:

```text
enumerate devices
→ reject unsupported API
→ reject missing swapchain/graphics capability
→ require at least one valid runtime candidate
```

Dispositivos incompatíveis são reportados e ignorados; o teste falha apenas se não existir nenhuma candidatura válida.

## Escopo

- corrigir apenas a semântica do teste de capability matrix;
- nenhum change em `VulkanContext` ou no runtime;
- nenhuma abstração de fault injection.

## Evidência

- `VulkanContext::selectPhysicalDevice()` já filtra candidatos inadequados;
- PR #164 alterou 14 linhas adicionadas / 4 removidas em um único ficheiro de teste;
- Linux / Clang / C++20 / Headless Vulkan: success;
- Linux / Clang / ASan + UBSan / Headless Vulkan: success;
- Windows / Clang / C++20: success;
- source-size, full suite e campaign validation: success.

## Critério de saída

```text
O teste representa a política real de seleção de physical devices
+ pelo menos uma candidatura válida é exigida
+ dispositivos incompatíveis não causam false failure
+ runtime de produção inalterado
+ três workflows obrigatórios verdes
```
