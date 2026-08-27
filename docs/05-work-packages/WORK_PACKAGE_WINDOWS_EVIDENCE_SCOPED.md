# Work Package — Windows portability evidence (scoped)

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `CI / Windows / build portability`  
**Work Package:** `9.6 Windows executable evidence — scoped`  
**Branch:** `fix/9-6-windows-evidence-scope-clean`  
**Base:** `main` @ `e4e1061b0b7d18f31c0893e898f04ed54c94fa25`

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

- [ ] workflow Windows reproduzível;
- [ ] `make game` verde;
- [ ] `make tests` verde;
- [ ] campanha verde;
- [ ] software Vulkan demonstrado;
- [ ] evidência preservada;
- [ ] documentação canónica atualizada;
- [ ] PR pronta para merge somente com base nos resultados observados.

## Alterações durante execução

Registar cada incompatibilidade Windows encontrada no runner antes da respetiva correção. Não incluir alterações Linux/generalistas nesta branch.

## Fecho

A tranche fecha apenas a evidência Windows. CI hardening geral permanece como trabalho separado, sem ser usado para inflar artificialmente a conclusão do Gate 9.6.
