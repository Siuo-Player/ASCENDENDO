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

Além disso, o Makefile referencia `external/glfw/lib-vc2022`, mas esse artefacto pré-compilado não está presente no repositório. A futura CI deve tornar a origem/resolução dessa dependência explícita e reproduzível.

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
→ make game
→ make tests
```

## Consumidores

`Makefile`, `run_tests_windows.cmd`, `.github/workflows/*`, developers Windows e futura portable release.

## Critério de evidência

A primeira execução deve preservar:

```text
OS image
compiler version
make version
Vulkan SDK version/path
GLFW source/version or resolved artefact
GLSL compiler version
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
