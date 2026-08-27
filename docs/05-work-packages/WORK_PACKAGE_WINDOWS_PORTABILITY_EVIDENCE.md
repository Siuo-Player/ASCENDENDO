# Work Package — Windows portability evidence

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `CI / Windows / build portability`  
**Work Package:** `9.6 Windows executable evidence`  
**Branch:** `fix/9-6-windows-portability-evidence`  
**Base:** `main` @ `38f589e43600c79c9abf7a787f6b4bec59506f07`

## Objetivo

Transformar o suporte Windows documentado em evidência executável e reproduzível, sem presumir incompatibilidade nem alterar o runtime.

## Descoberta

O `Makefile` já possui uma via Windows, e `Development/Tools/run_tests_windows.cmd` executa `build\\tests.exe` quando esse binário existe. Contudo, o workflow atual é exclusivamente Linux e não existe evidência de `make game` + `make tests` em Windows.

Além disso, o Makefile referencia `external/glfw/lib-vc2022`, mas esse artefacto pré-compilado não está presente no repositório. A CI torna agora a origem/resolução dessa dependência explícita e reproduzível.

A primeira execução encontrou uma incompatibilidade de toolchain: o runner `windows-2025-vs2026` expunha MSVC STL que rejeitava Clang 19 (`STL1000`). O workflow foi então fixado em LLVM/Clang 20.1.8.

A execução seguinte confirmou que GLFW 3.4 podia ser construído no runner com Visual Studio 18/2026, mas o `Makefile` falhou antes do link porque a flag `/MD` foi interpretada pelo driver GNU-style `clang++` como um nome de ficheiro (`no such file or directory: '/MD'`). Esta execução **não estabeleceu uma incompatibilidade CRT entre GLFW e o jogo**. A correção é usar a opção nativa de Clang `-fms-runtime-lib=dll`, equivalente ao runtime DLL `/MD`, enquanto o GLFW continua a ser construído com `CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`.

## Inclui

- workflow Windows inicial;
- aquisição/resolução explícita das dependências necessárias;
- `make clean`, `make game`, `make tests` em Windows;
- preservação de logs e versões de ambiente suficientes para diagnóstico;
- full test mode, não apenas `tests-fast`;
- documentação da evidência obtida ou da falha observada.

## Não inclui

- reescrever o Makefile sem necessidade observada;
- substituir GNU Make por CMake;
- Windows sanitizer obrigatório nesta tranche;
- provar compatibilidade universal com todos os drivers/GPU Windows;
- alterações no código de gameplay/renderer.

## Dependências

```text
Windows runner
→ LLVM / clang++ / llvm-ar
→ GNU Make
→ Vulkan SDK
→ glslc
→ GLFW resolvível
→ matching MSVC CRT (/MD)
→ make game
→ make tests
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
- [ ] documentação canónica atualizada;
- [ ] Gate Windows classificado por evidência real.
