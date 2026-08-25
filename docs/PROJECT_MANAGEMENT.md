# Gestão de projeto e engenharia de desenvolvimento

## Objetivo

Este documento define como o ASCENDENDO transforma arquitetura, roadmap e trabalho de implementação num sistema único de planeamento.

A política baseia-se em quatro princípios:

1. decompor o projeto hierarquicamente em trabalho verificável;
2. manter o planeamento deliberado, mas refinável;
3. tornar dependências técnicas e de coordenação visíveis;
4. tratar modularidade e dívida arquitetural como propriedades técnicas.

## 1. WBS — Work Breakdown Structure

A unidade de planeamento não é apenas uma feature nem uma branch. A decomposição operacional é:

```text
Projeto
└── Subsistema
    └── Bloco do Roadmap
        └── Work Package / PR
            ├── tarefas técnicas
            ├── testes / validação
            ├── documentação
            └── critério de saída
```

A **PR não substitui o WBS**. Uma PR é o mecanismo de integração e versionamento de um work package; o work package existe porque corresponde a um objetivo verificável.

Cada unidade deve ter, ao nível adequado de detalhe:

- resultado observável;
- escopo incluído e excluído;
- dependências conhecidas;
- validação identificável;
- critério de saída;
- riscos relevantes.

A WBS pode ser refinada durante a execução. Não se devem congelar detalhes ainda incertos, mas interfaces e dependências que condicionam o trabalho próximo devem estar explícitas.

### Exemplo

```text
ASCENDENDO
└── Presentation / Vulkan
    └── Hardening / renderer lifecycle
        └── PR 9.7 — renderer failure state
            ├── RendererCore state invariant
            ├── swapchain recreation
            ├── integration tests
            └── CI validation
```

Tausworthe descreve a WBS precisamente como mecanismo de decomposição de engenharia em subprojetos, tarefas, subtarefas e work packages, ligando objetivos, recursos e atividades e permitindo acompanhar o progresso. citeturn760156search0turn760156search7

## 2. Planeamento deliberado + desenvolvimento incremental

O ASCENDENDO não deve escolher entre Big Design Up Front e evolução totalmente emergente. A política é:

> **planeamento deliberado + refinamento incremental + validação empírica**.

Fluxo normal:

```text
problema
  ↓
objetivo de produto / arquitetura
  ↓
WBS / work packages
  ↓
dependências + riscos + decisões
  ↓
critério de saída
  ↓
implementação
  ↓
testes / profiling / validação
  ↓
atualização de arquitetura, dívida e roadmap
```

Uma descoberta deve atualizar o planeamento antes de acumular código fora do escopo quando alterar materialmente:

- objetivo;
- fronteira de subsistema;
- dependência crítica;
- critério de saída;
- risco arquitetural;
- ordem das fases.

A WBS e a arquitetura devem evoluir em conjunto.

## 3. Dependências como objetos de primeira classe

No ASCENDENDO, dependência inclui mais do que `#include`, chamadas e bibliotecas.

| Tipo | Exemplo |
|---|---|
| Produto | Campaign Editor depende do Level Editor |
| Técnica | `RendererFacade` depende de `RendererCore` e passes |
| Dados | runtime e editor dependem de `LevelData` |
| Validação | alteração de swapchain exige testes Vulkan |
| Coordenação | mudança de interface afeta consumidores, testes e documentação |

Para cada work package com impacto externo, registar:

```text
Depende de:
Produz para:
Consumidores afetados:
Testes afetados:
Documentos afetados:
Risco se a dependência mudar:
```

A investigação de Cataldo, Herbsleb e Carley mostra que modularização não representa por si só todas as dependências de trabalho; a eficácia depende da congruência entre necessidades de coordenação e coordenação efetivamente realizada. O estudo encontrou redução média de 32% no tempo de resolução de pedidos quando essa congruência era melhor. citeturn760156search36turn760156search5

Portanto:

> **uma dependência existente no código não está automaticamente gerida no processo.**

## 4. Coordination awareness

Coordenação não é apenas comunicação. Em desenvolvimento de grande escala, problemas de comunicação, capacidade, cooperação e dependências distribuídas podem tornar a integração difícil; Begel e Nagappan estudaram este fenómeno numa organização da Microsoft com equipas e dependências distribuídas. citeturn760156search1

Quando uma alteração atravessa uma fronteira, verificar explicitamente:

```text
interface
   ↓
consumidores
   ↓
testes
   ↓
ferramentas
   ↓
documentação
   ↓
roadmap
```

