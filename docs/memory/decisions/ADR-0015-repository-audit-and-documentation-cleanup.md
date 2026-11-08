# ADR-0015: Auditoria Global do Repositório, Remoção de Ficheiros Obsoletos e Modernização de Tooling

## Estado
Aprovado

## Contexto
1. **Auditoria Exhaustiva do Repositório**: A pedido do utilizador, realizou-se uma varredura completa de todos os ficheiros do repositório em busca de documentos desatualizados, referências quebradas, relatórios órfãos e scripts a necessitar de modernização.
2. **Post-Mortem Órfão**: `docs/level-clear-finding.md` continha uma nota histórica sobre `Level::clear()` que já havia sido totalmente corrigida em `Game/Logic/Level.h`. Conforme o `AGENTS.md`, relatórios ou post-mortems pontuais não devem persistir na documentação viva.
3. **Hiperligações e Referências Mortas em `docs/`**: `docs/01-architecture/README.md`, `docs/02-engineering/README.md`, `docs/03-gameplay-editor/README.md`, `docs/CODE_SIZE.md`, `docs/CONTRIBUTING.md` e `docs/TECHNICAL_REFERENCES.md` continham referências a ficheiros legados já eliminados (`BASE_ARCHITECTURE_AUDIT.md`, `9.6_EDITOR_VIEWPORT.md`, `99-history`, `DEVELOPMENT_PROTOCOL.md`, `PROJECT_MANAGEMENT.md`, `BRANCH_PLAN.md`).
4. **Alinhamento de Dívida Técnica**: `docs/TECH_DEBT.md` listava ainda a persistência de replays, replay watch mode e segurança de contas como tarefas adiadas, quando já foram integralmente integradas.
5. **Apresentação e Tooling**: O site continha um erro tipográfico (`PILICKARES DO MOTOR`), `scripts/pre-push.sh` não validava a integridade do catálogo e repositório, e `reorganize.py` indexava pastas transitórias no mapa estrutural.

## Decisão
1. **Eliminação de Ficheiros Órfãos**:
   - Removido `docs/level-clear-finding.md`.
2. **Reconciliação Documental e Alinhamento com `AGENTS.md`**:
   - `docs/01-architecture/README.md`, `docs/02-engineering/README.md` e `docs/03-gameplay-editor/README.md` atualizados com ficheiros canónicos ativos (`SECURITY.md`, `LEVEL_STRUCTURE.md`, `PROGRESS_MODEL_1_0.md`, etc.).
   - `docs/CODE_SIZE.md`, `docs/CONTRIBUTING.md` e `docs/TECHNICAL_REFERENCES.md` atualizados para referenciar diretamente `AGENTS.md`, `docs/ROADMAP.md` e o padrão `docs/memory/` (ai-memory).
   - `docs/TECH_DEBT.md` atualizado com as dívidas recentemente concluídas.
   - `docs/memory/procedures/build-and-verify.md` atualizado para refletir a totalidade dos 494 testes C++ e 53 testes Python.
3. **Melhorias de Tooling e Apresentação**:
   - `site/index.html`: Corrigido o typo para `PILARES DO MOTOR`.
   - `scripts/pre-push.sh`: Modernizado para executar `validate_repository_state.py` e `validate_campaign_catalogue.py` além do validador físico.
   - `reorganize.py`: Otimizado para indexar o mapa em memória nativa do Python e filtrar explicitamente `.git`, `build`, `dist`, `.pytest_cache`, `__pycache__` e `.vscode`.
   - `Development/project_structure.txt`: Atualizado sem artefactos transitórios.

## Consequências
- A base documental fica 100% íntegra, sem referências quebradas ou ficheiros órfãos.
- O tooling de pre-push e estruturação previne ruído no histórico do Git.
- Todos os testes mantêm 100% de passagem.
