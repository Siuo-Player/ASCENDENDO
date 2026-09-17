# Continuous Integration

O ASCENDENDO usa GitHub Actions para validar o projeto antes de integrar alterações em `main`.

## Política de diagnóstico

Uma falha CI deve ser tratada por evidência:

```text
Run → Job → Step → log → classificação → causa confirmada → correção → nova execução
```

Um step agregado que falha sem diagnóstico acessível não autoriza concluir qual componente está errado. A causa deve permanecer `UNKNOWN` até existir evidência suficiente.

## Gates atualmente integrados em `main`

A validação relevante do repositório está distribuída por workflows explícitos:

```text
Tests
├── Linux / Clang / C++20 / Headless Vulkan
└── Linux / Clang / ASan + UBSan / Headless Vulkan

Windows
└── Windows / Clang / C++20

Deterministic Capture Evidence
└── capturas headless em 4:3, 16:9 e 21:9, incluindo níveis 0–2

Campaign Validation
└── validação mecânica da campanha + relatório de dificuldade
```

`Tests` valida também source-size policy, assets/registos canónicos, catálogo da campanha, estado do repositório e a campanha ativa. O job normal executa `make clean`, `make game` e `make tests` em Xvfb com Vulkan software; o job ASan/UBSan executa a mesma suite com instrumentação.

O workflow Windows constrói o jogo e a suite de testes num ambiente Clang/MSVC com Vulkan software. O workflow de captura verifica que o build de release produz resultados determinísticos em diferentes aspect ratios. O workflow de campanha valida a campanha ativa independentemente do renderer.

## Gate Gameplay 1.0 — issue #273

O contrato de gameplay é `spawn → inputs → FLAG`, sem dependência do editor.

A evidência executável do loop completo está em:

`Tests/Acceptance/test_stable_gameplay_loop.cpp`

Esse acceptance test conduz o `GameSession` normal com input semântico e fixed-step physics real. Verifica, no mesmo run:

- spawn derivado do ground implícito e estado `grounded` inicial;
- carga contínua e força de salto calculada pelo runtime;
- três saltos reais numa rota fisicamente alcançável;
- streaming vertical através do caminho normal de `GameSession`;
- `FLAG` derivada do nível final e atingida por colisão normal;
- transição para `CREDITS` após conclusão;
- gravação do run;
- ausência de dependência do editor.

A implementação mantém a regra de ausência de air control: `Player::applyHorizontalMovement()` só altera velocidade horizontal enquanto o corpo está grounded; no lançamento do salto, `vx` e `vy` são definidos pela força de salto e pelo ângulo de 60°. A carga é linear em `jumpCharge` e limitada a `1.0`.

O contador `elapsedTime_` no `GameSession` avança pelos fixed steps realmente simulados. O loop de gameplay pode regressar voluntariamente ao menu através da ação de quit sem depender do editor.

O PR #364 integrou esta evidência em `main` após validação verde de Linux normal, ASan/UBSan, Windows, captura determinística e campaign validation. A integração não substitui novas execuções de CI para alterações futuras: qualquer novo HEAD deve ser validado novamente antes de merge.

## Validação local

No Windows, o Makefile seleciona recipes compatíveis com `cmd.exe`, evitando que PowerShell/Git Bash herdem comandos POSIX incorretos. A validação local continua útil para o ambiente Vulkan/driver do computador, mas o CI é a referência para a compilação Linux headless e para a matriz Windows.

## Source-size enforcement

A política normativa está em `docs/CODE_SIZE.md`. O checker atual é a autoridade executável; esta documentação não deve antecipar regras que ainda não estejam presentes no workflow.

## Limitações

Vulkan software não substitui testes com GPU física. Capturas determinísticas provam reprodutibilidade da apresentação suportada pelo workflow, não qualidade visual subjetiva. Validação mecânica de campanha prova invariantes estruturais/físicos, não diversão ou adequação de dificuldade.
