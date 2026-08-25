# Gestão de projeto e engenharia de desenvolvimento

## Objetivo

Este documento define como o ASCENDENDO transforma arquitetura, roadmap e trabalho de implementação num sistema único de planeamento. A intenção não é introduzir burocracia pesada: é tornar explícitos **escopo, dependências, decisões, critérios de conclusão e riscos** que, de outro modo, ficariam dispersos por PRs, commits e conhecimento implícito.

A abordagem baseia-se em quatro ideias:

1. decompor o projeto hierarquicamente em trabalho verificável;
2. manter o planeamento incremental e sincronizado com a arquitetura;
3. tornar dependências técnicas e de coordenação visíveis;
4. tratar modularidade e dívida arquitetural como propriedades técnicas do sistema.

## 1. WBS do ASCENDENDO

A unidade de planeamento não deve ser apenas "uma feature" nem apenas "uma branch". A decomposição operacional é:

```text
Projeto
└── Subsistema
    └── Bloco do Roadmap
        └── Work Package / PR
            ├── tarefas técnicas
            ├── testes/validação
            ├── documentação
            └── critério de saída
```

Exemplo:

```text
ASCENDENDO
└── Presentation / Vulkan
    └── Hardening 9.6
        └── PR 9.7 — renderer failure lifecycle
            ├── RendererCore state invariants
            ├── swapchain recreation
            ├── integration tests
            └── CI validation
```

A **PR não substitui o WBS**. Uma PR é um mecanismo de integração/versionamento de um work package; o work package existe porque corresponde a um objetivo verificável do roadmap.

O trabalho deve ser decomposto até que cada unidade tenha:

- resultado observável;
- responsável claro no contexto do projeto;
- dependências conhecidas;
- validação identificável;
- fronteira suficientemente pequena para revisão.

Tausworthe descreveu WBS especificamente para software como mecanismo para decompor um projeto em subprojetos, tarefas, subtarefas e work packages, ligando objetivos, recursos, atividades e acompanhamento do estado. A literatura posterior também enfatiza que os work packages devem explicitar dependências e que a WBS evolui em conjunto com a arquitetura. [1][2]

## 2. Planeamento incremental, mas deliberado

O ASCENDENDO não deve escolher entre "Big Design Up Front" e evolução totalmente emergente. A política é **planeamento deliberado + refinamento incremental**.

Antes de uma tranche:

```text
problema
  ↓
objetivo arquitetural/produto
  ↓
WBS / work packages
  ↓
dependências + riscos
  ↓
critério de saída
  ↓
implementação
  ↓
validação
  ↓
atualização da arquitetura/WBS
```

O nível de detalhe deve acompanhar a incerteza. Não é necessário especificar todos os ficheiros de uma fase distante, mas as interfaces, dependências e decisões que condicionam o trabalho próximo devem estar suficientemente explícitas.

A WBS deve ser revista quando a arquitetura muda materialmente. Em particular, uma mudança de fronteira arquitetural pode criar, remover ou dividir work packages. [2]

### Regra operacional

Uma branch nova deve nascer de um bloco já definido no roadmap. Durante o desenvolvimento é permitido descobrir novos detalhes, mas uma descoberta que altere:

- o objetivo;
- a fronteira de um subsistema;
- uma dependência crítica;
- um critério de saída;
- ou o risco arquitetural

deve atualizar o documento de planeamento antes de continuar a acumular código.

## 3. Dependências são objetos de primeira classe

Em software, dependências não são apenas `#include`, chamadas de função ou links de bibliotecas. Há pelo menos quatro classes relevantes:

| Tipo | Exemplo ASCENDENDO |
|---|---|
| Produto | Campaign Editor depende de Level Editor |
| Técnica | `RendererFacade` depende de `RendererCore` + passes |
| Dados | runtime e editor dependem do modelo comum `LevelData` |
| Coordenação | alteração de uma interface exige trabalho em consumidores/testes/documentação |

Estudos em desenvolvimento de grande escala mostram que tarefas dependentes criam necessidades de coordenação; essas necessidades podem mudar e ultrapassar fronteiras de equipas. A congruência entre quem precisa coordenar e quem efetivamente coordena está associada a melhor desempenho. [3][4]

