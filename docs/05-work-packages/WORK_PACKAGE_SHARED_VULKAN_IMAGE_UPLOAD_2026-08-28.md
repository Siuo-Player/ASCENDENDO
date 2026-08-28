# Work Package — shared Vulkan image upload primitive

## Estado

`DONE — PR #133`

## Contexto

`FontRendererGpu.cpp` e `SpriteRendererGpu.cpp` duplicavam a mesma sequência de criação/upload de imagens Vulkan. As diferenças eram propriedades concretas dos recursos (formato, filtro e dados de origem), não diferenças de lifecycle.

## Decisão

Escolhida a opção **B — primitive estreito de upload/creation de imagem**.

Não criar `TextureManager`, cache global ou abstração genérica de asset management.

## Contrato

```text
consumer
  ↓
uploadVulkanImage2D(ctx, pixels, width, height, format, filter, out)
  ↓
VkImage + VkDeviceMemory + VkImageView + VkSampler
```

O primitive é stateless depois da chamada. O consumidor mantém ownership dos handles devolvidos e do descriptor setup.

### Propriedades explícitas

- `VkFormat` continua fornecido pelo consumidor;
- `VkFilter` continua fornecido pelo consumidor;
- usage comum: `TRANSFER_DST | SAMPLED`;
- layout final: `SHADER_READ_ONLY_OPTIMAL`;
- command pool: graphics queue family existente;
- address mode: `CLAMP_TO_EDGE`;
- Font: `R8_UNORM` + `LINEAR`;
- Sprite: `R8G8B8A8_UNORM` + `NEAREST`.

### Failure contract

Falhas durante staging, allocation, binding, command recording/submission, image view ou sampler libertam recursos parcialmente criados e deixam o output nulo.

## Resultado

`Game/Graphics/VulkanImageUpload.h/.cpp` centraliza o lifecycle comum. `FontRendererGpu` e `SpriteRendererGpu` mantêm as responsabilidades específicas. Descriptor pools/sets não foram absorvidos pelo primitive.

## Validação

PR #133 foi integrada como `e3871bc935dfa52124ec5244ddbb04714caec161`.

Os três workflows obrigatórios passaram no head validado:
- Linux / Clang / C++20 / Headless Vulkan;
- Linux / Clang / ASan + UBSan / Headless Vulkan;
- Windows / Clang / C++20.

Os jobs correspondentes também passaram source-size, full tests, headless Vulkan e campaign validation.

## Exit criteria

- [x] existe uma única implementação do upload comum;
- [x] ownership está explícito;
- [x] `FontRendererGpu` usa o primitive sem regressão;
- [x] `SpriteRendererGpu` usa o primitive sem regressão;
- [x] failure paths limpam recursos parcialmente criados;
- [x] build/testes/sanitizers/Windows passam;
- [x] documentação finalizada.

## Próxima decisão

Reavaliar #22 `FontRenderer decomposition` e a decomposição de `SpriteRenderer` a partir das responsabilidades que permanecem após a remoção da infraestrutura Vulkan duplicada. Não generalizar o primitive sem nova evidência de duplicação e consumidores.
