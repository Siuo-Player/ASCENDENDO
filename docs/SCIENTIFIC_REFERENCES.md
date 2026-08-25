# Referências científicas — ASCENDENDO

Esta documentação complementa `docs/DESIGN_REFERENCES.md` com investigação académica relevante para o tipo de jogo que estamos a construir. A prioridade é dada a trabalhos específicos sobre **2D platformers, level design, dificuldade, experiência do jogador, autoria assistida e câmaras**.

A literatura não é uma especificação para copiar. É usada para transformar decisões que poderiam ser intuitivas em hipóteses testáveis e, quando possível, em ferramentas do próprio editor/validador.

## 1. Nível como unidade de composição — Smith, Cha & Whitehead (2008)

**A Framework for Analysis of 2D Platformer Levels**
DOI: `10.1145/1401843.1401858`

O trabalho propõe uma análise específica de níveis 2D platformer e uma linguagem para descrever componentes e áreas de desafio. A ideia mais importante para o ASCENDENDO é que level design deve ser analisado segundo as mecânicas e desafios próprios do género, em vez de receber apenas princípios genéricos de design. O artigo também trata o nível como espaço onde o jogador aprende e experimenta as regras do jogo.

### Consequência para o ASCENDENDO

O editor não deve ser apenas um desenhador de retângulos. Ao longo do roadmap, o modelo de nível deve permitir identificar **componentes de desafio**, sequências e relações entre plataformas.

Isto suporta a futura evolução para:

- análise de saltos individuais;
- classificação de secções por dificuldade;
- identificação de safe zones e recovery areas;
- identificação de padrões de introdução, escalada e quebra de ritmo;
- relatórios automáticos para ajudar autores da comunidade.

