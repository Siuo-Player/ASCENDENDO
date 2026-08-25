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

## 2. Física e timestep

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

## 3. Renderer / Vulkan

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

## 4. Compatibilidade Vulkan / hardware

Preferir feature detection e capability queries a assumptions baseadas apenas em vendor, nome da GPU ou versão nominal.

A surface, queues, extensions, features, formats e present modes devem ser validados antes de selecionar um device como utilizável.

## 5. Testing / CI

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

## 6. Packaging / runtime

A build final deve separar artefactos de instalação dos dados do utilizador e não depender do current working directory. O EXE distribuído não deve requerer SDK/compiler ou outros componentes de desenvolvimento.

Consultar documentação Microsoft de deployment e a documentação de sharing do projeto antes de estabilizar o pacote.

## 7. Web sharing / UGC

O site deve tratar mapas/campanhas como conteúdo não confiável. Upload/download deve ser declarativo, limitado e validado; o EXE é a autoridade final antes de permitir jogar um mapa importado.

HTTP(S) é suficiente inicialmente. WebSockets só entram se existir uma necessidade que export/import + HTTP(S) não resolvam.
