# Referências técnicas — ASCENDENDO

Esta documentação separa o projeto em subsistemas técnicos e indica investigação/documentação que deverá orientar futuras decisões. Não significa implementar todas as técnicas citadas.

## 1. Arquitetura C++ / manutenção

### Coupling, cohesion, size e maintainability

A literatura empírica relaciona métricas estruturais como coupling, cohesion, size e complexity com atributos externos de qualidade/manutenibilidade. Estudos em game engines também mostram que complexidade arquitetural dificulta compreensão e evolução, e trabalhos recentes sobre architectural smells encontram relações negativas com testability/maintainability.

Fontes:
- https://www.sciencedirect.com/science/article/pii/S0950584918302441
- https://www.sciencedirect.com/science/article/pii/S1875952124002003
- https://www.sciencedirect.com/science/article/pii/S0164121225000500

### Decisão ASCENDENDO

A evidência sustenta o **processo** de reduzir dependências desnecessárias, responsabilidades concentradas e componentes difíceis de testar. Não sustenta nomes específicos de classes como solução universal.

Aplicação prática:

- reduzir coupling entre runtime e presentation;
- evitar componentes com responsabilidades dispersas;
- dividir responsabilidades quando o crescimento torna manutenção/testes difíceis;
- medir o efeito das alterações antes de introduzir abstrações genéricas;
- manter a arquitetura específica do jogo sempre que isso reduzir complexidade.

### Data-oriented design e ECS

**The Essence of Entity Component System** (Tasnim & Zhao, 2026) formaliza aspectos de ECS e relaciona layout de dados, cache efficiency, estabilidade de frame e paralelismo. O resultado é útil como evidência de que separar dados e processamento pode melhorar determinados workloads, mas não justifica transformar o ASCENDENDO inteiro num ECS.

Fonte: https://arxiv.org/abs/2606.14919

### Decisão ASCENDENDO

Antes de adotar ECS completo, preferir:

- estruturas de dados contíguas onde há processamento em massa;
- separar modelo de nível de rendering;
- evitar virtualização/calls indiretas em hot paths sem necessidade;
- medir antes de otimizar.

## 2. Gestão de projeto, WBS e planeamento

### Work Breakdown Structure

Tausworthe estudou explicitamente WBS em software e descreve a decomposição de projetos em subprojetos, tarefas, subtarefas e work packages como instrumento de planeamento e acompanhamento. A literatura de engenharia de software também descreve work packages, dependências e a necessidade de desenvolver a WBS em conjunto com a arquitetura à medida que esta se consolida.

Fontes:
- R. C. Tausworthe, *The Work Breakdown Structure in Software Project Management*, Journal of Systems and Software 1, 181–186. DOI: https://doi.org/10.1016/0164-1212(79)90018-9
- *Software Engineering*, seção “Work breakdown structure”. https://www.sciencedirect.com/book/monograph/9780124077683/software-engineering

### Decisão ASCENDENDO

O roadmap não é apenas uma lista de features. Deve formar uma WBS progressiva:

```text
Projeto
  → subsistema
    → bloco do roadmap
      → work package / PR
        → implementação + testes + documentação + validação
```

A decomposição deve ser refinada conforme a incerteza diminui, e alterações arquiteturais relevantes devem poder provocar alterações correspondentes no WBS.

## 3. Dependências, coordenação e awareness

### Desenvolvimento de grande escala

