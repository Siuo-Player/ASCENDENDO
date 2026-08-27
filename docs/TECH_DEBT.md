# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável. Severidade:

- **P0** — impede evoluir com segurança ou pode causar comportamento incorreto em runtime/release.
- **P1** — deve ser tratado antes das fases seguintes que dependem dele.
- **P2** — melhoria importante de manutenção/performance.
- **P3** — limpeza posterior.

## Estado do Gate 9.6

| Área | Estado atual | Evidência |
|---|---|---|
| GraphicsRuntime rollback | proven | PR #81 |
| VulkanContext terminal failure state | implemented / partial evidence | PR #87 |
| Windows compatibility | proven for selected runner | PR #85 |
| Tick-scoped input | implemented | PR #88 |
| Replay tick representation | proven for explicit TickInput sequence | PR #90 |
| Malformed current grammar | proven | PR #91 |
| Vulkan lower-level failure/error paths | partial | fault injection/capability matrix ainda limitada |
| Paths/CWD independence | open | runtime roots ainda não demonstrados |
| Level world/chunk metadata | open | contrato ainda não formalizado |
| Collision-order determinism | open | propriedade física ainda não escolhida |
| Presentation/domain boundary | partial | RenderSnapshot geral continua bloqueado |
| Architecture/ownership final review | open | revisão final pendente |

## P0 restantes

O Gate só fecha quando existir evidência suficiente para:

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

## Regras arquiteturais

1. O renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Runtime não depende de current working directory.
5. `RuntimeBootstrap` é composição de startup, não `Application` genérica.
6. `LevelDataIO` é parser/serializer, não semantic validator nem schema authority.
7. A ordem de `Level::platforms()` não é assumida irrelevante para determinismo.
8. Input edge para replay pertence ao tempo de simulação.
9. `ReplayManager` usa `TickInput`; isso não prova independência do live input face ao frame rate.
10. Causas de falha CI exigem evidência observável.

## Evidência recente

### Windows — PR #85

Build, testes, Vulkan software driver, campaign validation e artefacto foram demonstrados num runner Windows real. A conclusão limita-se ao ambiente selecionado.

### Replay — PR #90

`ReplayManager` armazena e reproduz `TickInput`; testes Linux, ASan/UBSan e Windows passaram no mesmo head. Persistence e live sampling continuam distintas.

### Malformed syntax — PR #91

A gramática atual rejeita token desconhecido, número inválido, campo truncado e trailing tokens e mantém um caso válido. Tests, Sanitizers e Windows passaram no mesmo head. Semantic validation, schema/versioning e canonicalization seguem para a Fase 10.

### Vulkan — PR #87

Falhas de reconfiguração são tratadas como terminais e handles/estado são normalizados. Fault injection de APIs inferiores continua limitada.

## Não avançar ainda

A migração geral de `RenderSnapshot` permanece bloqueada até a revisão formal e fecho do Gate 9.6.
