# Work Package — Windows portability evidence

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `CI / Windows / build portability`  
**Work Package:** `9.6 Windows executable evidence`  
**Branch:** `fix/9-6-windows-portability-evidence`  
**Base:** `main` @ `38f589e43600c79c9abf7a787f6b4bec59506f07`

## Objetivo

Transformar o suporte Windows documentado em evidência executável e reproduzível, sem presumir incompatibilidade nem alterar o runtime.

## Descobertas e correções observadas

O `Makefile` já possui uma via Windows, mas o repositório não continha `external/glfw/lib-vc2022`. A CI tornou a aquisição e compilação da dependência explícitas.

O runner `windows-2025-vs2026` expôs Visual Studio 18/2026. O workflow foi alinhado com esse generator. A STL do runner rejeitou Clang 19 (`STL1000`), levando à fixação de LLVM/Clang 20.1.8.

Uma tentativa de usar `/MD` diretamente com o driver GNU-style `clang++` falhou porque a opção foi tratada como nome de ficheiro. A seleção de runtime passou a usar a opção nativa de Clang `-fms-runtime-lib=dll`, mantendo o GLFW em `MultiThreadedDLL`.

A execução posterior chegou a compilar e linkar `game.exe` e o executável dos testes, mas `make tests` falhou no tooling Windows: o `Makefile` produz `build\\test\\tests.exe`, enquanto `Development/Tools/run_tests_windows.cmd` procurava `build\\tests.exe`. Isto é um mismatch de contrato entre consumidor e produtor de testes, não uma falha do runtime Windows.

Também foi observado durante a auditoria de CI que warnings de terceiros e warnings de infraestrutura não devem ser confundidos com warnings do código do projeto. O workflow passou a preservar diagnósticos, e os cabeçalhos de `external/` são tratados como dependência externa.

## Inclui

- workflow Windows;
- aquisição/resolução explícita das dependências necessárias;
- `make clean`, `make game`, `make tests` em Windows;
- preservação de logs e versões de ambiente suficientes para diagnóstico;
- full test mode, não apenas `tests-fast`;
- documentação da evidência obtida ou das falhas observadas;
- verificação de contratos entre `Makefile` e ferramentas de teste.

## Não inclui

- substituir GNU Make por CMake;
- Windows sanitizer obrigatório nesta tranche;
- provar compatibilidade universal com todos os drivers/GPU Windows;
- alterações no código de gameplay/renderer sem evidência específica.

## Dependências

```text
Windows runner
→ LLVM / clang++ / llvm-ar
→ GNU Make
→ Vulkan SDK
→ glslc
→ GLFW resolvível
→ matching MSVC CRT
→ make game
→ make tests
→ campaign validation
```

## Consumidores

`Makefile`, `run_tests_windows.cmd`, `.github/workflows/*`, developers Windows e futura portable release.

## Critério de evidência

A execução deve preservar:

```text
OS image
compiler version
make version
Vulkan SDK version/path
GLFW source/version or resolved artefact
GLSL compiler version
CRT model for game/GLFW
make game result
make tests result
campaign validation result
relevant warnings/diagnostics
```

Uma execução verde demonstra compatibilidade observada para aquele ambiente. Não demonstra cobertura de todos os ambientes Windows.

## Definition of Ready

- [x] Study PR #10 consultada;
- [x] Makefile Windows inspecionado;
- [x] runner Windows inspecionado;
- [x] ausência de CI Windows confirmada;
- [x] GLFW prebuilt path tratado como dependência explícita;
- [x] não-equivalência entre documentação e evidência executável registada.

## Definition of Done

- [ ] workflow Windows reproduzível;
- [ ] dependências resolvidas de forma explícita;
- [ ] `make game` verde;
- [ ] `make tests` verde;
- [ ] logs/artifacts preservados para diagnóstico;
- [ ] tooling Windows usa o mesmo caminho de teste produzido pelo Makefile;
- [ ] documentação canónica atualizada;
- [ ] Gate Windows classificado por evidência real.
