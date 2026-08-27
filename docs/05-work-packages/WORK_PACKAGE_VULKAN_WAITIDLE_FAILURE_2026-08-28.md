# Work Package — Vulkan wait-idle failure evidence

## Identificação

**Roadmap:** 9.6 — Base Engineering Gate  
**Subsistema:** Presentation / Infrastructure  
**Work Package:** 9.6 — Vulkan wait-idle failure evidence  
**Branch:** `test/9-6-vulkan-waitidle-failure-20260828`  
**PR:** pending

## Objetivo

Demonstrar, com uma seam de teste mínima e local, a semântica de falha de `RendererCore::recreateSwapchain()` quando `vkDeviceWaitIdle()` retorna erro.

## Escopo

### Inclui

- contrato de falha de `vkDeviceWaitIdle()`;
- teste direto do estado de `RendererCore` após esse erro;
- preservação do comportamento consumidor fail-closed já existente;
- atualização da evidência do Gate 9.6.

### Não inclui

- redesign do renderer;
- mock framework Vulkan genérico;
- fault-injection framework transversal;
- migração geral de `RenderSnapshot`;
- alteração da política de swapchain não suportada por evidência.

## Dependências

### Depende de

- `RendererCore::recreateSwapchain()`;
- `RendererFacade::drawFrame()`;
- Gate 9.6 Vulkan lifecycle evidence;
- `docs/DEVELOPMENT_PROTOCOL.md`.

### Produz para

- `WORK_PACKAGE_9_6_GATE_EVIDENCE_SYNC.md`;
- decisão final do Gate 9.6.

### Consumidores afetados

- `RendererCore`;
- teste de lifecycle de `RendererCore`;
- documentação do Gate.

### Dependências de validação

- Linux/Clang/headless Vulkan test environment;
- teste com função `vkDeviceWaitIdle` substituída apenas na instância de `RendererCore` usada pelo teste de failure path.

## Decisões arquiteturais

```text
Problema/contexto:
A implementação de produção já retorna false antes de invalidar o estado quando vkDeviceWaitIdle() falha, mas a main não continha uma forma de induzir este VkResult num teste focado.

Decisão:
Adicionar um seam mínimo por instância: RendererCore recebe a função vkDeviceWaitIdle usada pela operação de recreateSwapchain(), com a implementação real como default. O teste injeta apenas uma função que devolve um erro Vulkan.

Alternativas consideradas:
1. mock layer Vulkan transversal — rejeitada por scope excessivo;
2. não testar o failure path — rejeitada porque o Gate exige evidência observável;
3. seam estreito por função — escolhido por alterar apenas a dependência necessária e manter a API/ownership local ao RendererCore.

Consequências:
A produção continua a usar vkDeviceWaitIdle diretamente através da função default. O teste passa a conseguir provar que o estado antigo é preservado quando o wait-idle falha.

Condição de revisão/remoção:
Reavaliar se uma futura abstração Vulkan dispatch já fornecer um seam equivalente; não expandir esta técnica para uma framework de mocks.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| seam ficar maior que a necessidade | baixa | médio | manter apenas uma função e uso local | mitigado |
| teste provar apenas RendererCore e não o loop completo | média | médio | documentar separadamente o consumer fail-closed já demonstrado por inspeção | aberto |
| assinatura de Vulkan ser mascarada por wrapper genérico | baixa | médio | manter tipo explícito e implementação default igual a `vkDeviceWaitIdle` | mitigado |

## Validação

### Testes automatizados

- `RendererCore` continua a passar na recreação normal;
- novo teste injeta `VK_ERROR_DEVICE_LOST` em `vkDeviceWaitIdle()` e verifica `false` + `isInitialized() == true` + preservação de recursos/estado observável por `swapchainExtent()`;
- testes normais, ASan/UBSan e Windows devem passar no mesmo commit.

### Validação manual

- revisar que `RendererFacade::drawFrame()` retorna o resultado de `recreateSwapchain()` sem tentar submeter outro frame;
- revisar que `main.cpp` interrompe o loop quando `drawFrame()` retorna `false`.

### Profiling / métricas

- nenhuma.

### Failure paths

- `vkDeviceWaitIdle() != VK_SUCCESS` no início de `recreateSwapchain()`;
- falha posterior à invalidação de `m_initialized` continua fail-closed.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [ ] implementação concluída dentro do escopo;
- [ ] testes relevantes passam;
- [ ] failure paths relevantes foram exercitados;
- [ ] documentação normativa foi atualizada;
- [ ] dependências alteradas foram revistas;
- [ ] dívida técnica criada foi classificada;
- [ ] critério de saída foi demonstrado;
- [ ] PR pronta para merge sem trabalho essencial oculto.

## Alterações durante a execução

```text
Descoberta: o gap não exige uma mock layer; um único function pointer por RendererCore é suficiente para reproduzir a falha.
Impacto: a tranche pode transformar a semântica já implementada em evidência executável sem alterar o lifecycle global.
Decisão tomada: usar seam mínimo local e manter a prova consumer/frame-loop como evidência estrutural separada.
Documentos atualizados: audit 2026-08-28, ROADMAP e TECH_DEBT já reconciliados em #93.
```

## Evidência / referências

- `docs/AUDITS/2026-08-28-vulkan-waitidle-contract-revalidation.md`;
- `Game/Graphics/RendererCore.cpp`;
- `Game/Graphics/RendererFacade.cpp`;
- `Tests/Integration/test_renderer_core.cpp`;
- Study `2026-08-27-vulkan-recreate-waitidle-contract.md` como evidência histórica revalidada contra `main`.

## Fecho

**Resultado:** `em implementação`  
**Critério de saída:** teste induzido do erro + estado pós-falha demonstrado + suites CI relevantes verdes  
**Dívida residual:** consumer/frame-loop failure-path não é diretamente injetado neste teste
