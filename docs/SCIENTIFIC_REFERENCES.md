# Referências científicas — ASCENDENDO

Esta é a bibliografia de trabalho para as decisões de gameplay, level design, dificuldade, player modelling, autoria assistida e experiência do jogador. Não é uma coleção de citações decorativas: cada referência deve responder a uma pergunta do projecto e produzir uma hipótese testável.

A regra é separar quatro coisas:

```text
1. validade física
        ↓
2. dificuldade executiva do conteúdo
        ↓
3. desempenho observado do jogador
        ↓
4. dificuldade / experiência percebida
```

O ASCENDENDO acrescenta uma escala própria:

```text
salto → secção → nível → sequência de níveis → campanha
```

Nenhum paper externo define sozinho os thresholds do jogo. Resultados publicados são hipóteses de medição; só passam a contrato do ASCENDENDO depois de calibração com a nossa física determinística e playtests.

## 1. Level design específico de platformers

### Smith, Cha & Whitehead (2008) — A Framework for Analysis of 2D Platformer Levels

DOI: `10.1145/1401843.1401858`

Fornece uma linguagem para analisar níveis 2D segundo componentes e desafios próprios do género.

**Aplicação:** o nível deve poder ser decomposto em componentes de desafio, sequências e relações entre plataformas, e não apenas em uma lista de retângulos.

