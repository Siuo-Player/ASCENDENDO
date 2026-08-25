# Gestão de projeto e engenharia de desenvolvimento

Este documento liga **arquitetura, roadmap, dependências e implementação**. O objetivo não é criar burocracia: é tornar explícitos escopo, riscos, decisões, validação e condições de conclusão.

## 1. WBS — Work Breakdown Structure

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

A PR é o mecanismo de integração de um work package, não a unidade de planeamento em si. Cada unidade deve ter resultado observável, escopo, dependências, validação e critério de saída.

Tausworthe descreve a WBS em software como decomposição em subprojetos, tarefas, subtarefas e work packages. [1] A literatura técnica posterior recomenda que a WBS seja refinada incrementalmente juntamente com a arquitetura. [2]

## 2. Planeamento deliberado + refinamento incremental

Política do ASCENDENDO:

> **planeamento deliberado + refinamento incremental + validação empírica.**

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
atualização da arquitetura, dívida e roadmap
```

Uma descoberta deve atualizar o planeamento antes de acumular código fora do escopo quando altera materialmente objetivo, fronteira de subsistema, dependência crítica, critério de saída, risco ou ordem das fases.

## 3. Dependências como objetos de primeira classe

| Tipo | Exemplo |
|---|---|
| Produto | Campaign Editor depende de Level Editor |
| Técnica | `RendererFacade` depende de `RendererCore` e passes |
| Dados | runtime/editor dependem de `LevelData` |
| Validação | swapchain depende de testes Vulkan/error paths |
| Coordenação | mudança de interface afeta consumidores, testes e documentação |

Para cada work package que atravessa uma fronteira:

```text
Depende de:
Produz para:
Consumidores afetados:
Testes afetados:
Documentos afetados:
Risco se a dependência mudar:
```

A investigação sobre socio-technical congruence mostra que reduzir dependências técnicas por modularização não elimina automaticamente as necessidades de coordenação; importa também alinhar necessidades de coordenação com coordenação efetivamente realizada. [3][4]

Consequência prática:

> **uma dependência existente no código não está automaticamente gerida no processo.**

## 4. Coordination awareness

Quando uma alteração atravessa uma fronteira, verificar:

```text
interface → consumidores → testes → ferramentas → documentação → roadmap
```

Estudos de desenvolvimento em grande escala mostram que coordenação distribuída pode ser difícil de alinhar com as dependências reais do trabalho. [2][5] Isto aplica-se ao ASCENDENDO mesmo sendo inicialmente um projeto individual: a coordenação pode ser entre branches, subsistemas, testes, ferramentas e documentos.

## 5. Arquitetura, modularidade e WBS

```text
Arquitetura
   ↕
WBS
   ↕
Branches / PRs
   ↕
Testes
```

Uma fronteira mal definida pode produzir work packages grandes, alto coupling e baixa testabilidade. Uma decomposição artificial pode produzir adapters e abstrações sem benefício real.

> **Mais módulos não significa automaticamente melhor arquitetura.**

Uma fronteira é justificável quando reduz responsabilidades concentradas, controla dependências relevantes, melhora testabilidade e permite evolução incremental sem complexidade gratuita.

### 5.1 Tamanho de ficheiros como sinal arquitetural

O ASCENDENDO não trata tamanho como uma métrica de qualidade isolada. Trata-o como **sinal de revisão arquitetural**.

A literatura sobre `God Class` e `Long Method` mostra associação com problemas de maintainability, compreensão e evolução; estudos de evolução mostram ainda que frequência de mudança e coocorrência de smells são úteis para priorizar refatorações. [13][14][15]

Assim:

```text
ficheiro cresce
    ↓
verificar responsabilidades / coesão / coupling
    ↓
identificar fronteiras naturais
    ↓
criar work package de refatoração
    ↓
validar comportamento equivalente
```

Política operacional:

- `< 300` linhas: normal;
- `300–399`: warning e planeamento de split;
- `>= 400`: CI bloqueia até existir subdivisão ou decisão arquitetural documentada excepcional.

Os limites são **guardrails internos**, não valores apresentados pela literatura como universais. A fronteira deve ser definida por coesão, coupling e estabilidade da responsabilidade, não por mover linhas arbitrariamente.

O entry point `main.cpp` é explicitamente coberto pelo checker para impedir que uma raiz do projeto contorne a política.

Ver `docs/CODE_MODULARITY.md` para o racional técnico e referências detalhadas.

## 6. Dívida arquitetural

Uma decisão temporária pode ser racional quando existe benefício imediato e condição clara de revisão:

```text
Decisão
├── Motivo atual
├── Benefício imediato
├── Custo futuro conhecido
└── Condição para remoção / revisão
```

A investigação sobre technical debt e architectural technical debt sustenta tratar a dívida como algo identificável, documentável e deliberadamente gerível. [6][7]

## 7. Conhecimento arquitetural

Decisões importantes devem preservar:

- problema/contexto;
- decisão;
- alternativas, quando relevantes;
- consequências;
- dependências;
- condição de revisão/remoção.

Architectural Knowledge Management trata decisões, contexto e rationale como conhecimento arquitetural. Revisões da área mostram que esse conhecimento apoia decisões e pode degradar-se quando permanece tácito. [8][9][10][11]

Hierarquia documental operacional:

```text
PRODUCT_DECISIONS.md
        ↓
ROADMAP.md
        ↓
ARCHITECTURE.md / TECH_DEBT.md / CODE_MODULARITY.md
        ↓
