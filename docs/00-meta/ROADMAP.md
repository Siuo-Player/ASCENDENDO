# ASCENDENDO — Roadmap operacional vivo

**Fonte canónica:** este ficheiro. Os restantes roadmaps/status datados são histórico, não estado actual.

## Regra de prioridade

```text
bloqueio real do jogador
→ conteúdo jogável
→ validação física
→ playtest humano
→ apresentação/arte
→ performance
→ release
```

A fundação existente é suficiente para produzir jogo. Não abrir novas camadas de infraestrutura sem uma necessidade demonstrada pelo conteúdo ou por um bug reproduzível.

## 1. Conteúdo — prioridade imediata

A campanha activa tem agora **15 níveis** (`inicio.lvl` … `nivel_15.lvl`). Os níveis 10–15 acrescentam uma segunda tranche de conteúdo, usando apenas o formato `.lvl` e as regras físicas existentes.

**Definition of done por nível:**

```text
entra em campaign.txt
→ passa ai_validator.py --campaign
→ carrega no runtime
→ é jogado manualmente
→ falhas e problemas de leitura ficam registados como evidência de produto
```

O validador físico é um filtro de segurança, não uma prova de diversão, dificuldade adequada ou completabilidade humana.

## 2. Design de níveis — agora com variedade real

A próxima tranche deve variar distância, largura, ritmo, recuperação e legibilidade. Não basta aumentar a quantidade repetindo o mesmo zig-zag. O Commitment Jump de 60° mantém-se como mecânica central; não criar novas mecânicas só para mascarar falta de conteúdo.

## 3. Playtest humano — gate antes de nova arquitectura

A prioridade seguinte é obter runs de pessoas fora do desenvolvimento. Medir apenas sinais que possam mudar o jogo: onde falham, onde hesitam, duração, mortes/restarts, compreensão da rota e vontade de tentar novamente.

Sem evidência humana, não declarar a mecânica “divertida”, a dificuldade “correcta” ou a apresentação “legível”.

## 4. Editor — consolidar, não recomeçar

`EDITOR` e `CAMPAIGN_EDITOR` já existem. Trabalhar neles apenas quando a autoria dos 15 níveis revelar uma lacuna concreta em preview, colocação, save, undo/redo ou validação.

## 5. Apresentação e assets — usar o que existe

O compositor e a cadeia de apresentação existentes devem ser exercitados pelo conteúdo actual, não ampliados preventivamente.

Para assets externos, a cadeia mínima é:

```text
ficheiro concreto
→ licença/proveniência
→ dimensões
→ SHA-256
→ revisão visual
→ integração
```

Um candidato sem binário concreto continua a ser investigação, nunca “asset integrado”.

## 6. Engenharia — só por evidência

Só corrigir/expandir a fundação quando existir um problema reproduzível, uma falha de integração ou uma medição que justifique a alteração. Profiling precede optimização; incidentes transitórios permanecem no dev log em vez de gerar post-mortems novos.

## 7. Release

A ordem é:

```text
campanha coerente
→ playtest externo
→ assets concretos
→ instalação simples
→ build reproduzível
→ release Windows
→ avaliar Linux/macOS com base em necessidade real
```

## Métricas de produto

- níveis activos jogáveis;
- minutos de gameplay únicos;
- percentagem de campanha completável;
- jogadores externos que completam a campanha;
- bugs de gameplay reproduzíveis.

Commits, PRs, documentos e auditorias não são métricas de progresso do jogo.

## Anti-fragmentação

Não criar novos roadmaps, status snapshots ou post-mortems por rotina. Este é o documento vivo; o histórico fica no Git.