Fonte: [UCSC](https://eis.ucsc.edu/papers/smith-sandbox-08.pdf).

### Khalifa, de Mesentier Silva & Togelius (2019) — Level Design Patterns in 2D Games

DOI: `10.1109/CIG.2019.8847953`

Analisa jogos 2D e formaliza padrões recorrentes como **Guidance, Safe Zone, Foreshadowing, Layering, Branching** e **Pace Breaking**.

**Aplicação:** campanhas longas devem ter vocabulário de ritmo e aprendizagem, não apenas níveis progressivamente mais difíceis. O editor pode futuramente reconhecer padrões sem os impor.

Fonte: [Universidade de Malta](https://www.um.edu.mt/library/oar/handle/123456789/82005).

## 2. O salto como unidade mensurável

### Aramini, Lanzi & Loiacono (2018) — An Integrated Framework for AI Assisted Level Design in 2D Platformers

DOI: `10.1109/GEM.2018.8516490`

Trata a dificuldade de saltos individuais e a probabilidade de sucesso como objetos mensuráveis dentro de uma toolbox de level design.

**Aplicação:** o futuro diagnóstico de salto deve produzir métricas como distância horizontal, diferença vertical, janela/margem temporal, margem geométrica e razão de sucesso estimada, em vez de apenas `válido/falha`.

Fonte: [arXiv](https://arxiv.org/abs/1804.09153).

### Tremblay, Borodovski & Verbrugge (2014) — I Can Jump! Exploring Search Algorithms for Simulating Platformer Players

DOI: `10.1609/aiide.v10i3.12744`

Compara **A\***, **MCTS** e **RRT** para simular comportamento de jogadores em platformers e usar essas buscas como apoio iterativo ao design de níveis.

**Aplicação:** é a referência mais direta para um solver auxiliar de níveis. Sustenta o princípio do nosso **PES (Perfect-Execution Solver)**, mas não sustenta por si só o nosso algoritmo de busca exata por frame: o PES é uma especialização própria para a física determinística do ASCENDENDO.

Fonte: [McGill](https://www.cs.mcgill.ca/~jtremb59/Papers/ICanJump.pdf).

### Mourato, Santos & Birra (2011) — Automatic Level Generation for Platform Videogames Using Genetic Algorithms

DOI: `10.1145/2071423.2071433`

Estuda geração automática de níveis de platformers como problema de pesquisa/otimização, procurando maior expressividade e diversidade.

**Aplicação:** apoia a futura investigação de PCG, mas reforça que geração automática deve ficar subordinada à jogabilidade e à expressividade, não à quantidade de níveis produzidos.

Fonte: [RCAAP](http://hdl.handle.net/10400.26/6088).

## 3. Player experience em platformers

### Pedersen, Togelius & Yannakakis (2009) — Modeling Player Experience in Super Mario Bros

DOI: `10.1109/CIG.2009.5286482`

Relaciona parâmetros de level design, comportamento individual e experiência reportada, usando 480 sessões. Gap size/placement e mudanças de direção são exemplos de variáveis ligadas a **challenge, frustration** e **fun**.

**Aplicação:** dificuldade do ASCENDENDO não deve ser inferida apenas por contagem de plataformas. Devemos medir geometria dos saltos, mudanças de direção, margem de erro, tentativas e sequências de dificuldade.

Fonte: [University of Malta](https://www.um.edu.mt/library/oar/handle/123456789/22955).

### Pedersen, Togelius & Yannakakis (2010) — Modeling Player Experience for Content Creation

DOI: `10.1109/TCIAIG.2010.2043950`

Constrói modelos quantitativos de experiência a partir de métricas de gameplay e parâmetros de níveis, usando preference learning.

**Aplicação:** fornece fundamento para uma camada posterior que relacione features do nível e telemetria com experiência. Não justifica transformar esse modelo diretamente num score de qualidade do ASCENDENDO.

Fonte: [University of Malta](https://www.um.edu.mt/library/oar/handle/123456789/22957).

### Shaker, Yannakakis & Togelius (2010) — Towards Automatic Personalized Content Generation for Platform Games

DOI: `10.1609/aiide.6.12399`

Mostra geração personalizada de níveis a partir de modelos de experiência e estilos de jogo, com avaliação algorítmica e humana.

**Aplicação:** player modelling e personalização são possíveis, mas devem ficar depois da instrumentação e calibração da dificuldade do conteúdo.

Fonte: [AAAI AIIDE](https://ojs.aaai.org/index.php/AIIDE/article/view/12399).

### Denisova, Cairns, Guckelsberger & Zendle (2020) — Measuring Perceived Challenge in Digital Games: CORGIS

DOI: `10.1016/j.ijhcs.2019.102383`

Desenvolve e valida uma escala de desafio percebido com quatro dimensões: **cognitive, performative, emotional** e **decision-making challenge**, em três estudos com 1390 jogadores.

**Aplicação:** reforça que “dificuldade” não é uma única variável. O ASCENDENDO deve separar dificuldade física/executiva de dificuldade percebida e, nos playtests, medir pelo menos componentes de execução, decisão e carga cognitiva.

Fonte: [ScienceDirect](https://www.sciencedirect.com/science/article/pii/S1071581919301491).

## 4. Desafio e sensação de dificuldade

### Hamari et al. (2016) — Challenging Games Help Students Learn: An Empirical Study on Engagement, Flow and Immersion

DOI: `10.1016/j.chb.2015.07.045`

Em dois jogos de aprendizagem, challenge e skill estiveram associados a engagement/immersion; challenge foi um forte preditor dos resultados estudados.

**Aplicação:** é evidência complementar, não específica a platformers. Apoia a ideia de que progressão deve acompanhar o crescimento de capacidade do jogador, mas não define thresholds do ASCENDENDO.

Fonte: [ScienceDirect](https://www.sciencedirect.com/science/article/pii/S074756321530056X).

### Lemmens & von Münchhausen (2023) — Let the Beat Flow: How Game Difficulty in Virtual Reality Affects Flow

DOI: `10.1016/j.actpsy.2022.103812`

Num experimento com Beat Saber, a condição de difficulty/skill mais equilibrada produziu mais flow do que a condição demasiado difícil; dificuldade excessiva esteve associada a frustração.

**Aplicação:** reforça a necessidade de observar o jogador humano em vez de assumir que maximizar dificuldade melhora automaticamente a experiência.

Fonte: [ScienceDirect](https://www.sciencedirect.com/science/article/pii/S0001691822003274).

### Fernández, Mikami & Kondo (2018) — Perception of Difficulty in 2D Platformers Using Graph Grammars

DOI: `10.20668/adada.22.2_38`

Relaciona construção de níveis, estimativas numéricas e desempenho/perceção de dificuldade em 2D platformers.

**Aplicação:** mantém explicitamente a distinção:

```text
métrica física estimada ≠ dificuldade percebida
```

Fonte: [J-STAGE](https://www.jstage.jst.go.jp/article/adada/22/2/22_38/_article).

## 5. Dificuldade executiva e o pipeline de validação

### Francillette et al. (2025) — A Comprehensive Model of Automated Evaluation of Difficulty in Platformer Games

DOI: `10.1145/3705013`

Propõe um modelo automático de dificuldade **game-centric/executiva** para platformers, distinguindo dificuldade intrínseca do conteúdo de dificuldade dependente do jogador e experimentando a abordagem com níveis de Super Mario Bros.

**Aplicação:** sustenta uma camada intermédia entre o validator físico e a experiência humana:

```text
VALIDADE FÍSICA
      ↓
DIFICULDADE EXECUTIVA
      ↓
DESEMPENHO DO JOGADOR
      ↓
EXPERIÊNCIA PERCEBIDA
```

Não usamos as fórmulas nem thresholds diretamente: o domínio e a física do ASCENDENDO são diferentes.

Fonte: [ACM/DOI](https://doi.org/10.1145/3705013).

### Sorenson, Pasquier & DiPaola (2011) — A Generic Approach to Challenge Modeling for the Procedural Creation of Video Game Levels

DOI: `10.1109/TCIAIG.2011.2161310`

Combina modelação de challenge com geração evolutiva e constraint satisfaction; usa conceitos semanticamente significativos como dificuldade, skill e rhythm groups.

**Aplicação:** reforça que ritmo e desafio podem ser propriedades explícitas do nível, não apenas efeitos emergentes de geometria aleatória.

Fonte: [DBLP](https://dblp.org/rec/journals/tciaig/SorensonPD11).

## 6. Autoria assistida

### Smith, Whitehead & Mateas (2010/2011) — Tanagra

**Tanagra: A Mixed-Initiative Level Design Tool** — DOI `10.1145/1822348.1822376`.

**Tanagra: Reactive Planning and Constraint Solving for Mixed-Initiative Level Design** — DOI `10.1109/TCIAIG.2011.2159716`.

Tanagra combina autoria humana com constraints, planeamento e verificação de jogabilidade.

**Aplicação:** o editor do ASCENDENDO pode ganhar assistência sem perder autoria. Geometria authored, validação, análise de dificuldade e futura geração devem continuar separadas.

Fontes: [ACM](https://doi.org/10.1145/1822348.1822376), [IEEE](https://doi.org/10.1109/TCIAIG.2011.2159716).

## 7. Procedural Content Generation

### Togelius, Yannakakis, Stanley & Browne (2011) — Search-Based Procedural Content Generation: A Taxonomy and Survey

DOI: `10.1109/TCIAIG.2011.2148116`

Taxonomiza PCG por conteúdo, representação e função de avaliação e mostra a importância de definir fitness/quality explicitamente.

**Aplicação:** `playable = true` é restrição, não qualidade suficiente. Antes de PCG precisamos de critérios de design que tenham significado para o ASCENDENDO.

Fonte: [UCF](https://stars.library.ucf.edu/facultybib2010/2003/).

### Yannakakis & Togelius (2011) — Experience-Driven Procedural Content Generation

DOI: `10.1109/T-AFFC.2011.6`

Liga geração procedural a modelos de experiência e player modelling.

**Aplicação:** apoia uma futura arquitetura `gerador → avaliador → modelo de jogador`, mas não justifica introduzir geração adaptativa antes de existir conteúdo authored suficiente.

Fonte: [bibliografia do autor](https://yannakakis.net/publications/journal-papers/).

### Summerville et al. (2018) — Procedural Content Generation via Machine Learning

DOI: `10.1109/TG.2018.2846639`

Survey de PCGML para níveis, incluindo platformers, discutindo representação, dados, repair, critique, co-creativity e problemas como datasets pequenos e tuning.

**Aplicação:** não introduzir PCGML cedo. Primeiro é necessário um corpus de níveis authored e instrumentos para avaliar esse corpus.

Fonte: [arXiv](https://arxiv.org/abs/1702.00539).

### Risi & Togelius (2020) — Increasing Generality in Machine Learning Through Procedural Content Generation

DOI: `10.1038/s42256-020-0208-z`

Review que enquadra PCG como espaço para estudar generalização e adaptação de modelos.

**Aplicação:** útil para uma fase futura de geração/adaptação, sem transformar já a produção num problema de ML.

Fonte: [Nature Machine Intelligence](https://www.nature.com/articles/s42256-020-0208-z).

## 8. Player simulation e PES

### O que a literatura suporta

`I Can Jump!`, Aramini et al. e a literatura de PCG suportam três ideias:

1. um solver pode verificar se existe uma solução;
2. diferentes algoritmos podem explorar soluções de platformer;
3. um solver pode ajudar o designer a encontrar defeitos ou shortcuts.

O **PES — Perfect-Execution Solver** do ASCENDENDO é uma especialização do projecto: procura soluções sob a nossa simulação determinística, com inputs exatos por frame, e usa o resultado para medir propriedades do conteúdo.

### O que o PES não prova

```text
PES encontra solução
      ≠
humano consegue aprender a solução
      ≠
humano acha a solução legível
      ≠
humano considera o nível divertido
```

Por isso, `T_OPT`, `T_BEST` e `T_RUN` devem permanecer métricas descritivas e nunca ser tratados como equivalentes automáticos de dificuldade humana.

## 9. Player modelling e adaptive difficulty

### Madineni (2025) — Analyzing Player Difficulty Perception in Platformers Through Procedural Level Generation

Master's Thesis, California Polytechnic State University, San Luis Obispo, 2025.

Estuda ajuste de dificuldade em tempo real num platformer procedural e mostra que a eficácia da adaptação pode variar com a experiência/competência inicial do jogador.

**Aplicação:** primeiro separar `dificuldade do conteúdo` de `desempenho do jogador`; só depois estudar adaptação. Não usar uma regra simples como `falhou → tornar fácil`.

Fonte: [Cal Poly Digital Commons](https://digitalcommons.calpoly.edu/theses/3068/).

### Lopes & Bidarra (2011) — Adaptivity Challenges in Games and Simulations: A Survey

DOI: `10.1109/TCIAIG.2011.2152841`

Survey sobre desafios de adaptação em jogos, incluindo combinação de player modelling e PCG.

**Aplicação:** adaptive difficulty é uma fase posterior e deve considerar efeitos colaterais, informação dada ao jogador e limites de adaptação.

Fonte: [survey metadata](https://explore.metascienceobservatory.org/papers/W2125266638).

### Rethinking Dynamic Difficulty Adjustment for Video Game Design (2024)

Review sistemática que discute dificuldade como fenómeno da interação entre **jogador e tarefa**, criticando a dependência excessiva de uma leitura simplificada de Flow.

**Aplicação:** particularmente importante para o ASCENDENDO: difficulty não deve ser tratada como propriedade fixa apenas do mapa nem como variável a ajustar automaticamente até caber num intervalo de Flow.

Fonte: [ScienceDirect](https://www.sciencedirect.com/science/article/pii/S1875952124000314).

## 10. Game feel, movimento e feedback

### Designing Game Feel: A Survey (2022)

DOI: `10.1109/TG.2021.3072241`

Survey de mais de 200 fontes que organiza game feel em **physicality, amplification** e **support**, distinguindo tuning da física, juicing do feedback e streamlining da execução da intenção do jogador.

**Aplicação:** para o Commitment Jump, a previsibilidade da física é parte do jogo. Feedback visual/sonoro pode amplificar a leitura do salto sem alterar silenciosamente as regras.

Isto reforça a separação:

```text
física autoritativa
      ≠
feedback audiovisual
```

Fonte: [IEEE](https://doi.org/10.1109/TG.2021.3072241).

## 11. Câmara e informação visível

A literatura de camera control mostra que o framing é parte da experiência e da informação disponível ao jogador.

**Aplicação:** no ASCENDENDO a câmara deve ser avaliada pelo que revela ou oculta do percurso, não apenas por suavidade ou aparência. Como muita literatura de camera control é 3D, usamos estes trabalhos como princípios de atenção/framing, não como arquiteturas técnicas a copiar.

Referências de base: Christie et al., *Camera Control in Computer Graphics*; trabalhos sobre experience-driven camera control.

## 12. Síntese operacional

A investigação atual sustenta este pipeline:

```text
GEOMETRIA AUTHORED
      ↓
VALIDAÇÃO FÍSICA
      ↓
ANÁLISE DE SALTOS
      ↓
ANÁLISE EXECUTIVA DO NÍVEL
      ↓
PES / PLAYER SIMULATION
      ↓
TELEMETRIA HUMANA
      ↓
MODELO DE EXPERIÊNCIA / DIFICULDADE PERCEBIDA
      ↓
PLAYTEST
      ↓
DECISÃO DE DESIGN
```

### Escalas que devemos conservar

**Salto:** alcance, tempo, margem, input necessário.  
**Secção:** sequência de saltos, safe zones, guidance, ritmo.  
**Nível:** dificuldade, rotas, pacing, recovery e leitura.  
**Campanha:** progressão, aprendizagem, repetição com variação e monotonia.  
**Jogador:** desempenho, aprendizagem, preferência e incerteza.

### Regras de interpretação

- validade não significa qualidade;
- existência de solução não significa solução humana plausível;
- solver time não significa automaticamente dificuldade humana;
- dificuldade física não significa dificuldade percebida;
- uma métrica publicada não vira threshold do ASCENDENDO sem calibração;
- PCG não é prioridade enquanto faltarem conteúdo authored e dados de avaliação;
- player modelling não deve começar antes de termos telemetria mínima e um corpus real;
- playtesting continua a ser autoridade para experiência humana.

## 13. Mapa de referências → decisões

| Decisão | Referências mais diretamente úteis |
|---|---|
| Representar desafios e secções | Smith et al. 2008; Khalifa et al. 2019 |
| Medir saltos | Aramini et al. 2018 |
| Simular soluções | Tremblay et al. 2014 |
| Separar dificuldade executiva de percebida | Francillette et al. 2025; Fernández et al. 2018 |
| Modelar experiência de platformer | Pedersen et al. 2009, 2010 |
| Medir desafio percebido | Denisova et al. 2020 |
| Fazer tooling mixed-initiative | Tanagra 2010/2011 |
| Preparar PCG | Togelius et al. 2011; Yannakakis & Togelius 2011; Summerville et al. 2018 |
| Modelar ritmo/challenge | Sorenson et al. 2011 |
| Player modelling | Shaker et al. 2010; Madineni 2025 |
| Adaptive difficulty | Lopes & Bidarra 2011; DDA review 2024 |
| Game feel e feedback | Designing Game Feel 2022 |
| Progressão e variety | Khalifa et al. 2019; Pedersen et al. 2010; Sorenson et al. 2011 |

## 14. Regra de evidência

Para decisões importantes, a ordem é:

1. evidência científica diretamente aplicável;
2. documentação técnica oficial;
3. experiência comprovada de jogos/ferramentas maduros;
4. testes internos, profiling e telemetria;
5. preferência estética/pessoal.

A regra não é “seguir papers”. É **usar papers para escolher melhor o que medir**.

O contrato só nasce quando a hipótese sobrevive a:

```text
literatura → protótipo → simulação → playtest → calibração → integração
```