Uma implicação importante para o ASCENDENDO é que **"a dependência existe no código" não significa "a dependência está gerida"**.

Para cada work package com impacto externo, registar:

```text
Depende de:
Produz para:
Consumidores afetados:
Testes afetados:
Documentos afetados:
Risco se a dependência mudar:
```

### Dependency awareness

Estudos de projetos de grande escala encontraram falta de awareness sobre dependências como explicação para coordenação ineficaz, especialmente quando o planeamento de especificação, prioridade, estimativa e alocação está desalinhado entre níveis. [5]

Portanto, o roadmap deve explicitar dependências **antes** de iniciar trabalhos paralelos. Não se deve descobrir integração apenas no final da branch.

## 4. Arquitetura e modularidade são parte do planeamento

A arquitetura não é apenas um diagrama para documentação. Ela determina a decomposição técnica do trabalho.

```text
Arquitetura
   ↕
WBS
   ↕
Branches / PRs
   ↕
Testes
```

Uma arquitetura com uma fronteira mal definida tende a produzir work packages com demasiado coupling, enquanto uma decomposição artificial pode produzir APIs, adapters e abstrações cuja única função é esconder o problema.

A investigação sobre socio-technical congruence é especialmente relevante aqui: modularização pode reduzir algumas dependências técnicas, mas não representa por si só todas as dependências de trabalho que exigem coordenação. [4]

Assim, o ASCENDENDO não usará "mais módulos" como sinónimo de "melhor arquitetura".

A pergunta correta é:

> A fronteira reduz responsabilidades, facilita testes e controla dependências que realmente importam para a evolução do sistema?

## 5. Dívida arquitetural

Dívida arquitetural corresponde a decisões arquiteturais subótimas que oferecem benefício imediato mas aumentam o custo ou risco de mudança futura. Estudos empíricos mostram que esta dívida pode acumular-se ao longo do tempo e que a sua gestão exige identificação, documentação, avaliação e refatoração deliberadas. [6][7]

No ASCENDENDO, uma decisão arquitetural temporária deve ter quatro elementos:

```text
Decisão
├── Motivo atual
├── Benefício imediato
├── Custo futuro conhecido
└── Condição para remoção/revisão
```

Exemplo:

```text
RendererFacadeAdapter
├── Motivo: migração incremental do renderer
├── Benefício: mantém API compatível durante cut-over
├── Custo: adiciona uma camada e duplica conceitos
└── Remoção: quando todos os consumidores usam RendererFacade
```

Isto distingue uma **ponte deliberada** de uma abstração que simplesmente ficou esquecida.

## 6. Documentação arquitetural: mínimo necessário, conhecimento máximo

A documentação deve preservar não apenas "o que existe", mas também **porquê existe**. Investigação sobre Architectural Knowledge Management define conhecimento arquitetural como incluindo design, decisões, pressupostos, contexto e razões que determinam a solução. Grande parte desse conhecimento tende a permanecer tácita se não for explicitada. [8][9]

A documentação do ASCENDENDO deve portanto preservar, quando relevante:

- decisão;
- contexto/problema;
- alternativas consideradas;
- consequência;
- dependências;
- critério de revisão.

Não é necessário escrever um documento enorme para cada classe. A documentação deve concentrar-se nas decisões que condicionam vários componentes ou fases.

## 7. Contrato de uma branch / PR

Cada work package deve ser rastreável por:

```text
Roadmap item
   ↓
Branch
   ↓
PR
   ↓
Tests / validation
   ↓
Documentation update
   ↓
Merge
```

### Definition of Ready

Uma branch está pronta para começar quando existe:

- objetivo explícito;
- escopo incluído/excluído;
- dependências conhecidas;
- documentos relevantes consultados;
- critério de saída;
- estratégia de validação.

### Definition of Done

Uma branch só termina quando:

- o código correspondente ao escopo está concluído;
- testes relevantes existem e passam;
- falhas observadas na branch foram corrigidas ou registadas explicitamente;
- documentação normativa está atualizada;
- dívida criada foi classificada;
- dependências alteradas foram revistas;
- a PR pode ser integrada sem deixar trabalho essencial implícito.

