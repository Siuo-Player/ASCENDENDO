# Plano da branch atual

**Branch:** `docs/9-4-foundation`

**Base:** `main` após integração do PR #1 (`dd034f8`).

## Já concluído antes desta branch

- Fases 1–8 do roadmap.
- Fase 9.1: ações lógicas e bindings.
- Fase 9.2: rato, viewport lógico e menus clicáveis.
- Fase 9.3: estado `EDITOR`, câmera livre e grelha.
- Correção de bootstrap GLFW/Vulkan e limpeza de recursos Vulkan em falhas parciais.
- Remoção de output de testes versionado que era artefacto gerado.

## Objetivo desta branch

Transformar a documentação existente num sistema de documentação técnica coerente antes de continuar a implementação da 9.4.

## Implementado nesta branch

- índice da documentação em `docs/README.md`;
- arquitetura e responsabilidades em `docs/ARCHITECTURE.md`;
- formato de `.lvl` e `campaign.txt` em `docs/LEVEL_FORMAT.md`;
- especificação de interação, grid, entidades, sprites e bindings do editor em `docs/LEVEL_EDITOR.md`;
- estratégia de testes e validação em `docs/TESTING.md`;
- roadmap consolidado em `docs/ROADMAP.md`;
- workflow de branches/PRs/TDD em `docs/CONTRIBUTING.md`.

## Critério de conclusão

A documentação desta branch fica concluída quando estes documentos estiverem coerentes entre si e o estado do projeto, sem declarar como concluída uma fase que ainda depende de validação real.

## Próxima branch

Depois desta PR ser integrada, a próxima branch deve concentrar-se exclusivamente na primeira tranche executável da 9.4: modelo de documento/entidade e lógica de grid/snap, com testes unitários. A UI Vulkan fica para depois de o modelo determinístico estar sólido.