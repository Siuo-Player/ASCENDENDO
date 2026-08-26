# RoadMap status — 2026-08-26

Este documento é o **estado operacional atual** do `docs/ROADMAP.md`. O conteúdo histórico das fases permanece no RoadMap; esta folha evita reescrever histórico apenas para atualizar marcadores antigos.

## Fechado desde a última sincronização

- PR #41 — `GameStateMachine` isolada e ligada ao `main.cpp`.
- PR #42 — contrato de transições coberto por testes.
- PR #43 — wiring da `GameStateMachine` no entry point.
- PR #44 — `SimulationOrchestrator` criado e testado.
- PR #46 — wiring da simulação no `main.cpp`.
- PR #48 — ownership do `RendererFacade` convertido para `std::unique_ptr`.
- PR #49 — GLFW RAII e source-size policy canónica em KiB.
- PR #51 — graphics/present queue families separadas; `VK_KHR_swapchain` validada; presentation pela queue suportada; swapchain sharing correto quando necessário.
- PR #52 — isolamento dos ficheiros temporários dos testes de `Level`.

## Gate 9.6 — estado atual

```text
input/action consistency             ✅
fixed timestep / catch-up            ✅
GameStateMachine                     ✅
SimulationOrchestrator               ✅
RendererFacade ownership/RAII        ✅
GLFW lifetime                        ✅
source-size policy (KiB)             ✅
graphics/present queue separation   ✅
swapchain basic CI path              ✅
Level test temp isolation            ✅

main.cpp decomposition               🔄
RenderSnapshot geral                 🔒
Base Engineering Gate               🔒
```

## Próxima ordem de trabalho

1. Atualizar/fechar a documentação normativa do Gate para não manter branches históricas como trabalho corrente.
2. Separar runtime/user-data paths de `Development/` e do current working directory.
3. Continuar a decomposição do `main.cpp` por ownership real: bootstrap, carregamento de configuração/campanha e composição de serviços.
4. Rever swapchain error paths/recreation com testes específicos antes de fechar o Gate.
5. Rever CI transversal: Windows, sanitizers e capability matrix.
6. Só depois rever o bloqueio do `RenderSnapshot` geral.

## Regra

Antes de cada nova branch: consultar `PROJECT-STUDIES/ASCENDENDO`, verificar mudanças desde a última tranche, atualizar esta folha/documentação normativa quando necessário e só então implementar.
