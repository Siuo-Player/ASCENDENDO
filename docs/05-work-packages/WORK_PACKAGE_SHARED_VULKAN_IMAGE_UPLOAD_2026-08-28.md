# Work Package — shared Vulkan image upload primitive

## Estado

`READY FOR IMPLEMENTATION`

## Contexto

`FontRendererGpu.cpp` e `SpriteRendererGpu.cpp` duplicam a mesma sequência de criação/upload de imagens Vulkan. As diferenças são propriedades concretas dos recursos (formato, filtro e dados de origem), não diferenças de lifecycle.

## Decisão

Escolhida a opção **B — primitive estreito de upload/creation de imagem**.

Não criar `TextureManager`, cache global ou abstração genérica de asset management.

## Contrato

```text
consumer
  ↓
uploadImage2D(ctx, pixels, width, height, format, filter, out)
  ↓
VkImage + VkDeviceMemory + VkImageView + VkSampler
```

O primitive é stateless depois da chamada. O consumidor passa a ser responsável pelos handles devolvidos e pelo descriptor setup.

### Propriedades explícitas

- `VkFormat` é fornecido pelo consumidor;
- `VkFilter` é fornecido pelo consumidor;
- usage comum fica limitado a `TRANSFER_DST | SAMPLED`;
- image layout final é `SHADER_READ_ONLY_OPTIMAL`;
- command pool usa a graphics queue family existente;
- sampler mantém clamp-to-edge e anisotropy efetiva atual.

### Failure contract

Qualquer falha durante staging, allocation, binding, command recording/submission, image view ou sampler deixa todos os handles de saída nulos e liberta os recursos parcialmente criados.

## Escopo

- adicionar primitive compartilhado;
- migrar `FontRendererGpu`;
- migrar `SpriteRendererGpu`;
- manter descriptor resources específicos;
- adicionar testes de preconditions;
- validar em Linux normal, ASan/UBSan e Windows.

## Fora de escopo

- alterar semântica visual;
- alterar filtros ou formatos atuais;
- shaders/pipelines;
- asset manager/cache;
- performance tuning.

## Dependências

- `VulkanContext`;
- #22 `FontRenderer decomposition`;
- `docs/CODE_SIZE.md`;
- `docs/ARCHITECTURE.md`;
- `docs/DEVELOPMENT_PROTOCOL.md`.

## Exit criteria

- existe uma única implementação do upload comum;
- ownership está explícito;
- `FontRendererGpu` e `SpriteRendererGpu` mantêm comportamento;
- failure paths limpam recursos parcialmente criados;
- build/testes/sanitizers/Windows passam;
- documentação finalizada.
