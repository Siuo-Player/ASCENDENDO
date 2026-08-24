# Plano da branch atual

**Branch:** `feat/9-4-editor-ui-integration`

**Base:** `main` após integração da PR #6 (`55e4a0a`).

## Estado herdado — concluído

- Fases 1–8.
- 9.1–9.3.
- bootstrap GLFW/Vulkan e cleanup de falhas parciais.
- documentação técnica inicial.
- 9.4 tranche 1: modelo determinístico do editor, grid/snap, plataformas, spawn, FLAG e testes.
- 9.4 tranche 2: `EditorInteractionController`, cursor logical→world, hit-test, STAMP/DRAG, seleção, movimento e delete com testes sem GPU.
- requisito de produto: EXE portable Windows x64, offline-first, import/export e futura biblioteca online com validação local obrigatória.

O plano da branch anterior (`feat/9-4-editor-ui-v2`) fica **concluído**.

## Objetivo desta branch

Garantir que a infraestrutura de build/testes funciona de forma consistente no Windows e no CI, e fechar as correções de robustez encontradas antes da integração visual do editor.

## Implementado até agora nesta branch

- correção da regra de dimensão mínima de plataformas: dimensão inválida é rejeitada antes do snap;
- Makefile com comandos de criação/remoção de diretórios adequados ao Windows e Linux;
- targets `tests`, `tests-fast` e `tests-verbose` sem depender de `./`/`cat` no Windows;
- separação dos comandos de execução/leitura/erro por plataforma;
- GitHub Actions para compilar e executar os testes em Ubuntu;
- execução headless dos testes gráficos através de Xvfb + Mesa Vulkan software;
- seleção explícita do ICD Vulkan de software `lavapipe` no CI para tornar o ambiente headless determinístico;
- `vulkaninfo --summary` no CI para falhar cedo quando o driver Vulkan de software não estiver disponível;
- execução do CI num display X virtual com resolução fixa;
- GitHub Actions para validar toda a campanha ativa com `ai_validator.py --campaign`;
- remoção de warning morto no `EditorInteractionController`;
- correção da impressão de resultados de testes falhados no Makefile, mantendo-a síncrona;
- remoção dos logs locais gerados do repositório.

## Resultado da validação conhecida

A primeira execução local encontrou corretamente o bug da plataforma mínima (`3x2` aceite indevidamente). Depois da correção, uma execução local ainda não passou porque o Makefile antigo usava comandos POSIX (`./`, `cat`, `mkdir -p`, `rm -rf`) sob o shell Windows.

A primeira execução CI compilou todo o projeto e confirmou os testes da nova camada do editor, mas falhou nos testes gráficos/Vulkan porque o runner Ubuntu não tinha display nem ambiente Vulkan headless configurado. O workflow foi então atualizado para usar Xvfb e Mesa software.

A execução CI #8, com o ICD Vulkan de software explicitamente selecionado, terminou **com sucesso**. Está a ser feita uma nova execução após a última correção do Makefile, que não altera a lógica do jogo e serve para validar a tranche final com o estado exato da branch.

## Ordem interna restante

1. Confirmar CI verde no estado final da branch.
2. Rever warnings restantes que pertençam ao código do projeto.
3. Atualizar documentação final da tranche e critérios de aceitação.
4. Merge desta tranche.
5. Criar a branch seguinte dedicada à integração visual da 9.4.

## Não entra nesta branch

- integração visual completa do editor;
- save/serialização;
- validação assíncrona do editor;
- import/export de pacotes;
- biblioteca online/site;
- networking;
- campanha/playlist 9.6;
- sistema final de sprites do editor.

## Próxima branch após esta

Depois de esta tranche ser integrada, criar uma branch nova dedicada à **integração visual da 9.4**: renderização de entidades, cursor/preview, STAMP, DRAG, seleção, movimento, botão direito e feedback visual. O `docs/BRANCH_PLAN.md` dessa nova branch deve substituir este plano e marcar esta tranche como concluída.
