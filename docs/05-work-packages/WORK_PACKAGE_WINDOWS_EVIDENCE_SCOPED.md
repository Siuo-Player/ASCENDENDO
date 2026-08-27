# Work Package — Windows portability evidence (scoped)

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `CI / Windows / build portability`  
**Work Package:** `9.6 Windows executable evidence — scoped`  
**Branch:** `fix/9-6-windows-evidence-scope-clean`  
**Base:** `main` @ `38f589e43600c79c9abf7a787f6b4bec59506f07`

## Objetivo

Demonstrar de forma executável se o baseline atual do ASCENDENDO consegue construir e executar a validação Windows prevista no Gate 9.6, sem misturar hardening geral de CI.

## Inclui

- workflow Windows;
- aquisição explícita das dependências necessárias ao Makefile atual;
- provisionamento de Vulkan software driver e verificação do ICD;
- `make clean`, `make game`, `make tests` e campanha em Windows;
- correções estritamente necessárias de consumidores Windows já demonstradas pelos runners;
- preservação da evidência específica desta execução.

## Não inclui

- Linux workflow cleanup;
- sanitizer workflow redesign;
- política global de `-Werror`;
- modernização geral de GitHub Actions;
- warning cleanup não necessário ao Windows gate;
- redesign de gameplay/rendering.

## Dependências

```text
Windows runner
→ GNU Make
→ Clang/LLVM compatível com MSVC STL
→ Visual Studio/MSVC environment
→ Vulkan SDK
→ Vulkan software ICD
→ GLFW
→ matching CRT/ABI
→ make game
→ make tests
→ campaign validation
```

## Consumidores

`Makefile`, `Development/Tools/run_tests_windows.cmd`, Windows workflow e futuros mantenedores da matriz Windows.

## Dependências de validação

A execução deve preservar pelo menos:

```text
runner image
compiler
make
Vulkan SDK
Vulkan ICD
GLFW resolved commit
CRT model
make game result
make tests result
campaign result
failure diagnostics
```

## Decisões arquiteturais

A tranche não altera a arquitetura de runtime. Qualquer correção de código deve resultar diretamente de uma falha observada no caminho Windows e permanecer confinada à interface/compatibilidade necessária.

## Riscos

- diferenças entre versões do runner Windows;
- ABI/CRT entre `clang++` e biblioteca GLFW;
- disponibilidade e localização do Vulkan software ICD;
- ausência de cobertura universal para todos os ambientes Windows.

## Definition of Ready

- [x] Studies/ASCENDENDO verificados;
- [x] PR #83 auditada quanto a scope creep;
- [x] baseline `main` confirmado;
- [x] dependências Windows necessárias identificadas;
- [x] failure modes anteriores classificados por evidência.

## Definition of Done

- [x] workflow Windows executado com sucesso no runner selecionado;
- [x] `make game` verde;
- [x] `make tests` verde;
- [x] campanha verde;
- [x] software Vulkan demonstrado via `vulkaninfoSDK.exe`/Lavapipe;
- [x] evidência preservada como artifact;
- [x] documentação desta tranche atualizada;
- [ ] PR pronta para merge após revisão final de scope/base e classificação das evidências.

## Resultado observado — 2026-08-27

A execução Windows `#99` foi concluída com sucesso no runner `windows-2025-vs2026`, imagem `20260824.214.3`.

Ambiente demonstrado:

```text
Windows Server 2025
Visual Studio 18.9.1 / MSVC 14.51.36231
Clang 20.1.8 — x86_64-pc-windows-msvc
GNU Make 4.4.1
CMake 4.4.2
Vulkan SDK 1.4.309.0
Lavapipe/Mesa 25.2.5
GLFW commit 7b6aead9fb88b3623e3b3725ebb42670cbe4c579
```

`vulkaninfoSDK.exe` demonstrou um ICD Lavapipe/llvmpipe ativo, Vulkan Instance Version 1.3.301 e device `llvmpipe` com driver Mesa 25.2.5.

Resultado da validação:

```text
source-size checks   ✅
make game             ✅
make tests             ✅
213 test cases         ✅
1166 assertions        ✅
campaign validation     ✅
artifact upload        ✅
```

O artifact `windows-build-test-evidence` foi finalizado com os cinco ficheiros previstos.

## Warnings não bloqueantes observados

1. GitHub Actions reporta que `ilammy/msvc-dev-cmd@v1` e `KyleMayes/install-llvm-action@v2.0.9` ainda têm runtime Node 20 e estão a ser executadas sob Node 24. Isto é dívida futura de CI/tooling, não falha desta tranche.
2. O loader Vulkan reporta `Registry lookup failed to get layer manifest files`. O ICD explicitamente selecionado funciona e toda a suite passa; o warning permanece registado como observação de ambiente, não como bug confirmado do runtime.
3. `RuntimePaths.cpp` emite warning de `std::getenv` deprecated no CRT Windows. Não foi corrigido nesta tranche porque não foi necessário para o gate e pertence ao futuro warning/toolchain cleanup.
4. `external/stb/stb_image.h` produz warnings de funções não usadas. São provenientes de terceiro e não devem ser corrigidos alterando o vendor source dentro deste WP.

Nenhum destes warnings invalida a evidência observada deste runner, mas devem permanecer visíveis para trabalho futuro.

## Alterações durante execução

Foram observadas e corrigidas nesta branch, antes do resultado final:

- incompatibilidade inicial do generator CMake com o Visual Studio 18 do runner;
- incompatibilidade do Clang inicial com o MSVC STL;
- incompatibilidade de CRT/ABI no link;
- caminho incorreto para `tests.exe`;
- resolução do ICD Lavapipe;
- nome real do executável `vulkaninfoSDK.exe`.

Cada correção foi derivada de um failure log observável do runner, sem atribuição causal especulativa.

## Fecho

A propriedade **Windows build + full test/campaign path no runner selecionado** está demonstrada por evidência reproduzida numa execução de CI com environment manifest e artifact preservado.

Isto **não** demonstra compatibilidade universal com todos os ambientes Windows e **não fecha o Gate 9.6 global**. Permanecem como critérios independentes: Vulkan failure/recovery evidence, tick-exact replay determinism, malformed/error-path policy e arquitetura/ownership final review.

CI hardening geral permanece como work package separado.
