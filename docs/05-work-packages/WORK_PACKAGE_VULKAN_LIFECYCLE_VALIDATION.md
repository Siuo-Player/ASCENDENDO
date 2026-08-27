# Work Package — Vulkan lifecycle and capability validation

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Graphics / Vulkan`
**Work Package:** `9.6 Vulkan lifecycle + capability validation`
**Branch:** `feat/9-6-vulkan-lifecycle-validation`
**PR:** `#74`

## Objetivo

Demonstrar, com evidência executável, que o caminho Vulkan atual respeita as invariantes de queue selection, device capabilities, frame synchronization e swapchain error handling exigidas pelo Gate 9.6.

## Contexto / problema

O código atual já contém decisões relevantes: graphics e present queues são representadas separadamente; a criação do logical device deduplica families apenas quando são iguais; o swapchain usa concurrent sharing quando são distintas; `VK_ERROR_OUT_OF_DATE_KHR` e `VK_SUBOPTIMAL_KHR` são tratados no acquire/present; e o fence só é reset imediatamente antes de `vkQueueSubmit`.

O problema restante era de **evidência**: os testes demonstravam parte destas propriedades com dados sintéticos ou cobriam apenas inicialização básica. O Gate exige evidência da capability matrix e dos invariants do frame lifecycle, não apenas CI verde.

## Decisão arquitetural

Não introduzir uma nova abstração Vulkan nesta tranche.

Adicionar cobertura de validação diretamente aos testes de integração existentes, expondo apenas os invariants que já fazem parte da API/estado observável de `VulkanContext`/`Swapchain`/`RendererCore`.

A implementação prefere capability discovery real e invariantes sobre hardware/software disponível, em vez de mocks que poderiam confirmar apenas a própria implementação.

## Resultado observado

A cobertura adicionada demonstrou no CI Linux/headless:

- capability matrix mínima: Vulkan 1.3+, pelo menos uma physical device, `VK_KHR_swapchain` e graphics queue;
- criação de uma janela + `VkSurfaceKHR` real;
- graphics queue e present queue válidas no `VulkanContext`;
- suporte efetivo da present family à surface através de `vkGetPhysicalDeviceSurfaceSupportKHR`;
- execução normal e ASan/UBSan sem falhas.

A capability matrix foi **observada no ambiente CI atual**, não universalmente provada para todo hardware possível.

## Scope

### Inclui

- testes da capability matrix mínima de device/queues/extensions;
- testes de consistência entre queue families reportadas e estado do context;
- testes de lifecycle que exercitem inicialização, criação de surface, swapchain e cleanup sem violar ownership;
- reforço da evidência de `OUT_OF_DATE`/`SUBOPTIMAL` e fence reset como invariantes estáticos do código;
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

Consumers affected: integration tests and future Gate review only; no gameplay consumer API changed.

## Invariantes de saída

- device utilizado pelo runtime possui graphics queue válida;
- quando existe uma surface, present queue é válida para essa surface;
- graphics/present podem ser families distintas sem alterar a correção do setup;
- `VK_KHR_swapchain` é requisito explícito;
- acquire/present tratam `OUT_OF_DATE`/`SUBOPTIMAL` como condições de recreação e não como sucesso normal;
- fence não é reset antes de existir um caminho de submissão;
- cleanup ocorre depois de o device ficar idle;
- recursos dependentes do swapchain não sobrevivem à sua destruição.

## O que foi realmente demonstrado

| Invariante | Tipo de evidência | Resultado |
|---|---|---|
| Vulkan ≥ 1.3 | executável, driver CI | ✅ |
| physical device disponível | executável, driver CI | ✅ |
| `VK_KHR_swapchain` disponível | executável, driver CI | ✅ |
| graphics queue disponível | executável, driver CI | ✅ |
| present family válida para surface | executável, surface real | ✅ |
| graphics/present não são estruturalmente assumidas iguais | executável + teste estrutural | ✅ |
| acquire/present OUT_OF_DATE/SUBOPTIMAL tratados | inspeção estática do `RendererCore` | ✅ |
| fence reset apenas antes de submit | inspeção estática do `RendererCore` | ✅ |
| reset/submit/present error-path sem deadlock | sem injeção real de `VkResult` nesta tranche | ⚠️ evidência incompleta |

A última linha permanece dívida explícita: sem uma camada de injeção/mocking de Vulkan não é possível transformar todos os `VkResult` adversariais em execução real sem criar uma abstração nova. O código existente continua a tratar os paths previstos e permanece coberto por execução normal + sanitizers.

## Validation

- `Tests/Tooling/test_check_source_sizes.py`;
- `Development/Tools/check_source_sizes.py`;
- `make tests` em Linux/headless;
- workflow independente ASan/UBSan;
- campaign validation;
- inspeção do diff e da capability evidence;
- distinguir explicitamente error paths executados daqueles apenas comprovados por inspeção.

### Evidência final

Commit de código: `8998290fa2ad2d04c5306640553d228406a3543a`.

- `Tests #775` — success;
- `Sanitizers #17` — success;
- source-size checks — success;
- headless Vulkan — success;
- campaign validation — success.

A atualização documental posterior também passou o workflow normal final `Tests #778` — success. Os commits posteriores desta branch não alteraram runtime/build code.

## Definition of Ready

- [x] Roadmap e dívida consultados;
- [x] código atual de Vulkan inspecionado;
- [x] invariantes explicitados;
- [x] alternativas registadas;
- [x] scope limitado a evidence/hardening;
- [x] branch criada a partir do `main` integrado após PR #73.

## Definition of Done

- [x] capability matrix mínima coberta por testes executáveis;
- [x] lifecycle invariants cobertos ou explicitamente classificados como apenas estáticos;
- [x] normal + ASan/UBSan verdes;
- [x] dívida/documentação de Vulkan sincronizada;
- [x] workflow final validado em CI;
- [x] branch pronta para integração;
- [ ] PR integrada.

## Próxima decisão

A dívida restante de error-path adversarial é mantida explícita. Não introduzir mocks/abstrações Vulkan apenas para fabricar esses resultados nesta tranche. A próxima revisão do Gate deve decidir se essa evidência adicional justifica uma camada de injeção testável ou se a classificação estática é suficiente para 9.6.
