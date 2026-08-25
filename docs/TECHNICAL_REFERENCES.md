# Referências técnicas — ASCENDENDO

Esta documentação separa o projeto em subsistemas técnicos e indica investigação/documentação que deverá orientar futuras decisões. Não significa implementar todas as técnicas citadas.

## 1. Arquitetura C++ / engine

### Data-oriented design e ECS

**The Essence of Entity Component System** (Tasnim & Zhao, 2026) formaliza aspectos de ECS e relaciona layout de dados, cache efficiency, estabilidade de frame e paralelismo. O resultado é útil como evidência de que separar dados e processamento pode melhorar previsibilidade/cache, mas não justifica transformar o ASCENDENDO inteiro num ECS.

Fonte: https://arxiv.org/abs/2606.14919

### Decisão ASCENDENDO

Antes de adotar ECS completo, preferir:

- estruturas de dados contíguas onde há processamento em massa;
- separar modelo de nível de rendering;
- evitar virtualização/calls indiretas em hot paths sem necessidade;
- medir antes de otimizar.

O editor e os níveis são bons candidatos para dados compactos e previsíveis. O jogo continua pequeno o suficiente para manter uma arquitetura explícita em C++ sem uma framework ECS pesada.

## 2. Física e timestep

A física deve continuar determinística e com timestep fixo. A investigação de software/game testing reforça a utilidade de replay determinístico para regressão, enquanto a arquitetura do jogo deve evitar que rendering ou input variável alterem a simulação.

### Regras futuras

- fixed timestep permanece independente do frame rate;
- clamp de catch-up para evitar spiral of death;
- testes de invariantes físicos;
- replay como teste de regressão;
- nenhum sistema visual deve decidir o resultado físico.

## 3. Renderer / Vulkan

### Sincronização

A documentação oficial do Vulkan salienta que sincronização é responsabilidade explícita da aplicação e que sincronização excessiva também prejudica desempenho.

Fontes:
- https://docs.vulkan.org/guide/latest/synchronization.html
- https://docs.vulkan.org/samples/latest/samples/performance/pipeline_barriers/README.html

### Consequências

- não usar `ALL_COMMANDS`/barreiras excessivamente amplas sem necessidade;
- definir stages/access masks conforme a dependência real;
- preferir `synchronization2` quando fizer sentido para simplificar a implementação;
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

## 4. Compatibilidade Vulkan / hardware

A documentação de versões e portabilidade do Vulkan recomenda **feature detection**, fallback gracioso e testes em múltiplas implementações, em vez de assumir que uma versão do Vulkan implica todas as features desejadas.

Fonte: https://docs.vulkan.org/guide/latest/versions.html

A própria especificação recomenda desenvolver com Validation Layers habilitadas e removê-las por defeito na release após a aplicação estar validada.

Fonte: https://registry.khronos.org/vulkan/specs/latest-ratified/pdf/vkspec.pdf

### Consequências para ASCENDENDO

Antes da release devemos ter uma matriz de capabilities:

```text
GPU
Vulkan version
queue families
present support
required features
optional features
swapchain formats
present modes
limits
```

A seleção de GPU deve rejeitar apenas o que é realmente obrigatório e ter caminhos alternativos para features opcionais.

## 5. Câmera / rendering vs level design

A câmara é parte da informação que o jogador recebe. Análises técnicas de câmaras de platformers usam dead zones, damping e look-ahead para equilibrar suavidade e antecipação.

Fonte complementar: https://www.gamedeveloper.com/design/camera-logic-in-a-2d-platformer

Isto deve ser cruzado com a literatura de level design: não avaliar a câmera apenas por movimento suave; avaliar quanto do próximo desafio permanece visível durante um salto.

## 6. Testes e qualidade

**Automated Regression Testing within Video Game Development** propõe uma abordagem específica para regressão em jogos, incluindo record/playback e testes orientados ao jogo.

Fonte: https://link.springer.com/article/10.7603/s40601-013-0010-4

A revisão sistemática **A Literature Review of Software Testing Practices and Frameworks in the Video Gaming Industry** (2025) destaca o crescimento de automação e a necessidade de acompanhar bugs em dispositivos heterogéneos.

Fonte: https://onlinelibrary.wiley.com/doi/full/10.1002/stvr.70001

Trabalhos de 2024–2026 exploram BDD, imitation learning, RL e smart playtesting. São referências para o futuro, não justificações para introduzir ML imediatamente.

Fontes:
- https://doi.org/10.1145/3643658.3643919
- https://doi.org/10.1145/3786171.3788378
- https://doi.org/10.1145/3742473

### Consequência

A pirâmide de qualidade deve evoluir para:

