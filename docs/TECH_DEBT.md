# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável.

## Gate 9.6 — estado

**OPEN**

### Evidência integrada

- PR #81 — rollback agregado de `GraphicsRuntime`.
- PR #87 — falhas de reconfiguração de `VulkanContext` tratadas como terminais; lower-level fault injection continua parcial.
- PR #85 — Windows build/test evidence em runner real com Vulkan software driver.
- PR #88 — `TickInput` como unidade semântica da simulação.
- PR #90 — `ReplayManager` alinhado a `TickInput`; prova reprodução de sequência explícita de ticks, não persistence nem live-input frame-rate independence.
- PR #92 — malformed syntax da gramática atual coberta por testes para token desconhecido, número inválido, campo truncado e trailing tokens. PR #91 foi superseded.

### Gaps restantes

- capability/error evidence Vulkan além do happy path, incluindo teste/failure-path evidence específico para `vkDeviceWaitIdle()`;
- paths/runtime roots independentes do current working directory;
- contrato world/chunk metadata de `Level`;
- determinismo de múltiplos contactos/collision-order;
- semantic validation/schema/versioning de `LevelData` (Fase 10);
- revisão final de ownership/architecture.

## Regras

1. Renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Runtime não depende de current working directory.
5. `RuntimeBootstrap` é composição de startup, não `Application` genérica.
6. `LevelDataIO` é parser/serializer, não schema authority nem semantic validator.
7. Ordem de `Level::platforms()` não é assumida irrelevante para determinismo.
8. Input edge para replay pertence ao tempo de simulação.
9. `ReplayManager` usa `TickInput`; isso não prova live-input frame-rate independence.
10. Causas de falha CI exigem evidência observável.
11. A ausência de um fault-injection seam não pode ser tratada como prova de que um failure path passou; implementation semantics e executable evidence permanecem estados distintos.

## GitHub merge gate — CI e proteção de `main`

O repositório é público e o fluxo normal é:

```text
feature branch
    → pull request para `main`
    → CI
    → review
    → merge
    → delete branch
```

### Checks candidatos ao merge gate

Após auditoria dos workflows atuais, os checks obrigatórios mínimos são os três jobs abaixo. Os nomes são os nomes reais dos jobs GitHub Actions, não os nomes dos workflows:

```text
Linux / Clang / C++20 / Headless Vulkan
Linux / Clang / ASan + UBSan / Headless Vulkan
Windows / Clang / C++20
```

`tests.yml`, `sanitizers.yml` e `windows.yml` têm `pull_request` limitado à branch `main`, permissões mínimas (`contents: read`) e não usam path filters/condições que façam estes jobs desaparecer de uma PR normal para `main`.

O workflow `.github/workflows/actions-smoke.yml` é deliberadamente apenas manual (`workflow_dispatch`) e não faz parte do merge gate.

### O que cada check valida

- `Linux / Clang / C++20 / Headless Vulkan`: build, testes completos, políticas de tamanho de fonte e validação da campanha sob Vulkan headless/software.
- `Linux / Clang / ASan + UBSan / Headless Vulkan`: suite instrumentada com AddressSanitizer e UndefinedBehaviorSanitizer, além da validação da campanha.
- `Windows / Clang / C++20`: build Windows, suite completa, validação da campanha e recolha dos artefactos de evidência.

Runs reais recentes associados a uma PR para `main` produziram os três job checks acima com `success`.

### Política administrativa

O ruleset existente `Protect main` deve permanecer como o mecanismo de proteção da branch. A política pretendida é:

- enforcement `Active`;
- pull request obrigatória antes do merge;
- 1 aprovação obrigatória;
- dismiss de aprovações stale quando aplicável;
- resolução das conversas obrigatória;
- os três checks acima como required status checks;
- exigir branch atualizada apenas se compatível com os checks selecionados;
- impedir force-push e eliminação de `main`;
- sem pushes diretos como fluxo normal de desenvolvimento.

A configuração efetiva do ruleset é uma propriedade do GitHub e deve ser verificada contra o estado real da API/UI, não apenas contra esta documentação.

## Fecho do Gate

Antes da migração geral de `RenderSnapshot`, deve existir evidência suficiente para:

```text
Vulkan failure/error semantics
+
queue/capability assumptions
+
deterministic simulation/replay
+
Windows build/test
+
malformed/error paths
+
architecture/ownership review
```