BRANCH_PLAN.md
        ↓
PR / testes / commits
```

## 8. Definition of Ready

Uma branch/work package está pronta quando existe:

- objetivo explícito;
- escopo incluído/excluído;
- documentos normativos consultados;
- dependências críticas identificadas;
- estratégia de validação;
- critério de saída;
- riscos relevantes.

## 9. Definition of Done

Uma branch/work package só termina quando:

- o escopo está implementado;
- testes relevantes existem e passam;
- failure paths relevantes foram exercitados quando possível;
- falhas encontradas foram corrigidas ou registadas;
- documentação normativa está atualizada;
- dependências alteradas foram revistas;
- dívida criada foi classificada;
- o critério de saída foi demonstrado;
- a PR pode ser integrada sem trabalho essencial oculto.

## 10. Gates e progressão

```text
work package → implementação → validação → PR → merge → main atualizado → novo work package
```

Não iniciar um bloco dependente antes de integrar o predecessor em `main`, salvo dependência paralela explicitamente planeada.

Uma fase pode avançar quando as dependências críticas estão fechadas ou são aceites explicitamente como dívida com risco, condição de revisão e posição no roadmap.

## 11. Métricas

Não usar commits, linhas alteradas ou número de branches como objetivos de produtividade.

Usar métricas apenas para diagnóstico:

- work packages concluídos segundo critérios de saída;
- retrabalho estrutural depois da integração;
- dependências descobertas tardiamente;
- alterações que atravessam muitas fronteiras;
- defeitos associados a alterações de fronteira;
- tendência da dívida arquitetural;
- coupling/tamanho de componentes críticos;
- cobertura de invariantes e failure paths.

## 12. WBS de alto nível

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

Cada ramo é refinado quando entra no próximo bloco do roadmap.

## 13. PoC como decisão arquitetural

Uma PoC que influencia uma decisão arquitetural não deve desaparecer depois de produzir um resultado. Trabalho recente propõe tratar PoCs como instrumentos explícitos de decisão, com planeamento, execução e decisão rastreável. [12]

```text
Hipótese → PoC → resultado observado → decisão → consequência
```

## Referências

[1] R. C. Tausworthe, *The Work Breakdown Structure in Software Project Management*, Journal of Systems and Software 1 (1979), 181–186. DOI: 10.1016/0164-1212(79)90018-9.

[2] A. Begel, N. Nagappan, *Coordination in Large-Scale Software Development: Helpful and Unhelpful Behaviors*, Microsoft Research Technical Report MSR-TR-2009-135 (2009).

[3] M. Cataldo, J. D. Herbsleb, K. M. Carley, *Socio-technical congruence: a framework for assessing the impact of technical and work dependencies on software development productivity*, ESEM 2008. DOI: 10.1145/1414004.1414008.

[4] M. Cataldo, J. D. Herbsleb, *Coordination Breakdowns and Their Impact on Development Productivity and Software Failures*, IEEE TSE 39(3) (2013), 343–360. DOI: 10.1109/TSE.2012.32.

[5] S. Bick et al., *Coordination challenges in large-scale software development: a case study of planning misalignment in hybrid settings*, IEEE TSE 44(10) (2018), 932–950. DOI: 10.1109/TSE.2017.2730870.

[6] J. Yli-Huumo, A. Maglyas, K. Smolander, *How do software development teams manage technical debt? – An empirical study*, Journal of Systems and Software 120 (2016). DOI: 10.1016/j.jss.2016.05.018.

[7] *Investigating Architectural Technical Debt accumulation and refactoring over time: A multiple-case study*, Information and Software Technology 67 (2015), 237–253. DOI: 10.1016/j.infsof.2015.07.005.

[8] P. Kruchten et al., *Building up and Exploiting Architectural Knowledge*, WICSA 2005. DOI: 10.1109/WICSA.2005.19.

[9] M. Ali Babar, I. Gorton, *Architecture Knowledge Management: Challenges, Approaches, and Tools*, ICSE 2007 Companion. DOI: 10.1109/ICSECOMPANION.2007.20.

[10] M. Ali Babar, T. Dingsøyr, P. Lago, H. van Vliet (eds.), *Software Architecture Knowledge Management: Theory and Practice*, Springer, 2009.

[11] R. F. Capilla et al., *10 years of software architecture knowledge management: Practice and future*, Journal of Systems and Software 116 (2016), 191–205. DOI: 10.1016/j.jss.2015.08.054.

[12] B. F. Antognolli, F. Petrillo, *Proof of Concept as a First-Class Architectural Decision Instrument*, arXiv:2604.05835 (2026). https://arxiv.org/abs/2604.05835

[13] Lacerda et al., *Code smells and refactoring: A tertiary systematic review of challenges and observations*, Journal of Systems and Software 167 (2020), 110610. DOI: 10.1016/j.jss.2020.110610.

[14] *Towards a Severity and Activity based Assessment of Code Smells*, Procedia Computer Science 116 (2017), 460–467. DOI: 10.1016/j.procs.2017.10.040.

[15] *A large-scale empirical study on the lifecycle of code smell co-occurrences*, Information and Software Technology 99 (2018), 1–10. DOI: 10.1016/j.infsof.2018.02.004.

## Documentos relacionados

- `docs/WORK_PACKAGE_TEMPLATE.md`
- `docs/CODE_MODULARITY.md`
- `docs/PRODUCT_DECISIONS.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/BRANCH_PLAN.md`
