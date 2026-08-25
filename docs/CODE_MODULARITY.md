# Modularidade e tamanho de ficheiros

## Princípio

O ASCENDENDO não deve usar ficheiros grandes como contentores de responsabilidades não relacionadas.

O objetivo não é atingir um número arbitrário de linhas. O objetivo é manter **alta coesão, baixo acoplamento e fronteiras compreensíveis**. Tamanho é um sinal de alerta que deve desencadear análise de responsabilidade, dependências e change-proneness.

Isto vale especialmente para:

- entry points;
- classes/componentes que coordenam demasiadas responsabilidades;
- ficheiros que concentram lógica de domínio, infraestrutura e presentation;
- métodos longos e difíceis de testar isoladamente.

## Evidência científica

A literatura sobre code smells identifica `God Class` e `Long Method` como sinais recorrentes de problemas de design. Uma revisão sistemática terciária conclui que smells afetam negativamente maintainability, compreensão e evolução e associa especialmente `God Class` e `Long Method` a maior impacto em qualidade. [1]

Estudos de evolução mostram que smells não devem ser avaliados apenas pela presença isolada: frequência de mudança e interação entre smells ajudam a priorizar refatorações. [2][3]

Estudos de `God Class` descrevem precisamente a concentração de demasiados dados/funcionalidades num único componente e a extração de responsabilidades coesas como estratégia de refatoração. [4]

A decomposição modular clássica também não recomenda “dividir por tamanho” de forma cega. A ideia é esconder decisões de design e produzir módulos com interfaces simples e responsabilidades estáveis. [5]

Assim, o ASCENDENDO adota a regra:

> **tamanho dispara a revisão; coesão e dependências determinam a fronteira.**

## Política prática

O CI verifica simultaneamente bytes e linhas:

| Estado | Linhas | Tamanho | Ação |
|---|---:|---:|---|
| Normal | `< 300` | `< 30 KiB` | continuar |
| Atenção | `300–399` | `30–36 KiB` | planear split/refactor |
| Bloqueio | `>= 400` | `> 36 KiB` | subdividir antes de merge |

Os valores são **guardrails do projeto**, não leis universais da engenharia de software. O CI deve impedir crescimento silencioso e obrigar a uma decisão explícita.

`main.cpp` também é verificado; entry points não podem escapar à política apenas por estarem na raiz.

## Como dividir

Uma divisão válida deve ser explicável por responsabilidade:

```text
main.cpp
    ↓
Application lifecycle
    ↓
Game state / simulation
    ↓
Rendering boundary
    ↓
Subsystem implementations
```

Exemplos corretos no projeto:

```text
Game/Core/ApplicationHelpers.*
    dados de apresentação + helpers da aplicação

Game/Core/ApplicationGraphics.*
    bootstrap de Window/Vulkan/swapchain/pipelines

Game/Graphics/RendererFacade.*
    fronteira entre runtime e renderer

Game/Graphics/WorldRenderer.*
    world pass

Game/Graphics/RenderSnapshot.h
    contrato de dados de presentation
```

Evitar:

```text
A.cpp
  50 linhas movidas de B.cpp

B.cpp
  50 linhas movidas de A.cpp
```

sem que a nova divisão tenha uma responsabilidade coerente.

## God Component / God File

Um ficheiro pode estar abaixo dos limites e ainda ser arquiteturalmente mau. O inverso também é possível: um ficheiro relativamente grande pode ser coerente.

Por isso, durante uma revisão perguntar:

1. Quantas responsabilidades independentes existem?
2. Quantas razões diferentes podem levar o ficheiro a mudar?
3. Quantos subsistemas diferentes ele importa?
4. Existem consumidores que precisam apenas de uma pequena parte?
5. É possível testar uma responsabilidade sem inicializar o resto?
6. A divisão reduziria coupling ou apenas aumentaria o número de arquivos?

Se várias respostas forem desfavoráveis, abrir um work package de refatoração mesmo antes do limite de 400 linhas.

## Relação com WBS

O tamanho de ficheiro deve aparecer como sinal técnico no WBS:

```text
Componente cresce
      ↓
limite / smell / coupling observado
      ↓
análise de responsabilidades
      ↓
proposta de fronteira
      ↓
work package de refatoração
      ↓
teste de comportamento equivalente
      ↓
merge
```

Isto evita que a refatoração seja tratada como “limpeza” opcional.

## Referências

[1] Lacerda et al., *Code smells and refactoring: A tertiary systematic review of challenges and observations*, Journal of Systems and Software 167 (2020), 110610. DOI: 10.1016/j.jss.2020.110610.

[2] A. M. H. Kessentini et al., *Towards a Severity and Activity based Assessment of Code Smells*, Procedia Computer Science 116 (2017), 460–467. DOI: 10.1016/j.procs.2017.10.040.

[3] T. Bavota et al., *A large-scale empirical study on the lifecycle of code smell co-occurrences*, Information and Software Technology 99 (2018), 1–10. DOI: 10.1016/j.infsof.2018.02.004.

[4] *Identification and application of Extract Class refactorings in object-oriented systems*, Journal of Systems and Software (2012). DOI: 10.1016/j.jss.2012.06.002.

[5] D. L. Parnas, *On the Criteria To Be Used in Decomposing Systems into Modules*, Communications of the ACM 15(12) (1972), 1053–1059. DOI: 10.1145/361598.361623.

[6] *Prevalence and severity of design anti-patterns in open source programs—A large-scale study*, Information and Software Technology (2024). DOI: 10.1016/j.infsof.2024.107445.

## Regra para futuras branches

Nenhuma branch deve adicionar um ficheiro grande apenas para “terminar mais depressa”. Se uma alteração fizer um componente aproximar-se do limite, a própria branch deve subdividi-lo antes de passar para funcionalidade dependente.
