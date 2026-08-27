# Work Package — Vulkan wait-idle failure evidence

## Identificação

**Roadmap:** 9.6 — Base Engineering Gate  
**Subsistema:** Presentation / Infrastructure  
**Work Package:** 9.6 — Vulkan wait-idle failure evidence  
**Branch:** `docs/9-6-evidence-reconciliation-20260828` (documentation/reconciliation; implementation branch to be created only if a justified test seam is identified)  
**PR:** pending

## Objetivo

Clarificar e, quando tecnicamente justificável sem uma mock layer ampla, demonstrar a semântica de falha de `RendererCore::recreateSwapchain()` quando `vkDeviceWaitIdle()` retorna erro.

## Escopo

### Inclui

- contrato de falha de `vkDeviceWaitIdle()`;
- comportamento consumidor em `RendererFacade::drawFrame()`;
- atualização da evidência do Gate 9.6;
- avaliação explícita da necessidade de um seam de teste.

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
- `RendererFacade`;
- integração de frame loop em `main.cpp`;
- testes de lifecycle;
- documentação do Gate.

### Dependências de validação

- Linux/Clang/headless Vulkan test environment;
- eventual mecanismo estreito de injeção de `vkDeviceWaitIdle` apenas se o custo e ownership forem justificados.

## Decisões arquiteturais

```text
Problema/contexto:
A falha de vkDeviceWaitIdle() preserva o estado anterior de RendererCore e propaga false até ao frame loop, mas ainda não existe teste que induza especificamente esse VkResult.

Decisão:
Não adicionar uma camada genérica de mocks/fault injection apenas para este finding. Preservar o comportamento fail-closed existente e avaliar primeiro se existe um seam pequeno, local e reutilizável.

Alternativas consideradas:
1. mock layer Vulkan transversal — rejeitada por scope excessivo;
2. ignorar a ausência de teste — rejeitada porque o Gate exige evidência observável;
3. seam estreito para a chamada wait-idle — permitida apenas se se mantiver confinada ao renderer e melhorar testabilidade real.

Consequências:
O Gate continua com uma lacuna de evidência específica, mas a semântica de produção fica explicitamente documentada e não é alterada por abstrações especulativas.

Condição de revisão/remoção:
Reavaliar quando o conjunto de failure-path tests Vulkan justificar um seam partilhado ou quando o Gate exigir um teste executável desta condição.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| ausência de teste deixa o failure path parcialmente não demonstrado | alta | alto | manter item explícito no Gate e procurar seam mínimo | aberto |
| fault injection cresce para infraestrutura genérica | média | alto | limitar qualquer seam ao renderer e a uma necessidade comprovada | mitigado |
| documentação passa a sugerir robustez superior à evidência | média | alto | separar implementação, comportamento observado e evidência induzida | mitigado |

## Validação

### Testes automatizados

- teste existente de recreate bem-sucedido permanece verde;
- eventual teste específico de wait-idle failure, apenas após seam justificado.

### Validação manual

- confirmar que `drawFrame()` retorna `false` depois de uma falha de recreação;
- confirmar que o frame loop não continua para um novo draw/submit após esse retorno.

### Profiling / métricas

- nenhuma.

### Failure paths

- `vkDeviceWaitIdle() != VK_SUCCESS` em `RendererCore::recreateSwapchain()`;
- falha posterior à invalidação de `m_initialized` permanece fail-closed.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [ ] implementação concluída dentro do escopo, caso uma implementação seja necessária;
- [ ] testes relevantes passam;
- [ ] failure paths relevantes foram exercitados, ou a limitação ficou explicitamente demonstrada;
- [ ] documentação normativa foi atualizada;
- [ ] dependências alteradas foram revistas;
- [ ] dívida técnica criada foi classificada;
- [ ] critério de saída foi demonstrado;
- [ ] PR pronta para merge sem trabalho essencial oculto.

## Alterações durante a execução

```text
Descoberta: current main já propaga false de recreateSwapchain() até drawFrame() e o main loop interrompe o ciclo após false.
Impacto: o finding histórico é mais estreito; não há evidência de continuação para um novo submit nesse caminho.
Decisão tomada: manter a lacuna como evidência induzida ausente e não introduzir mock infrastructure pesada.
Documentos atualizados: audit de 2026-08-28 e ROADMAP/TECH_DEBT após esta reconciliação.
```

## Evidência / referências

- `docs/AUDITS/2026-08-28-vulkan-waitidle-contract-revalidation.md`;
- `Game/Graphics/RendererCore.cpp`;
- `Game/Graphics/RendererFacade.cpp`;
- `Tests/Integration/test_renderer_core.cpp`;
- Study `2026-08-27-vulkan-recreate-waitidle-contract.md` como evidência histórica, sempre revalidada contra `main`.

## Fecho

**Resultado:** `parcialmente concluído`  
**Critério de saída:** semântica atual documentada; evidência induzida continua aberta  
**Dívida residual:** teste executável específico para `vkDeviceWaitIdle()` permanece em aberto