O ASCENDENDO aplica isto mesmo sendo um projeto individual: a “coordenação” pode ser entre branches, subsistemas, ferramentas, testes e documentos.

Além disso, trabalho posterior sobre coordenação mostra que dependências lógicas podem ser mais relevantes para determinar necessidades de coordenação do que dependências puramente sintáticas do código. citeturn760156search37

## 5. Arquitetura, modularidade e WBS

A arquitetura não é apenas um diagrama. Ela condiciona a decomposição do trabalho:

```text
Arquitetura
   ↕
WBS
   ↕
Branches / PRs
   ↕
Testes
```

Uma fronteira mal definida pode produzir work packages grandes, alto coupling e baixa testabilidade. Uma decomposição artificial pode produzir adapters e abstrações que apenas escondem o problema.

Portanto:

> **mais módulos ≠ melhor arquitetura**.

Uma fronteira é justificável quando:

- reduz responsabilidades concentradas;
- controla dependências relevantes;
- melhora testabilidade;
- permite evolução incremental;
- reduz custo de mudança sem introduzir complexidade gratuita.

## 6. Dívida arquitetural

Uma decisão subótima pode ser racional quando traz benefício imediato e tem condição clara de revisão. O problema é quando a solução temporária fica permanente sem reavaliação.

Registar:

```text
Decisão
├── Motivo atual
├── Benefício imediato
├── Custo futuro conhecido
└── Condição para remoção / revisão
```

Exemplo:

```text
RendererFacadeAdapter
├── Motivo: migração incremental
├── Benefício: compatibilidade durante o cut-over
├── Custo: camada adicional temporária
└── Remoção: todos os consumidores usam RendererFacade
```

## 7. Conhecimento arquitetural

A documentação deve preservar não apenas **o que existe**, mas também **porque existe**, quando essa razão condiciona múltiplos componentes ou fases.

Para decisões relevantes, guardar:

- problema/contexto;
- decisão;
- alternativas consideradas, quando útil;
- consequências;
- dependências;
- critério de revisão/removal.

A investigação em Architectural Knowledge Management trata explicitamente design, decisões, contexto e rationale como conhecimento arquitetural; trabalhos empíricos mostram também que esse conhecimento frequentemente permanece tácito ou fica desatualizado quando não existe uma prática explícita de captura. citeturn760156search38turn196680search0turn196680search1turn196680search2

A documentação operacional do ASCENDENDO deve seguir esta hierarquia:

```text
PRODUCT_DECISIONS.md
        ↓
ROADMAP.md
        ↓
ARCHITECTURE.md / TECH_DEBT.md
        ↓
BRANCH_PLAN.md
        ↓
PR / testes / commits
```

## 8. Definition of Ready

Uma branch/work package está pronta quando existe:

- objetivo explícito;
- escopo incluído/excluído;
- documentos relevantes consultados;
- dependências conhecidas;
- estratégia de validação;
- critério de saída;
- riscos relevantes conhecidos.

Não é necessário prever todos os detalhes da implementação.

## 9. Definition of Done

Uma branch/work package só termina quando:

- o escopo está implementado;
- testes relevantes existem e passam;
- falhas descobertas foram corrigidas ou registadas explicitamente;
- documentação normativa está atualizada;
- dependências alteradas foram revistas;
- dívida criada foi classificada;
- o critério de saída foi verificado;
- a PR pode ser integrada sem trabalho essencial oculto.

## 10. Gates e progressão

```text
work package
    ↓
implementação
    ↓
validação
    ↓
PR
    ↓
merge
    ↓
main atualizado
    ↓
novo work package
```

Não iniciar um bloco dependente antes de integrar o predecessor em `main`, salvo dependência paralela explicitamente planeada.

Uma fase pode avançar quando as dependências críticas anteriores estão fechadas ou explicitamente aceites como dívida com risco, condição de revisão e posição no roadmap.

## 11. Métricas

Evitar métricas que incentivem produção artificial, como linhas alteradas ou número de commits.

Métricas úteis para diagnóstico:

- work packages concluídos segundo critérios de saída;
- retrabalho estrutural após integração;
- dependências descobertas tardiamente;
- mudanças que atravessam muitas fronteiras;
- defeitos associados a alterações de fronteira;
- tendência da dívida arquitetural;
- coupling/tamanho dos componentes críticos;
- cobertura de invariantes e failure paths.

As métricas servem para melhorar o processo, não para criar um scoreboard.

## 12. WBS atual de alto nível

