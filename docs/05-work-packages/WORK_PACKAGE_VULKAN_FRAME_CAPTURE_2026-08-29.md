# ASCENDENDO — Work Package: Validation Vulkan Frame Capture

**Data:** 2026-08-29  
**Fase:** Presentation / camera validation  
**Estado:** IMPLEMENTAÇÃO EM VALIDAÇÃO CI  

## Objetivo

Criar o menor caminho possível entre um frame Vulkan realmente apresentado e uma imagem no host, para suportar futura evidência visual das stress scenes definidas no WP de 2026-08-29.

## Contrato

Quando `ASCENDENDO_CAPTURE_PPM` está definido antes da inicialização do jogo:

```text
swapchain
  → COLOR_ATTACHMENT + TRANSFER_SRC
  → render pass
  → transfer image → host-visible staging buffer
  → fence
  → binary PPM
```

Quando a variável não está definida, a swapchain mantém a política anterior e nenhuma cópia de readback é gravada.

## Escopo

- suporte apenas ao formato atual `VK_FORMAT_B8G8R8A8_SRGB`;
- staging buffer host-visible/coherent;
- uma captura por processo;
- PPM binário como formato de evidência simples;
- integração no `RendererCore`, sem `ScreenshotManager` genérico;
- `TRANSFER_SRC` pedido apenas quando necessário.

## Evidência executável

O teste unitário caracteriza o formato suportado. Os três gates CI devem demonstrar que o código compila e passa os testes sem captura ativada.

A execução efetiva com `ASCENDENDO_CAPTURE_PPM` e a validação visual do ficheiro produzido ainda são uma etapa separada.

## Limite explícito

Esta tranche **não** seleciona automaticamente as fixtures A–E do manifesto `visual_stress_scenes.py`. O launcher determinístico das cenas e a recolha sistemática `5 cenas × 3 viewports` continuam como próximo incremento.

## Critérios de aceitação

- runtime normal sem `ASCENDENDO_CAPTURE_PPM` mantém o comportamento existente;
- quando ativada, a surface deve suportar `VK_IMAGE_USAGE_TRANSFER_SRC_BIT`;
- falhas de criação/record/readback devem falhar fechadamente;
- nenhuma propriedade de gameplay ou física é alterada.
