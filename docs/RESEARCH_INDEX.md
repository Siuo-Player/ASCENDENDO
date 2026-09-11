# Mapa de investigação — ASCENDENDO

Cada decisão importante consulta a investigação correspondente. A bibliografia detalhada está em `docs/SCIENTIFIC_REFERENCES.md`; este ficheiro é apenas o mapa de ligação entre perguntas do projecto e linhas de evidência.

| Subsistema | Questão | Referências principais | Aplicação futura |
|---|---|---|---|
| Level design | Como compor desafios 2D? | Smith et al. 2008; Khalifa et al. 2019 | vocabulário de design, análise do editor |
| Jump design | O salto é executável e com que margem? | Aramini et al. 2018 | alcance, margem, diagnóstico |
| Player simulation | Como procurar soluções de platformer? | Tremblay et al. 2014 | PES, replay analysis, shortcut detection |
| Difficulty | O conteúdo é fisicamente válido, executivamente difícil e percebido como difícil? | Francillette 2025; Fernández 2018; CORGIS 2020 | score em camadas, expectativa vs realidade |
| Challenge/rhythm | Como representar ritmo e desafio? | Sorenson et al. 2011; Khalifa et al. 2019 | secções, safe zones, pacing |
| Player experience | Como relacionar nível, comportamento e experiência? | Pedersen et al. 2009, 2010 | telemetria e playtesting |
| Player modelling | Como estimar capacidade sem confundir desempenho com skill? | Shaker et al. 2010; Madineni 2025 | skill proxies, aprendizagem, incerteza |
| Adaptive difficulty | Quando e como adaptar? | Lopes & Bidarra 2011; DDA review 2024 | fase posterior, só após calibração |
| Procedural generation | Como gerar sem perder autoria/qualidade? | Togelius et al. 2011; Yannakakis & Togelius 2011; Summerville et al. 2018 | PCG, repair, critique |
| Authoring tools | Como misturar autor e assistência? | Tanagra 2010/2011 | constraints e mixed-initiative |
| Game feel | Como preservar previsibilidade e melhorar feedback? | Designing Game Feel 2022 | tuning, feedback audiovisual, input support |
| Progression | Como construir campanhas longas? | Khalifa et al. 2019; Pedersen et al. 2010; Sorenson et al. 2011 | progressão e variedade |
| Camera | Que informação deve permanecer visível? | camera-control literature + platformer analyses | framing, lookahead, dead zones |

## Hierarquia de evidência

Quando existem conflitos:

1. evidência científica diretamente aplicável ao problema;
2. documentação técnica oficial da tecnologia;
3. experiência comprovada de jogos/ferramentas maduros;
4. testes internos, profiling e telemetria;
5. preferência estética/pessoal.

Isto não significa que um paper “vence” o playtest. A investigação dá-nos uma hipótese melhor; o jogo tem de confirmar se ela funciona para a nossa física e público.

## Regra de múltiplas fontes

Nenhuma decisão importante de level design, dificuldade ou UX deve ser justificada por uma única fonte.

Para dificuldade, o modelo de investigação é:

```text
motor / física determinística
        +
modelo de salto / perigo
        +
player simulation
        +
telemetria de desempenho
        +
playtesting / experiência percebida
        ↓
modelo de dificuldade ASCENDENDO
```

Para PCG:

```text
conteúdo authored
        ↓
representação semântica
        ↓
função de qualidade
        ↓
geração / repair
        ↓
validação
        ↓
avaliação humana
```

## Limite epistemológico

Uma métrica publicada é uma **hipótese de medição**, não um threshold do ASCENDENDO. Qualquer nova fórmula ou classe de dificuldade deve passar por:

```text
literatura → protótipo → simulação → playtest → calibração → integração
```

O mesmo vale para PES: um solver pode demonstrar existência e custo de uma solução sob execução perfeita, mas isso não é equivalente à dificuldade humana.

## Documento principal

- `docs/SCIENTIFIC_REFERENCES.md` — referências, limitações e implicações práticas.
- `docs/ROADMAP.md` — ordem de implementação.
