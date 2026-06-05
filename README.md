# README.txt

**Feito por Rafael Gomes Bernardo, auxiliado por IA (Gemini, e planeado uso de Claude).**

## 1. Sobre o Projeto

Este projeto consiste no desenvolvimento do zero de um motor de jogo 2D customizado e de um jogo do género *Vertical Precision Platformer*. O objetivo principal é garantir uma estabilidade técnica irrepreensível, portabilidade máxima entre PCs e um desempenho otimizado focado na precisão.

O motor utiliza a API gráfica Vulkan para renderização explícita e de baixo nível. A física do jogo corre num *Fixed Timestep* independente da taxa de renderização, garantindo que a física e as colisões são 100% determinísticas em qualquer hardware. Este determinismo absoluto permite gravar *replays* exatos de movimento e a criação de *save states* perfeitos, permitindo ao jogador reverter para pontos específicos sem qualquer perda de fidelidade na simulação.

O jogo foca-se em movimento horizontal e numa mecânica de salto baseada em carga (pressão do botão determina a força), complementada por métricas de dificuldade geradas algoritmicamente. O projeto adota uma arquitetura de *Level Streaming* para gestão eficiente de memória e segue uma abordagem rigorosa de *Test-Driven Development* (TDD) para garantir a sustentabilidade do código.

## 2. Estrutura do Projeto

A arquitetura modular separa rigidamente a lógica, a renderização e o desenvolvimento.

* **`/Game/`**: A diretoria final e autónoma do jogo.
* **`/Game/Graphics/`**: Abstração do motor gráfico. Contém exclusivamente a implementação do Vulkan (inicialização da *swapchain*, gestão de *descriptor sets*, *shaders* e sincronização da *pipeline*). Expõe apenas as funções essenciais de desenho, incluindo a renderização de elementos de interface (UI).
* **`/Game/Assets/`**: Armazenamento de texturas, ficheiros de níveis e matrizes de colisão.
* **`/Game/Logic/`**: Núcleo do jogo. Gere o *Fixed Timestep*, física vetorial, leitura de *inputs* (WASD, Setas), matriz central de física do mundo (*bitmasking* de colisões), gravação/leitura de ficheiros de *replay* e *save states*, e o sistema de *Level Streaming* da câmera ortográfica.


* **`/Development/`**: Ferramentas internas exclusivas para criação e testes, não incluídas na versão final.
* **`/Development/LevelEditor/`**: Interface para desenho visual dos mapas e definição da matriz de bits físicos.
* **`/Development/AI_Validation/`**: Validador algorítmico baseado em geometria euclidiana e teoria dos grafos para calcular parábolas de salto, extrair margens de erro e gerar métricas rigorosas de dificuldade.
* **`/Development/dev_log.txt`**: Ficheiro imutável de registo do ciclo de desenvolvimento.



## 3. Gestão de Dependências

Nenhuma dependência externa é adicionada sem avaliação prévia. Qualquer biblioteca de terceiros deve respeitar as seguintes regras:

1. Requer justificação detalhada no `dev_log.txt` sobre o seu propósito.
2. O sistema (ou script de build) deve pedir permissão explícita por texto, explicando o que faz, antes de iniciar qualquer *download*.
3. O script inicial trata de baixar e compilar tudo de forma automatizada e limpa.

## 4. Regras de Teste e Versionamento (Git)

O desenvolvimento é sustentável e suportado por testes exaustivos (Unitários, Integração, Sistemas, Regressão e Aceitação).

* **Bloqueio de Commits/Push:** Nenhum código é aceite na *main branch* sem passar 100% da bateria de testes.
* **Test-Driven:** Os testes são escritos na fase de "Ideia", ditando o comportamento esperado das funções.
* **Imutabilidade de Testes:** Testes antigos servem de regressão e não são alterados para acomodar código novo, exceto em refatorizações arquitetónicas planeadas e registadas.

## 5. Diário de Desenvolvimento (`dev_log.txt`)

Ficheiro *append-only* que regista cada passo. Formato obrigatório:
`[YYYY-MM-DD HH:MM:SS]`
`Ideia: [O que vamos implementar e o comportamento que esperamos obter]`
`Implementado: [Se funcionou, como foi feito de facto, problemas encontrados e soluções aplicadas]`

## 6. Planeamento de Desenvolvimento

**Fase 1: Configuração do Ambiente e Arquitetura Base** (ESTAMOS AQUI)

* Criação do repositório, estrutura de pastas e `dev_log.txt`.
* Implementação do gestor e inquiridor de dependências.
* Setup da infraestrutura de testes.

**Fase 2: Motor Gráfico Base (Vulkan Isolado)**

* Inicialização da *Instance*, *Device*, *Swapchain* e render passes básicos.
* Renderização de primitivas (quadrados) para testar a comunicação entre lógica e gráfica.

**Fase 3: Lógica Base, Inputs e Física Determinística**

* Leitura de *inputs* e gestão da janela.
* Implementação do *Fixed Timestep* (loop principal).
* Sistema de colisões por matriz de bits (*Bounding Boxes* e AABB).
* **Implementação do sistema de *Save States* e gravação de *Replays* de *inputs*.**

**Fase 4: Sistema de Mapas (*Level Streaming*) e Câmera**

* Leitura de ficheiros de mapas interligados.
* Lógica da Câmera Ortográfica (Renderizar o nível atual, o de cima e o de baixo).
* Implementação de *Object Pooling* no Vulkan para reaproveitamento de recursos em quedas rápidas, evitando *stutters*.

**Fase 5: Ferramentas de Desenvolvimento e Editor**

* Criação do `/LevelEditor/` para manipulação de bits e texturas.
* Regras de *Autotiling* para geração de mapas.

**Fase 6: O Jogo e IA de Validação**

* Polimento das mecânicas do personagem (movimento horizontal).
* **Implementação da mecânica de salto por carga: UI com barra de carregamento e display numérico do valor da força.**
* Implementação do validador de níveis na pasta `/Development/` (Algoritmo de exploração espacial para ditar e escalar a dificuldade dos mapas de forma automatizada).

**Fase 7: V2 / New Game+ (Pós-Lançamento Base)**

* Introdução de física deslizante (atrito variável baseado no tipo de bloco).
* Novas mecânicas de interação com o mapa e níveis de dificuldade extremos.