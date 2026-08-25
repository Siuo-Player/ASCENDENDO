# Continuous Integration

O ASCENDENDO usa GitHub Actions para validar o projeto antes de integrar alterações em `main`.

## Política de diagnóstico

Uma falha CI deve ser tratada por evidência:

```text
Run → Job → Step → log → classificação → causa confirmada → correção → nova execução
```

Um step agregado que falha sem diagnóstico acessível não autoriza concluir qual componente está errado. A causa deve permanecer `UNKNOWN` até existir evidência suficiente.

## Pipeline atualmente integrado em `main`

`.github/workflows/tests.yml` executa em `ubuntu-latest`:

1. checkout do repositório;
2. instalação de Clang, GNU Make, Vulkan de desenvolvimento, GLFW, `glslc`, Xvfb, `vulkan-tools` e drivers Vulkan Mesa;
3. `Development/Tools/check_source_sizes.py`;
4. seleção explícita do ICD Vulkan de software `lavapipe`, seguida de `vulkaninfo --summary`;
5. um step agregado que executa `make clean`, `make game` e `make tests-verbose` dentro de Xvfb;
6. `python3 Development/AI_Validation/ai_validator.py --campaign`.

A separação futura em steps independentes de build/teste/campaign é uma melhoria de observabilidade e deve ser validada como alteração de workflow própria. Não deve ser descrita como já integrada em `main` sem prova no workflow.

## Incidente atual — 2026-08-25

Run #281 (`32879936455`) falhou no step agregado **Build and run tests in virtual X display**. A verificação de tamanho não foi a causa da falha desse run.

A causa detalhada não está confirmada porque o log diagnóstico não ficou acessível através da interface GitHub utilizada para a auditoria.

```text
Run: #281
Classificação: build/test failure
Causa confirmada: UNKNOWN
Ação: não atribuir causalidade específica sem log
```

Esta informação deve ser atualizada assim que o diagnóstico ficar disponível.

## Incidente histórico de source-size

Run #251 encontrou um ficheiro em erro e quatro em warning. Esse resultado é tratado como dívida de modularidade real; o response correto é analisar coesão/coupling e criar work packages de subdivisão, não simplesmente aumentar o limite.

## Source-size enforcement

A política normativa atual está em `docs/CODE_SIZE.md`: `<300` linhas normal, `300–399` warning, `>=400` error.

**Estado real de `main`:** o checker ainda usa KiB (`30/36 KiB`) e percorre apenas `Game/` e `Tests/`. `main.cpp` ainda não é incluído.

A migração do checker e a validação do novo workflow são tarefas de implementação posteriores a esta documentação. Até serem concluídas, qualquer afirmação de que o novo gate já bloqueia por número de linhas seria incorreta.

## Validação local

No Windows, o Makefile seleciona recipes compatíveis com `cmd.exe`, evitando que PowerShell/Git Bash herdem comandos POSIX incorretos. A validação local continua útil para o ambiente Vulkan/driver do computador, mas o CI é a referência para a compilação Linux headless.

## Limitações

O CI Linux usa Vulkan por software e não substitui testes com GPU física Windows. Sanitizers e Windows build/tests continuam trabalho de roadmap e devem possuir jobs explícitos quando forem promovidos a gates.