```text
unit
  ↓
integration
  ↓
render/runtime
  ↓
replay regression
  ↓
level validation
  ↓
campaign validation
  ↓
hardware matrix
```

## 7. Packaging / EXE

O objetivo do ASCENDENDO não deve ser confundido com a ideia de que "um EXE sozinho" é sempre a melhor forma técnica. Em C++/Vulkan existem DLLs, assets, shaders e runtime components.

A documentação atual da Microsoft distingue deployment framework-dependent de self-contained, e zip/xcopy deployment de MSIX/installer.

Fontes:
- https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/
- https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/self-contained-deploy/deploy-self-contained-apps
- https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/packaging/

### Decisão futura

Manter três objetivos separados:

1. **portable development build** — pasta executável simples;
2. **portable release archive** — zip que funciona após extrair;
3. **distribuição final** — instalador/empacotamento, se for útil.

O requisito do utilizador de "EXE simples" é preservado como experiência: quem recebe a release não deve instalar o ambiente de desenvolvimento nem SDK Vulkan.

### Runtime path

Os assets devem ser encontrados relativamente ao diretório do executável/release, nunca depender do current working directory.

## 8. Web / partilha de mapas

O site futuro deve tratar mapas como **conteúdo não confiável**.

Princípios de segurança:

- validar extensão e assinatura real do conteúdo;
- limitar tamanho do upload;
- armazenar fora de diretórios executáveis;
- usar nomes/IDs gerados pelo servidor;
- bloquear path traversal;
- tratar archives comprimidos com limites de expansão;
- não executar conteúdo enviado;
- revalidar o mapa no EXE do jogador;
- opcionalmente recalcular hash/Campaign ID no servidor.

Referência: OWASP File Upload Cheat Sheet, https://cheatsheetseries.owasp.org/cheatsheets/File_Upload_Cheat_Sheet.html

### Transferência

Para ficheiros pequenos/compactos, HTTP(S) normal é suficiente. Não assumir WebSockets sem necessidade.

Para uploads grandes ou instáveis, considerar um protocolo de upload resumível como tus antes de inventar um sistema próprio.

Fonte: https://tus.io/

### Separação de autoridade

```text
SITE
  = distribuição, metadata, ranking/moderação

EXE
  = autoridade final sobre formato e validade jogável
```

## 9. User Generated Content / comunidade

Super Mario Maker fornece um caso real de criação e distribuição massivas: um estudo de Scientific Reports analisou 115.032 níveis, 795.313 votos e mais de 32 milhões de tentativas, permitindo estudar a relação entre dificuldade, expectativa e apreciação.

Fonte: https://www.nature.com/articles/s41598-025-14628-2

Um estudo de Digital Culture & Society analisa especificamente o trabalho de construir níveis extremos e a relação entre autores e jogadores em comunidades Super Mario Maker.

Fonte: https://doi.org/10.14361/dcs-2019-0207

### Consequência

Quando a biblioteca web existir, os metadados não devem ser apenas:

```text
nome + ficheiro
```

Devem preparar espaço para:

- autor;
- versão do formato;
- campaign ID / level ID;
- hash;
- descrição;
- dificuldade declarada;
- dificuldade observada;
- versão do jogo;
- estado de validação;
- estatísticas de utilização;
- reports/moderação.

Nenhuma destas métricas deve permitir que um mapa inválido passe a ser válido por popularidade.

## 10. Separação final do sistema

O projeto deve ser encarado como estes subsistemas:

```text
GAMEPLAY
  physics / player / input / state

LEVEL DESIGN
  LevelData / editor / validation / analysis

CAMPAIGN
  campaign metadata / order / campaign editor / progression analysis

RENDERING
  Vulkan / pipelines / resources / frame synchronization

TOOLS
  validator / replay / profiling / diagnostics

QUALITY
  unit / integration / regression / hardware matrix

PACKAGING
  release build / assets / runtime dependencies / updater

SHARING
  export / import / web upload / download / metadata / moderation
```

Cada subsistema deve ter testes e documentação próprios. A integração deve acontecer por contratos claros.

## 11. Regra de otimização

O objetivo é correr no pior computador que possa suportar o jogo, mas "otimização" não significa micro-otimização indiscriminada.

Ordem preferida:

1. reduzir trabalho desnecessário;
2. escolher bons layouts de dados;
3. reduzir alocações e IO no hot path;
4. reduzir CPU/GPU synchronization;
5. reduzir bandwidth/overdraw;
6. usar batching/instancing quando a medição justificar;
7. otimizar instruções individuais apenas depois.

Nenhuma otimização entra sem teste ou profiling que demonstre o motivo.
