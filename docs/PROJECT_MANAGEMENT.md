# Gestão de projeto e engenharia de desenvolvimento

## Objetivo

Este documento define como o ASCENDENDO transforma arquitetura, roadmap e trabalho de implementação num sistema único de planeamento. A intenção não é introduzir burocracia pesada: é tornar explícitos **escopo, dependências, decisões, critérios de conclusão e riscos** que, de outro modo, ficariam dispersos por PRs, commits e conhecimento implícito.

A abordagem baseia-se em quatro ideias:

1. decompor o projeto hierarquicamente em trabalho verificável;
2. manter o planeamento deliberado e incremental;
3. tornar dependências técnicas e de coordenação visíveis;
4. tratar modularidade e dívida arquitetural como propriedades técnicas do sistema.

## 1. WBS do ASCENDENDO

A unidade de planeamento não deve ser apenas “uma feature” nem apenas “uma branch”. A decomposição operacional é:

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

A **PR não substitui o WBS**. A PR é o mecanismo de integração/versionamento de um work package; o work package existe porque corresponde a um objetivo verificável do roadmap.

Cada unidade deve ter, na medida adequada ao nível de incerteza:

- resultado observável;
- escopo incluído e excluído;
- dependências conhecidas;
- validação identificável;
- critério de saída;
- risco arquitetural relevante.

A decomposição pode ser refinada durante a execução. Não se deve congelar artificialmente detalhes que ainda são incertos, mas as interfaces e dependências que condicionam o trabalho próximo devem estar explícitas.

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

## 2. Planeamento incremental, mas deliberado

O ASCENDENDO não deve escolher entre **Big Design Up Front** e evolução totalmente emergente. A política é:

> **planeamento deliberado + refinamento incremental + validação empírica**.

O fluxo normal é:

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

Uma descoberta durante uma branch deve atualizar o planeamento antes de acumular código fora do escopo quando alterar materialmente:

- objetivo;
- fronteira de subsistema;
- dependência crítica;
- critério de saída;
- risco arquitetural;
- ordem das fases.

A arquitetura e a WBS devem evoluir em conjunto. Uma nova fronteira arquitetural pode criar, dividir ou eliminar work packages.

## 3. Dependências como objetos de primeira classe

No ASCENDENDO, “dependência” inclui mais do que `#include`, chamadas e bibliotecas.

| Tipo | Exemplo |
|---|---|
| Produto | Campaign Editor depende do Level Editor |
| Técnica | `RendererFacade` depende de `RendererCore` e passes |
| Dados | runtime e editor dependem de `LevelData` |
| Validação | uma alteração de swapchain requer testes Vulkan |
| Coordenação | alteração de uma interface afeta consumidores, testes e documentação |

Para cada work package com impacto externo, registar pelo menos:

```text
Depende de:
Produz para:
Consumidores afetados:
Testes afetados:
Documentos afetados:
Risco se a dependência mudar:
```

O princípio operacional é:

> **uma dependência existente no código não está automaticamente gerida no processo.**

Dependências críticas devem ser identificadas antes de paralelizar trabalho que possa entrar em conflito ou produzir integração tardia.

## 4. Coordenação e awareness

Software de grande escala apresenta frequentemente uma diferença entre dependências técnicas e coordenação efetivamente realizada. Para o ASCENDENDO, isto significa que a informação sobre “quem depende de quê” deve estar disponível no nível de planeamento relevante, e não apenas implícita no código.

Isto aplica-se mesmo num projeto individual: a futura coordenação pode ser entre branches, subsistemas, testes, ferramentas e decisões documentais, e não necessariamente entre pessoas.

### Regra prática

Quando uma mudança atravessa uma fronteira, verificar explicitamente:

```text
interface → consumidores → testes → ferramentas → documentação → roadmap
```

Investigação sobre **socio-technical congruence** mostra que coordenação eficaz depende de alinhar as atividades de coordenação com as necessidades criadas pelas dependências de trabalho; além disso, dependências lógicas podem ser mais relevantes para coordenação do que simples dependências de código. cite-not-applicable

No ASCENDENDO, isto traduz-se numa regra simples: analisar não apenas “que ficheiros importam”, mas também “que trabalho precisa de ser coordenado quando esta interface muda”.

## 5. Arquitetura, modularidade e WBS

A arquitetura não é apenas documentação visual. Ela determina a forma como o trabalho pode ser decomposto.

