# Mapa de investigação — ASCENDENDO

O projeto é dividido em subsistemas. Cada decisão importante deve consultar a investigação correspondente, em vez de procurar uma única "fonte mestre".

| Subsistema | Questão | Referências principais | Aplicação futura |
|---|---|---|---|
| Level design | Como compor desafios 2D? | Smith/Cha/Whitehead; Khalifa et al. | vocabulário de design, análise do editor |
| Jump design | O salto é executável e com que margem? | Aramini/Lanzi/Loiacono | alcance, margem, diagnóstico |
| Difficulty | O conteúdo é fisicamente válido, executivamente difícil e percebido como difícil? | Francillette et al. 2025; Pedersen; Fernández | score de dificuldade por camadas, expectativa vs realidade |
| Motor difficulty | Quais perigos e margens tornam um nível difícil para executar? | Francillette et al. 2025 | danger zones, hazards, risco/margem, análise automática |
| Player modelling | Como o desempenho do jogador deve informar uma estimativa de capacidade? | Madineni 2025 + futura telemetria própria | skill proxies, aprendizagem, incerteza |
| Adaptive difficulty | Como adaptar sem degradar a experiência ou favorecer apenas certos perfis? | Madineni 2025 + player experience literature | fase posterior, só após calibração |
| Progression | Como construir campanhas longas? | level-design patterns; learning-progress research | 50–200 níveis sem monotonia |
| User Generated Content | Como autores trabalham e aprendem? | Super Mario Maker studies; SuperTux | onboarding, feedback, metadata |
| Extreme levels | Como distinguir desafio intencional de erro? | Johnson 2019; Mario Maker studies | classificação e comunicação de dificuldade |
| Authoring tools | Como misturar autor e assistência? | Tanagra | futura assistência com constraints |
| Playtesting | Como testar sem depender só de humanos? | automated regression; smart playtesting | agentes, replay regression |
| Camera | Que informação deve permanecer visível? | camera-control literature + platformer analyses | framing, lookahead, dead zones |
| Physics | Como preservar determinismo? | engine/replay/testing practice | fixed timestep, replay |
| C++ architecture | Como manter desempenho sem sobre-engineering? | coupling/cohesion/size research; architecture knowledge management | boundaries, ownership, testability |
| Software architecture knowledge | Como preservar decisões e rationale? | Kruchten/Lago/van Vliet; AKM literature | ADRs/architecture docs, reviewability |
| Project management | Como decompor trabalho sem perder a ligação à arquitetura? | Tausworthe WBS; software-engineering WBS practice | WBS, work packages, PR gates |
| Dependency management | Como tornar dependências e coordenação visíveis? | Cataldo/Herbsleb/Carley; Begel/Nagappan; Bick et al.; Sablis et al. | dependency maps, coordination awareness |
| Modularity | Quando uma fronteira reduz de facto a complexidade? | socio-technical congruence; modularity/architecture research | coupling, subsystem boundaries |
| Architectural technical debt | Como evitar acumulação silenciosa de decisões subótimas? | ATD multiple-case studies; TD management studies | TECH_DEBT, refactoring gates |
| Vulkan | Como otimizar sem quebrar sincronização? | Khronos spec/samples | barriers, queues, profiling |
| Hardware | Como suportar GPUs diferentes? | Vulkan portability/version docs | feature detection, fallbacks |
| Testing | Como detetar regressões? | game testing literature | CI, replay, hardware matrix |
| Packaging | Como distribuir sem ambiente de desenvolvimento? | Microsoft deployment docs | portable archive/installer |
| Sharing | Como transferir conteúdo não confiável? | OWASP; resumable upload protocols | import/export/site |
| Community | Como gerir conteúdo e metadata? | UGC/Mario Maker research | IDs, stats, reports, moderation |
| Base audit | Como fechar contradições entre arquitetura e implementação? | `docs/BASE_ARCHITECTURE_AUDIT.md` + referências técnicas | hardening antes de features |

## Hierarquia de evidência

Quando existem conflitos:

1. evidência científica diretamente aplicável ao problema;
2. documentação técnica oficial da tecnologia;
3. experiência comprovada de jogos/ferramentas maduros;
4. testes internos, profiling e telemetria;
5. preferência estética/pessoal.

Isto não significa que um paper "vence" o playtest. A investigação dá-nos uma hipótese melhor; o jogo tem de confirmar se ela funciona para a nossa física e público.

Para questões de processo, arquitetura e gestão de dependências, a literatura deve ser usada da mesma forma: como base para hipóteses e mecanismos verificáveis, não como receita rígida.

## Regra de múltiplas fontes

Nenhuma decisão importante de level design ou UX deve ser justificada por apenas um jogo.

Por exemplo:

```text
Mario Maker
   + papers de player experience
   + papers de level design
   + testes internos
   = decisão ASCENDENDO
```

O mesmo princípio vale para Vulkan, arquitetura e web.

Para planeamento:

```text
WBS / project management research
        +
architecture / dependency research
        +
repository history / current code
        +
CI + tests
        ↓
work package ASCENDENDO
```

Para dificuldade:

```text
motor / física determinística
        +
modelo de perigo/dificuldade executiva
        +
telemetria de desempenho
        +
playtesting / experiência percebida
        ↓
modelo de dificuldade ASCENDENDO
```

## Novas referências de 2025

### Francillette et al. — dificuldade executiva em platformers

**A Comprehensive Model of Automated Evaluation of Difficulty in Platformer Games**
DOI: `10.1145/3705013`

O trabalho propõe avaliação automática de dificuldade game-centric/executiva por zonas de perigo estáticas e perigos dinâmicos. Foi validado nos níveis de Super Mario Bros. Para o ASCENDENDO, a principal contribuição é a separação conceptual entre validade do conteúdo e dificuldade executiva; as fórmulas e thresholds não são assumidos como válidos para a nossa física sem calibração interna.

### Madineni — ajuste de dificuldade e player modelling

**Analyzing Player Difficulty Perception in Platformers Through Procedural Level Generation**
California Polytechnic State University, 2025.

A dissertação estuda ajuste de dificuldade em tempo real num platformer procedural e encontra desempenho desigual entre jogadores mais confortáveis e menos confortáveis com platformers. É uma fonte útil para a fase futura de player modelling, mas não é suficiente para definir uma política universal de adaptive difficulty.

## Conteúdo oficial pretendido

O objetivo editorial registado no roadmap é ter várias campanhas:

- campanhas normais: ~50 níveis em média;
- campanhas especiais: 100–200 níveis;
- níveis muito difíceis/extremos;
- conteúdo comunitário em escala muito superior.

O número de níveis deve influenciar arquitetura, análise e tooling, mas não deve ser usado como objetivo vazio de quantidade.

## Documentos relacionados

- `docs/SCIENTIFIC_REFERENCES.md` — level design, platformers, difficulty, player experience e playtesting.
- `docs/TECHNICAL_REFERENCES.md` — C++, Vulkan, profiling, hardware, packaging e web sharing.
- `docs/DESIGN_REFERENCES.md` — Tiled, Godot, SuperTux, Jump King e Mario Maker como referências práticas.
- `docs/BASE_ARCHITECTURE_AUDIT.md` — auditoria atual da base, prioridades P0/P1/P2 e critérios de saída.
- `docs/PROJECT_MANAGEMENT.md` — WBS, work packages, dependency awareness, Definition of Ready/Done e relação arquitetura↔roadmap.
- `docs/PRODUCT_DECISIONS.md` — decisões do autor.
- `docs/ROADMAP.md` — ordem de implementação e gates.
