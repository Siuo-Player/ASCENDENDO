# Continuous Integration

O ASCENDENDO usa GitHub Actions para validar o projeto antes de integrar alterações em `main`.

## Pipeline atual

`.github/workflows/tests.yml` executa em `ubuntu-latest`:

1. checkout do repositório;
2. instalação de Clang, GNU Make, Vulkan SDK de desenvolvimento, GLFW e `glslc`;
3. `make clean && make tests-verbose -j2`;
4. `python3 Development/AI_Validation/ai_validator.py --campaign`.

Um PR para `main` só deve ser considerado pronto quando o workflow estiver verde.

## Validação local

No Windows, o Makefile seleciona recipes compatíveis com o shell `cmd.exe`, que é o shell usado pelo GNU Make neste ambiente. Isto evita que PowerShell/Git Bash herdem comandos POSIX incorretos como `cat`, `./...`, `mkdir -p` ou `rm -rf`.

A validação local continua útil para testar o ambiente Vulkan/driver específico do computador, mas o CI é a referência para a compilação e testes determinísticos de lógica.

## Limitações atuais

O CI Linux ainda não substitui um teste de execução gráfica real do renderer Vulkan num GPU Windows. Esse teste será tratado separadamente quando o pipeline de release e smoke tests gráficos forem implementados.
