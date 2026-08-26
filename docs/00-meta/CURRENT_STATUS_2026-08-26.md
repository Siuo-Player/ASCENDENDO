# ASCENDENDO — Estado operacional — 26/08/2026

## Base hardening integrado

As seguintes tranches estão integradas em `main` e validadas por CI Linux/Clang/headless Vulkan:

- #41/#42/#43 — `GameStateMachine`, contrato de transições e wiring no entry point.
- #44/#46 — `SimulationOrchestrator` e wiring do fixed-step.
- #48 — ownership RAII de `RendererFacade` com `std::unique_ptr`.
- #49 — lifetime RAII de GLFW e política única de source-size em KiB (40 KiB warning / 48 KiB hard limit).
- #51 — separação de graphics/present queue families e seleção de device/surface compatível.
- #52 — isolamento dos ficheiros temporários dos testes de `Level`.
- #54/#55 — fronteira `RuntimePaths` e integração no `main.cpp`.

## Próxima fronteira

O `main.cpp` ainda concentra bootstrap gráfico e composição de serviços. A decomposição seguinte deve retirar responsabilidades por propriedade real, sem criar uma `Application` monolítica.

Prioridade imediata:

1. separar bootstrap/composição gráfica mantendo ownership explícita;
2. tornar os caminhos de erro de criação de Vulkan observáveis/testáveis;
3. rever acquire/reset/submit/present e recriação de swapchain;
4. depois rever o Base Engineering Gate e preparar a migração geral de `RenderSnapshot`.

## Investigação atualizada

A pasta `ASCENDENDO` de `PROJECT-STUDIES` recebeu em 26/08 uma nova varredura de papers e recomendações. A regra operacional mantém-se: usar a investigação para definir propriedades e experiências verificáveis, não para impor nomes de classes ou padrões arquiteturais arbitrários.

O material novo sobre avaliação de PCG, diversidade/qualidade/controllability e avaliação por agentes é relevante para fases posteriores de conteúdo e análise; não deve ultrapassar o hardening da base.

## Evidence boundary

O CI Linux/headless confirma compilação, testes e validação da campanha nesse ambiente. Windows, sanitizers e uma matriz de hardware continuam sem evidência equivalente.
