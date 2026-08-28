# Work Package — Runtime-root independence evidence

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsistema:** `Runtime / Process bootstrap`  
**Work Package:** `9.6 Runtime-root independence evidence`  
**Branch:** `test/9-6-runtime-root-independence-20260828`  
**PR:** `#105` — merged

## Objetivo

Demonstrar por teste executável que a resolução de `RuntimePaths::fromProcess()` para recursos relativos ao executável não depende do current working directory nos ambientes suportados pelo projeto.

## Escopo

### Inclui

- executar `RuntimePaths::fromProcess(nullptr)` a partir de dois CWDs diferentes;
- comparar executable root e caminhos derivados de assets/levels/sprite;
- manter a política atual de resolução baseada no executável como contrato observado.

### Não inclui

- escolha de uma futura política de packaging;
- suporte a resource bundles novos;
- alteração de `RuntimePaths`;
- alteração de `RuntimeBootstrap`;
- mudança do formato de níveis;
- refactor genérico de `main.cpp`.

## Dependências

### Depende de

- `RuntimePaths` atual;
- `RuntimeBootstrap` existente;
- documentação de Gate 9.6 e Studies.

### Produz para

- Gate 9.6 runtime-root evidence;
- futura revisão de deployment/packaging.

### Consumidores afetados

- `Tests/Unit/test_runtime_paths.cpp`;
- documentação do Gate.

### Dependências de validação

- Linux normal;
- ASan/UBSan;
- Windows Clang/Vulkan CI.

## Decisões arquiteturais

```text
Problema/contexto:
O Gate exige independência de CWD, mas a policy de resource-root futura ainda é uma decisão de produto.

Decisão:
Provar apenas o comportamento atual: fromProcess() usa o root do executável nos ambientes suportados e os caminhos derivados não variam com o CWD.

Alternativas consideradas:
Declarar CWD como contrato; introduzir project-root configurável; adicionar abstração de packaging.

Consequências:
Fortalece a garantia atual sem antecipar uma decisão de distribuição/packaging.

Condição de revisão/remoção:
Mudança futura da estratégia de distribuição ou requirement de resource bundles configuráveis.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| Teste validar apenas uma plataforma | baixa | médio | executar a matriz Linux/ASan/Windows | mitigado |
| Fallback de `argv0` continuar dependente de CWD | média | médio | documentar que o contrato testado é o caminho normal de `fromProcess()` nos targets suportados | aberto |
| Confundir root do executável com política de packaging universal | média | médio | limitar explicitamente a alegação | mitigado |

## Validação

### Testes automatizados

- Linux normal: **success**, 230/230 test cases, 1318/1318 assertions;
- Linux ASan/UBSan: **success**, 230/230 test cases, 1318/1318 assertions;
- Windows: **success**, 230/230 test cases, 1318/1318 assertions; campaign validation `[OK]` para os níveis ativos; Windows evidence artifact produzido.

### Validação manual

- inspeção de `RuntimePaths.cpp` para os caminhos Windows/Linux/macOS e fallback por `argv0`.

### Profiling / métricas

- não aplicável.

### Failure paths

- não é uma failure-path tranche; demonstra invariância de resolução em condições normais.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [x] teste passa em Linux normal;
- [x] teste passa em ASan/UBSan;
- [x] teste passa em Windows;
- [x] comportamento observado documentado;
- [x] limitation sobre fallback/packaging preservada;
- [x] documentação normativa atualizada em PR #106;
- [x] PR #105 integrada.

## Alterações durante a execução

```text
Descoberta:
RuntimePaths::fromProcess() usa APIs específicas do processo em Linux/Windows/macOS antes do fallback argv0.

Impacto:
O teste pode caracterizar o comportamento normal sem alterar produção.

Decisão tomada:
Adicionar apenas teste executável e nenhuma nova abstração.

Resultado:
Dois CWDs diferentes produziram o mesmo executable root e os mesmos caminhos derivados na matriz CI.

Limitação preservada:
Isto não estabelece uma política universal de packaging/deployment e não elimina a análise do fallback argv0 em cenários que não foram exercitados.
```

## Evidência / referências

- `Game/Core/RuntimePaths.cpp`;
- `Game/Core/RuntimePaths.h`;
- `Game/Core/RuntimeBootstrap.cpp`;
- `Tests/Unit/test_runtime_paths.cpp`;
- PR #105;
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_DEEP.md`.

## Fecho

**Resultado:** `validated / bounded`  
**Critério de saída:** `mesmos roots/path derivados para dois CWDs sob fromProcess()`  
**Dívida residual:** `fallback argv0 e futura política de packaging continuam fora desta tranche`