Begel e Nagappan estudaram uma equipa Microsoft de 300 pessoas e observaram dificuldades de coordenação associadas a comunicação, capacidade e cooperação, agravadas pela distribuição. [Fonte](https://www.microsoft.com/en-us/research/publication/coordination-in-large-scale-software-development-helpful-and-unhelpful-behaviors/)

Cataldo, Wagstrom, Herbsleb e Carley mostraram que dependências de tarefas geram necessidades de coordenação e propuseram métodos para identificar quem precisa coordenar com quem. O estudo encontrou necessidades voláteis e que frequentemente ultrapassavam fronteiras de equipa. DOI: https://doi.org/10.1145/1180875.1180929

Cataldo, Herbsleb e Carley desenvolveram o conceito de **socio-technical congruence**: a estrutura das dependências técnicas e de trabalho deve ser acompanhada por padrões de coordenação adequados. O estudo encontrou associação entre maior congruência e menor tempo de resolução de pedidos de alteração. DOI: https://doi.org/10.1145/1414004.1414008

Bick et al. estudaram um contexto híbrido de grande escala e encontraram **dependency awareness** insuficiente como explicação para coordenação ineficaz, relacionada com desalinhamento de especificação, prioridade, estimativa e alocação. DOI: https://doi.org/10.1109/TSE.2017.2730870

Sablis et al. reforçam que coordenação em grande escala consiste em gerir dependências entre atividades e que estas atravessam equipas, especialistas e papéis de suporte. https://onlinelibrary.wiley.com/doi/full/10.1002/smr.2297

### Decisão ASCENDENDO

Uma dependência importante deve ser documentada como dependência de **engenharia e coordenação**:

```text
work package
├── depende de
├── produz para
├── consumidores afetados
├── testes afetados
└── risco de mudança
```

O objetivo não é simular processos de uma organização com centenas de pessoas. É evitar que, num projeto menor, as mesmas classes de falha apareçam apenas porque as dependências ficaram implícitas.

## 4. Conhecimento arquitetural e documentação

### Architectural Knowledge Management

Kruchten, Lago, van Vliet e Wolf descrevem conhecimento arquitetural como incluindo não só a estrutura da arquitetura, mas também decisões, pressupostos, contexto e razões. A documentação explícita desse conhecimento facilita a construção e evolução do sistema.

Fonte: https://doi.org/10.1109/WICSA.2005.19

O livro *Software Architecture Knowledge Management: Theory and Practice* organiza investigação sobre documentação arquitetural, design representation, estratégias de gestão e suporte ao processo de arquitetura. https://link.springer.com/book/10.1007/978-3-642-02374-3

### Decisão ASCENDENDO

Documentar decisões que atravessam múltiplas partes do sistema, não cada detalhe de implementação. Para decisões importantes, preservar:

- contexto/problema;
- decisão;
- alternativas consideradas;
- consequências;
- dependências;
- critério de revisão/remoção.

Isto é particularmente importante para adapters, snapshots, modelo comum de níveis e separação runtime/presentation.

## 5. Dívida arquitetural e technical debt

### Architectural Technical Debt

Estudos de múltiplos casos mostram que Architectural Technical Debt acumula-se ao longo do tempo, pode entrar em fases de crise e exige estratégias de refatoração. DOI: https://doi.org/10.1016/j.infsof.2015.07.005

Yli-Huumo et al. estudaram empiricamente como equipas gerem technical debt numa organização de software, mostrando que a gestão da dívida envolve mais do que encontrar problemas: exige decisões, priorização e integração com o trabalho de desenvolvimento. DOI: https://doi.org/10.1016/j.jss.2016.05.018

Um estudo observacional publicado no Journal of Systems and Software em 2025 analisou a gestão de technical debt numa equipa multidisciplinar e reforçou a necessidade de alinhar o tratamento da dívida com capacidade de sprint e avaliação contextual. https://www.sciencedirect.com/science/article/pii/S0164121225002158

### Decisão ASCENDENDO

Dívida arquitetural deliberada deve ter:

```text
motivo atual
+ benefício imediato
+ custo/risco futuro
+ condição para revisão/remoção
```

Um adapter temporário pode ser dívida aceitável se a condição de remoção for rastreável. Uma camada esquecida sem objetivo ou critério de saída não é uma estratégia de migração; é acoplamento permanente por acidente.

## 6. Física e timestep

A física deve continuar com passo fixo independente do frame rate. O texto clássico **Fix Your Timestep!** demonstra o problema de spiral-of-death quando a simulação tenta recuperar indefinidamente tempo perdido e recomenda deixar margem de desempenho ou limitar o número de passos por atualização.

Fonte principal: https://gafferongames.com/post/fix_your_timestep/

### Regras atuais

- passo físico atual: **60 Hz**;
- frame-time e rendering são independentes do passo físico;
- catch-up tem limite defensivo;
- tempos inválidos não entram na simulação;
- replay não deve ser declarado cross-platform bit-exact sem testes/estratégia específica.

### Meta de runtime

O objetivo é sustentar **60 FPS de forma consistente como piso** e **120 FPS quando o hardware permitir de forma consistente**. A métrica de aceitação deve privilegiar frame-time, custo da simulação e estabilidade de frame, e não apenas FPS médio.

O valor exato do limite de catch-up é uma decisão de engenharia e deve ser calibrado por profiling; a literatura demonstra o problema, não um número universal.

## 7. Renderer / Vulkan

### Sincronização

A documentação oficial do Vulkan salienta que sincronização é responsabilidade explícita da aplicação e que sincronização excessiva também prejudica desempenho.

Fontes:
- https://docs.vulkan.org/guide/latest/synchronization.html
- https://docs.vulkan.org/samples/latest/samples/performance/pipeline_barriers/README.html

### Swapchain, fences e presentation

A documentação oficial demonstra explicitamente dois pontos relevantes para o ASCENDENDO:

1. `VK_ERROR_OUT_OF_DATE_KHR` exige recreação da swapchain;
2. resetar um fence antes de sabermos que haverá submissão pode deixar o fence sem sinalização e produzir deadlock.

Fontes:
- https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/04_Swap_chain_recreation.html
- https://docs.vulkan.org/refpages/latest/refpages/source/vkQueuePresentKHR.html

### Queue / present support

A queue usada para `vkQueuePresentKHR` tem de ter suporte à surface correspondente. Graphics e present podem ser a mesma queue family, mas essa equivalência não pode ser assumida universalmente.

Fonte: https://docs.vulkan.org/refpages/latest/refpages/source/vkQueuePresentKHR.html

### Consequências

- não usar `ALL_COMMANDS`/barreiras excessivamente amplas sem necessidade;
- definir stages/access masks conforme a dependência real;
- preparar recreação segura da swapchain;
- verificar graphics e present support separadamente;
- preferir `synchronization2` quando reduzir complexidade;
- validar com Validation Layers em desenvolvimento;
- remover Validation Layers da configuração de release, salvo diagnóstico explícito.

A documentação Khronos mostra inclusive um caso em que uma barreira mais precisa eliminou bubbles e reduziu frame time em cerca de 13%, evidenciando que sincronização correta é também um problema de performance. Fonte: https://docs.vulkan.org/samples/latest/samples/performance/pipeline_barriers/README.html

### Profiling

A otimização Vulkan deve partir de profiling e não de contagem intuitiva de draw calls. A documentação recomenda distinguir fragment-bound/overdraw, bandwidth-bound e synchronization bubbles.

Fonte: https://docs.vulkan.org/guide/latest/profiling.html

### Tile-based GPUs

Como o objetivo é suportar hardware fraco, a arquitetura deve evitar pressupor que toda GPU funciona como uma desktop immediate renderer. A documentação Khronos destaca diferenças de tilers, incluindo o custo de barreiras e a importância de preservar overlap de workloads.

Fonte: https://docs.vulkan.org/guide/latest/tile_based_rendering_best_practices.html

### Regra

Primeiro medir em pelo menos:

- Intel integrated GPU;
- NVIDIA desktop/laptop;
- AMD;
- pelo menos uma implementação conhecida por usar tile-based rendering quando viável.

Depois otimizar o caminho comum sem criar hacks específicos por vendor.

## 8. Compatibilidade Vulkan / hardware

Preferir feature detection e capability queries a assumptions baseadas apenas em vendor, nome da GPU ou versão nominal.

A surface, queues, extensions, features, formats e present modes devem ser validados antes de selecionar um device como utilizável.

## 9. Testing / CI

Game testing research mostra que automação exige objetivos e oracles claros e que automated playtesting deve complementar, não simplesmente substituir, testes humanos.

Fontes principais:
- https://arxiv.org/abs/2103.06431
- https://arxiv.org/abs/2202.12777

Aplicação:

- unit/integration/system tests por fronteira;
- replay regression;
- property/invariant tests;
- malformed input tests;
- CI Linux/Windows;
- sanitizers quando suportados;
- campanha completa validada automaticamente;
- hardware capability matrix.

## 10. Packaging / runtime

A build final deve separar artefactos de instalação dos dados do utilizador e não depender do current working directory. O EXE distribuído não deve requerer SDK/compiler ou outros componentes de desenvolvimento.

Consultar documentação Microsoft de deployment e a documentação de sharing do projeto antes de estabilizar o pacote.

## 11. Web sharing / UGC

O site deve tratar mapas/campanhas como conteúdo não confiável. Upload/download deve ser declarativo, limitado e validado; o EXE é a autoridade final antes de permitir jogar um mapa importado.

HTTP(S) é suficiente inicialmente. WebSockets só entram se existir uma necessidade que export/import + HTTP(S) não resolvam.

## 12. Governança técnica e planeamento

O planeamento de engenharia deve ser ligado diretamente à arquitetura. Para o ASCENDENDO, a unidade operacional é:

```text
Projeto → subsistema → roadmap block → work package/PR → validação
```

A WBS fornece decomposição e acompanhamento do trabalho; a arquitetura define fronteiras e dependências técnicas; o dependency awareness garante que consumidores, testes e documentação são conhecidos antes da integração.

A relação entre estes elementos é documentada normativamente em `docs/PROJECT_MANAGEMENT.md`.

Fontes principais:
- Tausworthe (1979), WBS em software: https://doi.org/10.1016/0164-1212(79)90018-9
- Cataldo et al. (2008), socio-technical congruence: https://doi.org/10.1145/1414004.1414008
- Bick et al. (2018), planning misalignment/dependency awareness: https://doi.org/10.1109/TSE.2017.2730870
- Kruchten et al. (2005), architectural knowledge: https://doi.org/10.1109/WICSA.2005.19