## 8. Métricas que importam

Evitar métricas que incentivem produção artificial de trabalho, como número de commits ou linhas modificadas.

Métricas potencialmente úteis:

- work packages concluídos com critérios de saída;
- PRs que integram sem retrabalho estrutural;
- dependências conhecidas antes da implementação;
- tempo/custo de resolução de mudanças que atravessam fronteiras;
- defeitos introduzidos por alteração de fronteiras;
- tendência de dívida arquitetural;
- tamanho e coupling dos componentes centrais;
- cobertura de invariantes e failure paths.

O propósito das métricas é diagnosticar o sistema de desenvolvimento, não transformar o processo num scoreboard.

## 9. Aplicação imediata ao roadmap

O roadmap do ASCENDENDO passa a ser lido em três níveis:

```text
Nível 1 — Estratégico
fases / objetivos do produto e da arquitetura

Nível 2 — Tático
blocos do roadmap + dependências + gates

Nível 3 — Operacional
branches / PRs / testes / documentação
```

Uma fase só pode avançar quando as dependências críticas da fase anterior estiverem fechadas ou explicitamente aceites como dívida com owner, risco e condição de revisão.

## Referências

[1] R. C. Tausworthe, *The Work Breakdown Structure in Software Project Management*, Journal of Systems and Software, 1, 181–186. DOI: 10.1016/0164-1212(79)90018-9. https://doi.org/10.1016/0164-1212(79)90018-9

[2] *Software Engineering*, seção “Work breakdown structure”, Elsevier/Morgan Kaufmann. A obra descreve WBS como decomposição do esforço e destaca que a WBS deve evoluir com a arquitetura e explicitar dependências entre tarefas. https://www.sciencedirect.com/book/monograph/9780124077683/software-engineering

[3] A. Begel, N. Nagappan, *Coordination in Large-Scale Software Development: Helpful and Unhelpful Behaviors*, MSR-TR-2009-135, 2009. https://www.microsoft.com/en-us/research/publication/coordination-in-large-scale-software-development-helpful-and-unhelpful-behaviors/

[4] M. Cataldo, J. D. Herbsleb, K. M. Carley, *Socio-technical congruence: a framework for assessing the impact of technical and work dependencies on software development productivity*, ESEM 2008, pp. 2–11. DOI: 10.1145/1414004.1414008. https://doi.org/10.1145/1414004.1414008

[5] S. Bick, K. Spohrer, R. Hoda, R. Scheerer, A. Heinzl, *Coordination challenges in large-scale software development: a case study of planning misalignment in hybrid settings*, IEEE TSE 44(10), 2018, 932–950. DOI: 10.1109/TSE.2017.2730870. https://doi.org/10.1109/TSE.2017.2730870

[6] J. Yli-Huumo, A. Maglyas, K. Smolander, *How do software development teams manage technical debt? – An empirical study*, Journal of Systems and Software 120, 2016. DOI: 10.1016/j.jss.2016.05.018. https://doi.org/10.1016/j.jss.2016.05.018

[7] *Investigating Architectural Technical Debt accumulation and refactoring over time: A multiple-case study*, Information and Software Technology 67, 2015, 237–253. DOI: 10.1016/j.infsof.2015.07.005. https://doi.org/10.1016/j.infsof.2015.07.005

[8] P. Kruchten, P. Lago, H. van Vliet, T. Wolf, *Building up and Exploiting Architectural Knowledge*, WICSA 2005. DOI: 10.1109/WICSA.2005.19. https://doi.org/10.1109/WICSA.2005.19

[9] M. Ali Babar, T. Dingsøyr, P. Lago, H. van Vliet (eds.), *Software Architecture Knowledge Management: Theory and Practice*, Springer, 2009. https://link.springer.com/book/10.1007/978-3-642-02374-3

[10] T. Dingsøyr, F. O. Bjørnson, N. B. Moe, K. Rolland, E. A. Seim, *Rethinking coordination in large-scale software development*, CHASE 2018, pp. 91–92. DOI: 10.1145/3195836.3195850. https://doi.org/10.1145/3195836.3195850
