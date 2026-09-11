# Crítica externa e resposta — ASCENDENDO

**Data:** 2026-09-11
**Origem:** sessão de revisão de código por Claude (Anthropic), em chat, a pedido do
proprietário do repositório. Não nasceu de dentro do projecto — é uma leitura externa do
repositório público, sem acesso a contexto que não estivesse no próprio `git clone`.
**Âmbito:** repositório público `Siuo-Player/ASCENDENDO`, observado em dois momentos: antes
(commit `c2819e3`, 1014 commits, 2026-09-10) e depois (commit `4f8102f`, 1032 commits,
2026-09-11) de uma consolidação de documentação feita em resposta à primeira leitura.

Nota de proveniência: este ficheiro foi escrito fora do repositório privado
`Siuo-Player-PROJECT-STUDIES` porque o agente que o produziu não tinha credenciais para lá
aceder directamente. Foi entregue como ficheiro solto para ser colocado manualmente em
`ASCENDENDO/` dentro desse repositório — confirmar que o nome/local final seguem a convenção
já usada lá, que o agente não pôde ver.

## 1. Porque é que este documento existe

`docs/` dentro do ASCENDENDO tem de ser curto e correcto — não é o sítio para uma análise de
processo com data e hipóteses. Este ficheiro é precisamente isso: um registo do que uma
revisão externa encontrou, do que o projecto mudou em resposta, e do que se repetiu mesmo
depois de corrigido. O valor está em poder comparar previsão com realidade mais tarde — por
isso não deve ser reescrito se as coisas melhorarem; escreve-se um estudo novo que o referencia.

## 2. Achados da primeira leitura (1014 commits)

- **Documentação > produto.** 174 ficheiros `.md`, 15 418 linhas, mais do que as 10 870 linhas de código do jogo em `Game/`.
- **README desactualizado de forma material.** Afirmava "Fase 9.3" enquanto `main` já tinha `PlatformCompositor`, `GameState::CAMPAIGN_EDITOR`, `GraphicsRuntime`/`PresentationRuntime` — nenhum mencionado. Não apontava para onde estava o estado real.
- **Infra-estrutura à frente de conteúdo, de forma extrema.** `PlatformCompositor` (811 linhas de C++, ~20 PRs) construído para 3 níveis com 4 plataformas cada (12 no jogo inteiro). Um "Platform Asset Candidate Registry" com formulários de aprovação formal para tiles cujo `runtime_path`/`content_sha256` estavam todos por preencher — zero tiles de terreno reais chegaram a ser descarregados.
- **Burocracia de processo desproporcional.** 23 documentos de auditoria em 5 dias. Post-mortems formais (com hipóteses rejeitadas e "regra permanente") para um documento commitado directo em `main` por engano e para uma falha `403` transitória do `actions/upload-artifact` que se resolveu sozinho ao repetir o job.
- **Ritmo de commits incompatível com trabalho humano manual.** 1014 commits, um autor, 228 num único dia (28 de agosto). Rácio `docs:`/`feat:` ≈ 2,7:1. Commits `probe`/`placeholder` directo em `main`, sem branch.
- **Instruções de setup que não reproduziam o build real.** `external/glfw/lib-vc2022/` (biblioteca compilada) não estava no repositório; o `README.md` sugeria descarregar e extrair o GLFW 3.4 genérico, o que não produz a lib esperada — só o CI reconstruía isso correctamente, a partir de um commit fixado do GLFW com runtime MSVC específico.
- **Créditos inconsistentes.** `README.md` só creditava Claude; o ecrã de créditos do próprio jogo já mencionava Claude e Gemini.

## 3. Resposta do projecto (18 commits, mesmo dia/dia seguinte)