```text
ASCENDENDO
├── Runtime
│   ├── Simulation / Physics
│   ├── State management
│   ├── Campaign / Replay
│   └── Paths / user data
├── Editor
│   ├── LevelData / Document
│   ├── Interaction
│   ├── Validation
│   ├── Undo / Redo
│   └── Campaign Editor
├── Presentation
│   ├── RendererCore
│   ├── RendererFacade
│   ├── World / UI / Editor passes
│   └── RenderSnapshot
├── Infrastructure
│   ├── Vulkan / GLFW
│   ├── Build / CI
│   ├── Sanitizers / profiling
│   └── Portable packaging
└── Sharing / Community
    ├── import / export
    ├── local validation
    └── future online library
```

Cada ramo é refinado quando entra no próximo bloco do roadmap, em vez de transformar o futuro inteiro em tarefas artificiais.

## 13. Referências principais

[1] R. C. Tausworthe, *The Work Breakdown Structure in Software Project Management*, Journal of Systems and Software 1 (1979), 181–186. DOI: 10.1016/0164-1212(79)90018-9. https://doi.org/10.1016/0164-1212(79)90018-9

[2] A. Begel, N. Nagappan, *Coordination in Large-Scale Software Development: Helpful and Unhelpful Behaviors*, Microsoft Research Technical Report MSR-TR-2009-135 (2009). https://www.microsoft.com/en-us/research/publication/coordination-in-large-scale-software-development-helpful-and-unhelpful-behaviors/

[3] M. Cataldo, J. D. Herbsleb, K. M. Carley, *Socio-technical congruence: a framework for assessing the impact of technical and work dependencies on software development productivity*, ESEM 2008. DOI: 10.1145/1414004.1414008. https://doi.org/10.1145/1414004.1414008

[4] M. Cataldo, J. D. Herbsleb, *Coordination Breakdowns and Their Impact on Development Productivity and Software Failures*, IEEE Transactions on Software Engineering 39(3) (2013), 343–360. https://herbsleb.org/web-pubs/pdfs/Cataldo-Coordination-2013.pdf

[5] S. Bick et al., *Coordination challenges in large-scale software development: a case study of planning misalignment in hybrid settings*, IEEE TSE 44(10) (2018), 932–950. DOI: 10.1109/TSE.2017.2730870. https://doi.org/10.1109/TSE.2017.2730870

[6] J. Yli-Huumo, A. Maglyas, K. Smolander, *How do software development teams manage technical debt? – An empirical study*, Journal of Systems and Software 120 (2016). DOI: 10.1016/j.jss.2016.05.018. https://doi.org/10.1016/j.jss.2016.05.018

[7] *Investigating Architectural Technical Debt accumulation and refactoring over time: A multiple-case study*, Information and Software Technology 67 (2015), 237–253. DOI: 10.1016/j.infsof.2015.07.005. https://doi.org/10.1016/j.infsof.2015.07.005

[8] P. Kruchten et al., *Building up and Exploiting Architectural Knowledge*, WICSA 2005. DOI: 10.1109/WICSA.2005.19. https://doi.org/10.1109/WICSA.2005.19

[9] M. Ali Babar, I. Gorton, *Architecture Knowledge Management: Challenges, Approaches, and Tools*, ICSE 2007 Companion, 170–171. DOI: 10.1109/ICSECOMPANION.2007.20. https://doi.org/10.1109/ICSECOMPANION.2007.20

[10] P. Kruchten et al., *Software Architecture Knowledge Management: Theory and Practice*, Springer (2009). https://link.springer.com/book/10.1007/978-3-642-02374-3

[11] R. F. Capilla et al., *10 years of software architecture knowledge management: Practice and future*, Journal of Systems and Software 116 (2016), 191–205. DOI: 10.1016/j.jss.2015.08.054. https://doi.org/10.1016/j.jss.2015.08.054

[12] *Software architecture knowledge management approaches and their support for knowledge management activities: A systematic literature review*, Information and Software Technology. https://www.sciencedirect.com/science/article/pii/S0950584916301707

[13] M. Özdeş Demir, *Factors affecting architectural decision-making process and challenges in software projects: An industrial survey*, Journal of Software: Evolution and Process (2024). DOI: 10.1002/SMR.2703. https://doi.org/10.1002/SMR.2703

[14] C. R. B. de Souza, J. M. R. Costa, M. Cataldo, *Analyzing the scalability of coordination requirements of a distributed software project*, Journal of the Brazilian Computer Society 18 (2012), 201–211. https://link.springer.com/article/10.1007/s13173-012-0067-5

[15] T. Dingsøyr et al., *Rethinking coordination in large-scale software development*, CHASE 2018. DOI: 10.1145/3195836.3195850. https://doi.org/10.1145/3195836.3195850
