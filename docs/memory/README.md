# Memória do projeto (ai-memory)

A fonte da verdade da memória do ASCENDENDO é **Markdown versionado em Git**, inspirado no padrão [ai-memory](https://github.com/akitaonrails/ai-memory). Qualquer agente ou desenvolvedor pode ler, pesquisar e atualizar estes ficheiros, mantendo a continuidade sem amnésia entre conversas e entre diferentes ferramentas (Antigravity, Claude Code, Cursor, etc.).

## Estrutura

- `decisions/` — Architecture Decision Records (ADRs): contexto, decisão, consequências. Decisões antigas não são apagadas; uma nova substitui ou anula a anterior.
- `gotchas/` — Armadilhas, bugs subtis do ambiente de compilação, física e sistemas operativos.
- `procedures/` — Procedimentos repetíveis de verificação, compilação e release.
- `sessions/` — Diário de bordo de cada sessão com trabalho realizado, testes aprovados e próximos passos.

## Regras

1. Escrever em Markdown simples e portável.
2. Não incluir segredos, tokens ou dados sensíveis.
3. Consultar `decisions/` e `gotchas/` ao iniciar uma sessão.
4. Registar nota de sessão em `sessions/` ao concluir uma tarefa.
