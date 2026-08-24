# Continuous Integration

O ASCENDENDO usa GitHub Actions para validar o projeto antes de integrar alterações em `main`.

## Pipeline atual

`.github/workflows/tests.yml` executa em `ubuntu-latest`:

1. checkout do repositório;
2. instalação de Clang, GNU Make, Vulkan de desenvolvimento, GLFW, `glslc`, Xvfb e os drivers Vulkan Mesa;
3. `make clean` e `make tests-verbose` dentro de um display X virtual;
4. execução de Vulkan através do driver de software disponível no runner;
5. `python3 Development/AI_Validation/ai_validator.py --campaign`.

O objetivo é que os testes gráficos também possam ser exercitados num runner sem GPU física, usando uma sessão gráfica virtual e um driver Vulkan de software.

Um PR para `main` só deve ser considerado pronto quando o workflow estiver verde.

## Validação local

No Windows, o Makefile seleciona recipes compatíveis com o shell `cmd.exe`, que é o shell usado pelo GNU Make neste ambiente. Isto evita que PowerShell/Git Bash herdem comandos POSIX incorretos como `cat`, `./...`, `mkdir -p` ou `rm -rf`.

A validação local continua útil para testar o ambiente Vulkan/driver específico do computador, mas o CI é a referência para a compilação e testes determinísticos e para o smoke test gráfico headless.

## Limitações atuais

O CI Linux usa Vulkan por software e não substitui um teste de execução no GPU físico de Windows. Esse teste será tratado separadamente na validação de release e no smoke test da versão portable.