```text
Arquitetura
   ↕
WBS
   ↕
Branches / PRs
   ↕
Testes
```

Uma fronteira mal definida pode produzir work packages grandes, com elevado coupling e baixa testabilidade. Uma decomposição artificial pode produzir adapters, interfaces e abstrações que apenas aumentam a superfície do sistema.

Portanto:

> **mais módulos ≠ melhor arquitetura**.

A pergunta correta é se a fronteira:

- reduz responsabilidades concentradas;
- controla dependências relevantes;
- melhora testabilidade;
- permite evolução incremental;
- reduz o custo de mudança sem introduzir complexidade gratuita.

A modularidade técnica deve ser analisada juntamente com a realidade do trabalho. Uma fronteira pode reduzir coupling no código e, ainda assim, criar um pacote difícil de coordenar; o inverso também pode acontecer.

## 6. Dívida arquitetural

Uma decisão arquitetural subótima pode ser racional quando oferece um benefício imediato e possui condição clara de revisão. O problema surge quando a solução temporária se torna permanente sem reavaliação.

No ASCENDENDO, uma decisão temporária deve registar:

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
├── Motivo: migração incremental do renderer
├── Benefício: compatibilidade durante o cut-over
├── Custo: camada adicional temporária
└── Remoção: todos os consumidores migrados para RendererFacade
```

Isto transforma dívida arquitetural num objeto gerível, em vez de a tratar como simplesmente “código antigo”.

## 7. Conhecimento arquitetural e documentação

A documentação deve preservar não apenas **o que existe**, mas também **porque existe**, quando essa razão condiciona múltiplos componentes ou fases.

Para decisões relevantes, guardar:

- problema/contexto;
- decisão;
- alternativas consideradas, quando útil;
- consequências;
- dependências;
- critério de revisão ou remoção.

Estudos de Architectural Knowledge Management mostram que decisões, contexto e rationale tendem a permanecer tácitos ou a ficar desatualizados quando não existe uma prática explícita de captura. A literatura também trata decisões arquiteturais como entidades de primeira classe do processo. cite-not-applicable

Documentação normativa e documentação histórica devem ser distinguidas. O histórico pode permanecer útil como evidência; a documentação operacional deve refletir o estado atual.

### Hierarquia documental operacional

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

Uma mudança que contradiga um documento normativo deve atualizar primeiro esse documento ou ser explicitamente justificada no trabalho.

## 8. Definition of Ready

Uma branch/work package está pronta para começar quando existem:

- objetivo explícito;
- escopo incluído/excluído;
- documentos relevantes consultados;
- dependências conhecidas;
- estratégia de validação;
- critério de saída;
- riscos que mereçam acompanhamento.

Não é necessário prever todos os detalhes da implementação. O objetivo é impedir trabalho ambíguo ou dependências importantes descobertas apenas na integração.

## 9. Definition of Done

Uma branch/work package só termina quando:

- o escopo está implementado;
- testes relevantes existem e passam;
- falhas descobertas durante o desenvolvimento foram corrigidas ou registadas explicitamente;
- documentação normativa está atualizada;
- dependências alteradas foram revistas;
- dívida criada foi classificada;
- o critério de saída foi verificado;
- a PR pode ser integrada sem trabalho essencial oculto.

## 10. Gates e progressão

A progressão normal é:

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

Não iniciar um bloco dependente antes de integrar o bloco predecessor em `main`, exceto quando uma dependência paralela estiver deliberadamente planeada e documentada.

Uma fase pode avançar quando as dependências críticas anteriores estão:

1. fechadas; ou
2. explicitamente aceites como dívida, com risco, condição de revisão e lugar no roadmap.

## 11. Métricas de processo

Evitar métricas que incentivem produção artificial, como linhas alteradas, número de commits ou número de branches.

Métricas úteis para diagnóstico podem incluir:

- work packages concluídos segundo critérios de saída;
- retrabalho estrutural após integração;
- dependências descobertas tardiamente;
- mudanças que atravessam muitas fronteiras;
- defeitos introduzidos por alterações de fronteira;
- tendência da dívida arquitetural;
- coupling/tamanho dos componentes críticos;
- cobertura de invariantes e failure paths.

As métricas servem para melhorar o sistema de desenvolvimento, não para criar um scoreboard.

## 12. WBS atual de alto nível

A decomposição operacional corrente deve ser mantida em conjunto com o roadmap. O nível estratégico atual é:

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

Cada ramo acima deve ser refinado no momento em que entrar no próximo bloco do roadmap; não é necessário transformar o futuro inteiro em tarefas artificiais antes de existir informação suficiente.

## 13. Referências principais

[1] R. C. Tausworthe, *The Work Breakdown Structure in Software Project Management*, Journal of Systems and Software 1 (1979), 181–186. DOI: 10.1016/0164-1212(79)90018-9. https://doi.org/10.1016/0164-1212(79)90018-9

[2] A. Begel, N. Nagappan, *Coordination in Large-Scale Software Development: Helpful and Unhelpful Behaviors*, MSR-TR-2009-135, Microsoft Research, 2009. https://www.microsoft.com/en-us/research/publication/coordination-in-large-scale-software-development-helpful-and-unhelpful-behaviors/

[3] M. Cataldo, J. D. Herbsleb, K. M. Carley, *Socio-technical congruence: a framework for assessing the impact of technical and work dependencies on software development productivity*, ESEM 2008, 2–11. DOI: 10.1145/1414004.1414008. https://doi.org/10.1145/1414004.1414008

[4] M. Cataldo, J. D. Herbsleb, *Coordination Breakdowns and Their Impact on Development Productivity and Software Failures*, IEEE Transactions on Software Engineering 39(3) (2013), 343–360. https://herbsleb.org/web-pubs/pdfs/Cataldo-Coordination-2013.pdf

[5] S. Bick, K. Spohrer, R. Hoda, R. Scheerer, A. Heinzl, *Coordination challenges in large-scale software development: a case study of planning misalignment in hybrid settings*, IEEE TSE 44(10) (2018), 932–950. DOI: 10.1109/TSE.2017.2730870. https://doi.org/10.1109/TSE.2017.2730870

[6] J. Yli-Huumo, A. Maglyas, K. Smolander, *How do software development teams manage technical debt? – An empirical study*, Journal of Systems and Software 120 (2016). DOI: 10.1016/j.jss.2016.05.018. https://doi.org/10.1016/j.jss.2016.05.018

[7] *Investigating Architectural Technical Debt accumulation and refactoring over time: A multiple-case study*, Information and Software Technology 67 (2015), 237–253. DOI: 10.1016/j.infsof.2015.07.005. https://doi.org/10.1016/j.infsof.2015.07.005

[8] P. Kruchten, P. Lago, H. van Vliet, T. Wolf, *Building up and Exploiting Architectural Knowledge*, WICSA 2005. DOI: 10.1109/WICSA.2005.19. https://doi.org/10.1109/WICSA.2005.19

[9] M. Ali Babar, I. Gorton, *Architecture Knowledge Management: Challenges, Approaches, and Tools*, ICSE 2007 Companion, 170–171. DOI: 10.1109/ICSECOMPANION.2007.20. https://doi.org/10.1109/ICSECOMPANION.2007.20

[10] P. Kruchten et al., *Software Architecture Knowledge Management: Theory and Practice*, Springer (2009). https://link.springer.com/book/10.1007/978-3-642-02374-3

[11] R. F. Capilla, P. Lago et al., *10 years of software architecture knowledge management: Practice and future*, Journal of Systems and Software 116 (2016), 191–205. DOI: 10.1016/j.jss.2015.08.054. https://doi.org/10.1016/j.jss.2015.08.054

[12] *Software architecture knowledge management approaches and their support for knowledge management activities: A systematic literature review*, Information and Software Technology. https://www.sciencedirect.com/science/article/pii/S0950584916301707

[13] M. Özdeş Demir, *Factors affecting architectural decision-making process and challenges in software projects: An industrial survey*, Journal of Software: Evolution and Process (2024). DOI: 10.1002/SMR.2703. https://doi.org/10.1002/SMR.2703

[14] C. R. B. de Souza, J. M. R. Costa, M. Cataldo, *Analyzing the scalability of coordination requirements of a distributed software project*, Journal of the Brazilian Computer Society 18 (2012), 201–211. https://link.springer.com/article/10.1007/s13173-012-0067-5

[15] T. Dingsøyr et al., *Rethinking coordination in large-scale software development*, CHASE 2018, 91–92. DOI: 10.1145/3195836.3195850. https://doi.org/10.1145/3195836.3195850
