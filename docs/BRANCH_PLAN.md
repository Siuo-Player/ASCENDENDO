# Plano da branch atual

**Bloco do roadmap:** `9.6 Base Engineering Gate → D — modularity work packages`

**Work Package:** `D.0 — shared Vulkan image upload primitive`

**Issue:** `#23`

**Branch de implementação:** `refactor/shared-vulkan-image-upload-20260828`
**PR:** `#133`

## Resultado

`DONE — PR #133 integrado como e3871bc935dfa52124ec5244ddbb04714caec161`

## Descoberta

`FontRendererGpu.cpp` e `SpriteRendererGpu.cpp` continham duas implementações semelhantes do ciclo de vida Vulkan para imagens: staging, memória, imagem, command buffer one-shot, transitions, copy, view e sampler.

## Decisão

Escolhida a opção **B — primitive estreito de upload/creation de imagem Vulkan**. O primitive não é `TextureManager`, não possui cache global e não absorve descriptor policy.

## Contrato implementado

```text
FontRendererGpu ─┐
                 ├→ VulkanImageUpload
SpriteRendererGpu┘
```

`format` e `filter` permanecem explícitos. O primitive devolve `VkImage`, `VkDeviceMemory`, `VkImageView` e `VkSampler`; o consumer mantém ownership e configura os descriptors específicos.

## Validação

Os três workflows obrigatórios do head validado passaram:

- Linux / Clang / C++20 / Headless Vulkan;
- Linux / Clang / ASan + UBSan / Headless Vulkan;
- Windows / Clang / C++20.

Também passaram source-size, full tests, Vulkan headless e campaign validation nos jobs correspondentes.

## Mudanças

A infraestrutura duplicada foi removida dos dois consumidores. Os formatos/filtros existentes foram preservados:

- Font: `R8_UNORM` + `LINEAR`;
- Sprite: `R8G8B8A8_UNORM` + `NEAREST`.

## Próximo dependente

Reavaliar #22 `FontRenderer decomposition` e a decomposição de `SpriteRenderer` com base nas responsabilidades restantes. Não introduzir abstrações genéricas sem novos consumidores e evidência.