Fontes: [paper em UCSC](https://eis.ucsc.edu/papers/smith-sandbox-08.pdf), [DBLP](https://dblp.org/rec/conf/siggraph/0001CW08).

## 2. Padrões de level design em 2D — Khalifa, de Mesentier Silva & Togelius (2019)

**Level Design Patterns in 2D Games**
DOI: `10.1109/CIG.2019.8847953`

Este trabalho analisa mais de trinta jogos 2D e formaliza seis padrões recorrentes e relevantes para a experiência do jogador. Entre os padrões discutidos encontram-se **Guidance, Safe Zone, Foreshadowing, Layering, Branching e Pace Breaking**.

### Consequência para o ASCENDENDO

Estes padrões devem entrar no vocabulário futuro do editor e dos documentos de design. Não significa obrigar o autor a usar cada padrão; significa que a ferramenta poderá posteriormente ajudar o autor a reconhecer o que está a construir.

Para campanhas longas, isto é especialmente importante: 50–200 níveis não devem ser uma lista de obstáculos arbitrários. Devem permitir ritmo, introdução, repetição com variação, recuperação e escalada de desafio.

Fonte: [paper](https://akhalifa.com/documents/level-design-patterns.pdf), [repositório académico](https://www.um.edu.mt/library/oar/handle/123456789/82005).

## 3. Experiência do jogador em platformers — Pedersen, Togelius & Yannakakis (2009)

**Modeling Player Experience in Super Mario Bros**

O estudo relaciona parâmetros de level design de um platformer com características do jogador e com experiência reportada. Foram usados 480 sessions, e foram estudados elementos como tamanho/posição de gaps e mudanças de direção, relacionando-os com **fun, frustration e challenge**. Os modelos apresentados conseguiram prever challenge e frustration com precisão elevada no conjunto estudado.

### Consequência para o ASCENDENDO

A dificuldade não deve ser inferida apenas pelo número de plataformas ou pelo comprimento do nível. Devemos guardar métricas mais diretamente relacionadas com o espaço de decisão do jogador:

- distância e largura dos saltos;
- diferença vertical entre plataformas;
- necessidade de mudança de direção;
- repetição de tentativas;
- margem de erro do salto;
- sequência de dificuldades ao longo do nível.

Fonte: [Universidade de Malta](https://www.um.edu.mt/library/oar/handle/123456789/22955).

## 4. Otimização de níveis para experiência — Pedersen, Togelius & Yannakakis (2009)

**Optimization of Platform Game Levels for Player Experience**

Este trabalho usa níveis parametrizados de Super Mario Bros e recolhe dados de jogadores para modelar a relação entre parâmetros do nível e experiência. A investigação aponta para a possibilidade de otimizar automaticamente níveis para provocar determinadas características de experiência.

### Consequência para o ASCENDENDO

O objetivo do nosso validador não deve ficar limitado a `válido/inválido`. O roadmap deve deixar espaço para uma segunda camada:

```text
VALIDAÇÃO FÍSICA
    ↓
ANÁLISE DE DESIGN
    ↓
relatório de dificuldade / ritmo / margem
```

A validação física continua a ser autoridade para jogabilidade; a análise de design é uma ferramenta de apoio ao autor.

Fonte: [AIIDE/ResearchGate](https://www.researchgate.net/publication/220978549_Optimization_of_Platform_Game_Levels_for_Player_Experience).

## 5. Tanagra — autoria assistida específica para 2D platformers

**Tanagra: A Mixed-Initiative Level Design Tool** (Smith, Whitehead & Mateas, FDG 2010)
DOI: `10.1145/1822348.1822376`

Tanagra é especialmente relevante porque junta **editor + modelo de jogador + geração/constraints + verificação de jogabilidade**. O designer pode especificar geometria e pacing, enquanto o sistema ajuda a preencher ou verificar o espaço, garantindo jogabilidade ou explicando que as restrições são incompatíveis.

O trabalho posterior **Tanagra: Reactive Planning and Constraint Solving for Mixed-Initiative Level Design** (IEEE TCIAIG, 2011; DOI `10.1109/TCIAIG.2011.2159716`) aprofunda a arquitetura reativa e o uso de constraints.

### Consequência para o ASCENDENDO

Não precisamos de um gerador completo nesta fase. O princípio importante é arquitetural:

> o editor deve poder receber assistência automática sem deixar de ser um editor determinístico e controlado pelo autor.

Isto justifica manter desde já separadas:

- geometria authored;
- estado de validação;
- análise de dificuldade;
- futura assistência automática.

Fontes: [ACM](https://doi.org/10.1145/1822348.1822376), [AAAI AIIDE](https://ojs.aaai.org/index.php/AIIDE/article/view/12379), [IEEE/ResearchGate](https://www.researchgate.net/publication/224242275_Tanagra_Reactive_Planning_and_Constraint_Solving_for_Mixed-Initiative_Level_Design).

## 6. Métricas de salto e probabilidade de sucesso — Aramini, Lanzi & Loiacono (2018)

**An Integrated Framework for AI Assisted Level Design in 2D Platformers**
DOI: `10.1109/GEM.2018.8516490`

Este trabalho é extremamente próximo do núcleo do ASCENDENDO. Apresenta uma toolbox para 2D platformers que estima a dificuldade e probabilidade de sucesso de **saltos individuais** e usa métricas para avaliar dificuldade e probabilidade de completar níveis inteiros.

Isto está diretamente alinhado com o nosso jogo, onde o salto é a mecânica central.

### Consequência para o ASCENDENDO

O validador futuro deve conseguir explicar um salto, em vez de simplesmente classificá-lo como falhado. Por exemplo:

```text
SALTO #17

distância horizontal: 82 px
ΔY: +54 px
margem temporal: 0.11 s
margem horizontal: 6 px
probabilidade estimada: baixa
```

Na visualização do editor, isto pode traduzir-se em:

- trajetória tentada;
- ponto onde o avatar deixou de conseguir alcançar o alvo;
- marca no destino;
- diagnóstico como `ângulo impossível`, `alcance insuficiente`, `timing insuficiente` ou `colisão lateral`.

Fonte: [arXiv](https://arxiv.org/abs/1804.09153), [DOI](https://doi.org/10.1109/GEM.2018.8516490).

## 7. Percepção de dificuldade em platformers — Fernández, Mikami & Kondo (2018)

**Perception of Difficulty in 2D Platformers Using Graph Grammars**
DOI: `10.20668/adada.22.2_38`

O estudo investiga a percepção de dificuldade em 2D side-scrolling platformers e relaciona construção de níveis, estimativas numéricas de dificuldade e desempenho dos jogadores.

### Consequência para o ASCENDENDO

Não devemos assumir que uma métrica física corresponde automaticamente à dificuldade percebida. A roadmap deve distinguir:

```text
Dificuldade física estimada
        ≠
Dificuldade percebida pelo jogador
```

A longo prazo, runs da comunidade podem fornecer dados para comparar ambas.

Fonte: [J-STAGE / artigo open access](https://www.jstage.jst.go.jp/article/adada/22/2/22_38/_article).

## 8. Feature analysis e tamanho da janela de observação — Shaker, Yannakakis & Togelius (2011)

**Feature Analysis for Modeling Game Content Quality**

O trabalho analisa a relação entre características de níveis de platform games e experiência do jogador e mostra que usar apenas janelas muito pequenas do conteúdo pode perder informação relevante para a previsão de preferência/experiência.

### Consequência para campanhas longas

Para o ASCENDENDO, isto sugere que a análise não deve olhar apenas para um salto isolado. Precisamos de níveis e campanhas analisados em múltiplas escalas:

- salto;
- secção;
- nível completo;
- sequência de níveis;
- campanha.

Fonte: [Universidade de Malta](https://www.um.edu.mt/library/oar/handle/123456789/22927).

## 9. Câmara e atenção

A câmara não é apenas uma função técnica de seguir o jogador. Trabalhos sobre camera control tratam a câmara como uma componente da experiência e da informação que fica visível ao jogador.

Para o ASCENDENDO, a consequência principal é conservadora: **não devemos introduzir movimento de câmara só porque é tecnicamente possível**. O enquadramento tem de preservar a leitura dos próximos alvos e não esconder informação crítica.

Fontes relevantes incluem *Camera Control in Computer Graphics* (Christie et al.) e trabalhos sobre adaptive/experience-driven camera control. Estes trabalhos são sobretudo orientados a jogos 3D, por isso usamos os princípios de atenção/framing, não as arquiteturas específicas de 3D.

Fontes: [Camera Control in Computer Graphics](https://citeseerx.ist.psu.edu/document?doi=b6d321e69773e4cebe9d6e144503baa799d9b00f&repid=rep1&type=pdf), [Space Maze: Experience-Driven Game Camera Control](https://www.um.edu.mt/library/oar/handle/123456789/29709).

## 10. O que isto muda no roadmap

A pesquisa reforça cinco áreas que devem existir antes de enriquecer fortemente o catálogo de objetos:

1. **Editor confiável** — limites, snapping, undo/redo, playtest e persistência segura.
2. **Validador de saltos** — trajetória, alcance, margem e diagnóstico.
3. **Análise de nível** — sequência e densidade de desafios, safe zones, guidance e pace.
4. **Análise de campanha** — progressão, repetição, variação e transições.
5. **Telemetria/replays para estudos futuros** — permitir medir tentativa, sucesso, falha e tempo sem alterar a autoridade determinística do motor.

Só depois devemos expandir fortemente o número de elementos colocáveis.

## 11. Escala prevista do conteúdo oficial

O objetivo editorial passa a ser explicitamente de **campanhas longas e duráveis**:

- várias campanhas oficiais;
- campanhas normais com cerca de **50 níveis** em média;
- algumas campanhas especiais com **100–200 níveis**;
- níveis finais e desafios opcionais de dificuldade muito elevada;
- conteúdo comunitário sem limite prático imposto pelo motor, desde que o formato e a validação permaneçam compatíveis.

Isto significa que o editor e os formatos devem ser desenhados desde cedo para suportar centenas de níveis sem depender de hacks específicos para a campanha atual.

## 12. Regra de evidência

Quando uma decisão importante de level design, dificuldade, câmara, input ou experiência do jogador for tomada, devemos preferir:

1. evidência científica diretamente aplicável;
2. experiência de ferramentas/jogos maduros;
3. testes internos e telemetria;
4. preferência pessoal, quando os anteriores não responderem à questão.

A evidência científica não substitui playtesting. Serve para dar ao projeto melhores hipóteses iniciais e para decidir o que vale a pena medir.