- `docs/00-meta/` (todos os `STATUS_*`, `CURRENT_STATUS_*`, `ROADMAP_*` datados, e os post-mortems de incidente) removido por inteiro. Documentação: 174→46 ficheiros `.md`, 15 418→4 736 linhas.
- `docs/ROADMAP.md` novo, único documento vivo de planeamento, com uma secção "O que não fazer" que corresponde, quase ponto a ponto, às recomendações da leitura original: não criar snapshots datados, não expandir o compositor por antecipação, não tratar cada bug/PR como post-mortem, não usar percentagens de código/documentação como progresso.
- Campanha expandida de 3 para 25 níveis jogáveis.
- `docs/AI_CREDITS.md` criado — credita Claude, Gemini, **e** um terceiro assistente (ChatGPT/GPT-5.6 Luna) não mencionado antes em lado nenhum visível externamente.
- `docs/DEVELOPMENT_PROTOCOL.md` (310 linhas — sequência obrigatória, protocolo de falhas de CI, contrato de work package, política de dependências) removido por inteiro na mesma consolidação. Substituído por `docs/CONTRIBUTING.md`, mais curto, cobrindo só branches/PR/TDD. Não ficou claro se o resto do conteúdo foi deliberadamente descartado ou apenas perdido na consolidação.

## 4. O que se repetiu, na própria correcção (2ª leitura, mesmo dia)

- `README.md` passou a dizer "campanha actualmente com 15 níveis" — mas `campaign.txt`, editado mais tarde na mesma sessão, já tinha 25. O mesmo tipo de atraso entre documento e código que motivou a leitura original, só que em horas em vez de meses.
- `Development/dev_log.txt` — o diário append-only que o próprio projecto trata como obrigatório por sessão — não recebeu **nenhuma** entrada nesta sessão, apesar de ter consolidado ~130 ficheiros e quadruplicado o conteúdo jogável.
- Dois commits directos em `main`, 6 segundos entre si: `chore: placeholder` e `revert accidental placeholder on main`, ambos a tocar um ficheiro `.keep` vazio. Mesmo padrão de ruído (`probe`/`placeholder`) da primeira leitura, não resolvido.
- `docs/PROGRESS_MODEL_1_0.md`: dimensão "Conteúdo" continuou em ~35% antes e depois de a campanha ir de 3 para 25 níveis — ou o número não foi recalculado, ou o alvo implícito cresceu ao mesmo tempo (novo alvo declarado: 575 níveis / 10 campanhas), tornando a métrica pouco informativa nos dois casos.
- `docs/CI.md` descreve a política de `docs/CODE_SIZE.md` como baseada em linhas (`<300`/`300–399`/`>=400`); `docs/CODE_SIZE.md`, escrito/tocado na mesma consolidação, continua a definir o gate em KiB físicos (`<40`/`40–47.99`/`>=48`) — que é o que `check_source_sizes.py` de facto aplica. Dois documentos da mesma sessão já não concordam.
- `docs/02-engineering/README.md` refere um `99-history` como destino de snapshots históricos; essa pasta não existe no repositório.
- Novo alvo de conteúdo (575 níveis, distribuição 10/25/25/25/25/50/50/50/100/250) definido com precisão alta mas sem dados de velocidade de produção ou playtesting a sustentá-lo — na mesma frase que rejeita "percentagens artificiais" para outra coisa (dificuldade). Combinado com a proibição explícita de novas mecânicas, é uma aposta de design de alto risco de repetição, não só uma questão de gestão.

## 5. Padrão

A correcção resolveu o problema de **escala** de forma genuinamente rápida e substancial —
menos ficheiros, menos teatro processual, menos percentagens a fingir rigor. Não resolveu o
**hábito** por trás do problema: a distância entre o que os documentos afirmam e o que o
repositório contém continua a abrir-se sozinha, sem que ninguém o faça de propósito, e o
ritmo de commits (18 num dia, vários por um único autor com apoio de três assistentes de IA
diferentes) é rápido demais para sincronização manual disciplinada sem um mecanismo que force
essa sincronização — ver `AGENTS.md` na raiz do `ASCENDENDO`, que tenta ser esse mecanismo.

Isto não anula o progresso desta ronda. Anula a leitura de que consolidar ficheiros, por si só,
resolveu a causa.

## 6. Como usar isto mais tarde

Quando a campanha crescer de facto para perto de 575 níveis, ou quando `AGENTS.md` tiver sido
seguido durante vários ciclos, escrever um estudo novo (`AAAA-MM-DD-...md`) que revisite os
pontos da secção 4 um a um: continuam a acontecer, pararam, ou mudaram de forma? Não editar
este ficheiro para reflectir esse resultado — referenciá-lo a partir do novo.
