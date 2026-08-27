# Work Package — Vulkan lifecycle and capability validation

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Graphics / Vulkan`
**Work Package:** `9.6 Vulkan lifecycle + capability validation`
**Branch:** `feat/9-6-vulkan-lifecycle-validation`
**PR:** `<to be created>`

## Objetivo

Demonstrar, com evidência executável, que o caminho Vulkan atual respeita as invariantes de queue selection, device capabilities, frame synchronization e swapchain error handling exigidas pelo Gate 9.6.

## Contexto / problema

O código atual já contém decisões relevantes: graphics e present queues são representadas separadamente; a criação do logical device deduplica families apenas quando são iguais; o swapchain usa concurrent sharing quando são distintas; `VK_ERROR_OUT_OF_DATE_KHR` e `VK_SUBOPTIMAL_KHR` são tratados no acquire/present; e o fence só é reset imediatamente antes de `vkQueueSubmit`.

O problema restante é de **evidência**: os testes atuais demonstram parte destas propriedades com dados sintéticos ou cobrem apenas inicialização básica. O Gate exige evidência da capability matrix e dos invariants do frame lifecycle, não apenas CI verde.

## Observação / evidência atual

`VulkanContext`:
- rejeita devices sem graphics queue;
- exige `VK_KHR_swapchain`;
- para uma surface exige graphics + present queue family;
- cria uma queue para cada family distinta;
- não assume que graphics e present são a mesma family.

`RendererCore`:
- espera o in-flight fence antes do acquire;
- trata `OUT_OF_DATE`/`SUBOPTIMAL` do acquire;
- só reseta o fence no caminho de submissão;
- trata `OUT_OF_DATE`/`SUBOPTIMAL` do present;
- usa `vkDeviceWaitIdle` antes de cleanup/recreate.

`Swapchain`:
- valida suporte da present queue à surface;
- usa `CONCURRENT` quando graphics/present são diferentes;
- recria image views e recursos dependentes.

## Decisão arquitetural

Não introduzir uma nova abstração Vulkan nesta tranche.

Adicionar cobertura de validação diretamente aos testes de integração existentes, expondo apenas os invariants que já fazem parte da API/estado observável de `VulkanContext`/`Swapchain`/`RendererCore`.

A implementação deve preferir testes de capability discovery real e invariantes sobre hardware/software disponível, em vez de mocks que poderiam confirmar apenas a própria implementação.

## Alternativas consideradas

1. Reescrever `VulkanContext` num novo `DeviceRuntime` — rejeitado; a estrutura atual já separa instance/device/queue e a dívida é sobretudo de demonstração.
2. Introduzir mocks de Vulkan para simular todos os `VkResult` — rejeitado nesta tranche; exigiria uma camada de abstração que o projeto não possui e aumentaria coupling.
3. Fechar o item apenas porque CI/headless Vulkan passa — rejeitado; o RoadMap exige capability/error-path evidence adicional.

## Escopo

### Inclui

- testes da capability matrix mínima de device/queues/extensions;
- testes de consistência entre queue families reportadas e estado do context;
- testes de lifecycle que exercitem inicialização, criação de surface, swapchain e cleanup sem violar ownership;
- reforço da documentação do comportamento `OUT_OF_DATE`/`SUBOPTIMAL` e fence reset;
- validação Linux normal + ASan/UBSan + campaign validator;
- revisão da debt/roadmap após a evidência.

### Não inclui

- suporte a múltiplos frames-in-flight;
- redesign de `RendererCore`;
- sincronização avançada com timeline semaphores;
- swapchain resizing como nova feature;
- Windows CI;
- `RenderSnapshot`;
- nova abstração de device/queue.

## Dependências

```text
VulkanContext
   ↓
Swapchain
   ↓
RendererCore
   ↓
RendererFacade / PresentationRuntime
```

Validation dependencies: Linux Vulkan 1.3+, GLFW/Xvfb, software ICD disponível no CI.

Consumers affected: integration tests and future Gate review only; no gameplay consumer API should change.

## Invariantes de saída

- device utilizado pelo runtime possui graphics queue válida;
- quando existe uma surface, present queue é válida para essa surface;
- graphics/present podem ser families distintas sem alterar a correção do setup;
- `VK_KHR_swapchain` é requisito explícito;
- acquire/present não confundem `OUT_OF_DATE`/`SUBOPTIMAL` com sucesso normal;
- fence não é reset antes de existir um caminho de submissão;
- cleanup ocorre depois de o device ficar idle;
- recursos dependentes do swapchain não sobrevivem à sua destruição.

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| O CI só expõe uma queue family real | médio | médio | complementar capability discovery com invariantes estruturais; não declarar suporte físico não observado como demonstrado | aberto |
| Um teste passa sem exercitar erro real de acquire/present | médio | médio | separar evidência de implementação existente de execução real de erro; não usar sucesso normal como prova de erro | aberto |
| Alterar sincronização cria regressão subtil | médio | alto | limitar mudanças de código à mínima necessidade e validar com ASan/UBSan + headless | aberto |

## Validação

- `Tests/Tooling/test_check_source_sizes.py`;
- `Development/Tools/check_source_sizes.py`;
- `make tests` em Linux/headless;
- workflow independente ASan/UBSan;
- campaign validation;
- inspeção do diff e da capability evidence;
- documentar explicitamente quais error paths foram realmente executados e quais permanecem apenas comprovados estaticamente.

## Definition of Ready

- [x] Roadmap e dívida consultados;
- [x] código atual de Vulkan inspecionado;
- [x] invariantes explicitados;
- [x] alternativas registadas;
- [x] scope limitado a evidence/hardening;
- [x] branch criada a partir do `main` integrado após PR #73.

## Definition of Done

- [ ] capability matrix mínima coberta por testes executáveis;
- [ ] lifecycle invariants cobertos ou explicitamente classificados como apenas estáticos;
- [ ] normal + ASan/UBSan verdes;
- [ ] documentação de Roadmap/TECH_DEBT/Architecture sincronizada;
- [ ] PR integrada.

## Próxima decisão

Primeiro adicionar a cobertura de capability discovery e lifecycle observável. Só introduzir alteração de runtime se os testes demonstrarem uma lacuna concreta de comportamento.
