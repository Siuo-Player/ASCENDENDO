# Mapa de investigação — ASCENDENDO

O projeto é dividido em subsistemas. Cada decisão importante deve consultar a investigação correspondente, em vez de procurar uma única "fonte mestre".

| Subsistema | Questão | Referências principais | Aplicação futura |
|---|---|---|---|
| Level design | Como compor desafios 2D? | Smith/Cha/Whitehead; Khalifa et al. | vocabulário de design, análise do editor |
| Jump design | O salto é executável e com que margem? | Aramini/Lanzi/Loiacono | alcance, margem, diagnóstico |
| Difficulty | Difícil para o motor ≠ difícil para o jogador | Pedersen; Fernández; Scientific Reports 2025 | score de dificuldade, expectativa vs realidade |
| Progression | Como construir campanhas longas? | level-design patterns; learning-progress research | 50–200 níveis sem monotonia |
| User Generated Content | Como autores trabalham e aprendem? | Super Mario Maker studies; SuperTux | onboarding, feedback, metadata |
| Extreme levels | Como distinguir desafio intencional de erro? | Johnson 2019; Mario Maker studies | classificação e comunicação de dificuldade |
| Authoring tools | Como misturar autor e assistência? | Tanagra | futura assistência com constraints |
| Playtesting | Como testar sem depender só de humanos? | automated regression; smart playtesting | agentes, replay regression |
| Camera | Que informação deve permanecer visível? | camera-control literature + platformer analyses | framing, lookahead, dead zones |
| Physics | Como preservar determinismo? | engine/replay/testing practice | fixed timestep, replay |
| C++ architecture | Como manter desempenho sem sobre-engineering? | data-oriented/ECS research | layouts de dados e hot paths |
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
2. documentação técnica oficial da tecnologia utilizada;
3. experiência comprovada de jogos/ferramentas maduros;
4. testes internos, profiling e telemetria;
5. preferência estética/pessoal.

Isto não significa que um paper "vence" o playtest. A investigação dá-nos uma hipótese melhor; o jogo tem de confirmar se ela funciona para a nossa física e público.

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
- `docs/PRODUCT_DECISIONS.md` — decisões do autor.
- `docs/ROADMAP.md` — ordem de implementação e gates.
