# Documentação técnica do ASCENDENDO

Esta pasta complementa o `README.md` com documentação que precisa de mais detalhe do que a visão geral do projeto.

## Documentos

- [Arquitetura](ARCHITECTURE.md) — responsabilidades dos módulos e fluxo de execução.
- [Formato de níveis](LEVEL_FORMAT.md) — sintaxe de `.lvl`, `campaign.txt`, regras de coordenadas e invariantes.
- [Editor de níveis](LEVEL_EDITOR.md) — decisões de UX da Fase 9, grid, bindings e modelo de edição.
- [Testes e validação](TESTING.md) — build, testes unitários/integração e validação física.
- [Roadmap](ROADMAP.md) — estado atual, histórico das fases e próximos passos aprovados.
- [Contribuição](CONTRIBUTING.md) — fluxo de branches, PRs, TDD e regras de alteração.
- [Plano da branch atual](BRANCH_PLAN.md) — o que já está concluído e o objetivo específico desta branch.

## Regra de documentação

Cada nova branch de trabalho deve atualizar a documentação de estado antes da implementação principal. O documento da branch deve dizer:

1. o que já estava concluído quando a branch abriu;
2. o que a branch vai implementar;
3. critérios objetivos de conclusão;
4. o que fica explicitamente para a branch seguinte.

Ao abrir uma nova branch, o plano anterior deixa de ser "planeamento futuro" e passa a constar como concluído, ou é substituído pelo estado efetivamente alcançado.