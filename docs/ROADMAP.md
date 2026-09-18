# Roadmap — ASCENDENDO

Este é o **único documento vivo de planeamento do produto**. O Git é o histórico; o código e os assets no `main` são a fonte da verdade do estado implementado.

## Estado actual

O jogo já tem uma base jogável e um **catálogo oficial de 10 campanhas / 610 níveis** integrado no runtime. O runtime inclui física determinística a 60 Hz, Commitment Jump a 60°, câmara vertical, carregamento/transições de níveis, replay/save states, menus, controlos reconfiguráveis, editor de níveis, editor de campanha, validação automática e pipeline de apresentação/captura. A aceitação de conteúdo continua aberta: cada nível ainda precisa de validação final, funcionamento no runtime, jogo manual e correcção dos problemas encontrados.

O problema principal já não é falta de arquitectura. É transformar essa base num jogo que outras pessoas consigam jogar, entender e terminar.

## Prioridade agora

### 1. Completar e depurar o conteúdo 1.0

O catálogo oficial já está integrado com 10 campanhas / 610 níveis. O trabalho restante deste gate é provar que este conteúdo é materialmente final: validação automática, integração no runtime, jogo manual, progressão perceptível e correcção sistemática dos problemas encontrados.

### 2. Fechar a validação de 1.0

Exercitar no conteúdo oficial real a validação de runs/replays, métricas de rota/tempo, Difficulty Tester e a separação entre evidência automática e experiência humana. O gate de validação continua dependente dessa evidência.

Objectivo de conteúdo para 1.0: **10 campanhas**, começando com um conjunto inicial de tamanhos de referência de **10, 25, 25, 25, 25, 50, 50, 50, 100 e 250 níveis**. Esta é uma hipótese de escala de conteúdo, não uma obrigação de produzir exactamente 575 níveis independentemente dos resultados de playtesting. A quantidade final deve ser revista quando houver dados de minutos de jogo, retenção, ritmo de produção e qualidade dos níveis.

Cada nível só conta como concluído quando:

1. existe no formato oficial;
2. passa a validação automática;
3. abre e funciona no runtime;
4. é jogado manualmente;
5. os problemas encontrados são corrigidos.

### 3. Usar o editor como ferramenta de produção

O editor já existe. Agora deve ser medido pelo trabalho que permite fazer, não pelo número de funcionalidades que possui.

Prioridade de melhoria:

1. estabilizar undo/redo por gesto, validação física e testes de regressão;
2. tornar reachability e diagnóstico visíveis sem recalcular física no renderer;
3. corrigir HUD/contexto e layout responsivo;
4. melhorar selecção/picker e manipulação directa, incluindo resize;
5. melhorar navegação vertical e, depois, considerar minimap;
6. integrar apresentação visual estruturada só depois de o authoring estar sólido.

Corrigir apenas fricção real de autoria. Não criar nova arquitectura de editor sem uma necessidade concreta.

### 4. Validar com pessoas

A validação automática responde a perguntas como “é alcançável?” e “o resultado é reproduzível?”. Não responde a “é divertido?”, “é claro?” ou “a dificuldade faz sentido?”.

Fazer playtests externos cedo e repetidamente. Usar os resultados para alterar níveis, leitura, ritmo e UX.

### 5. Integrar a apresentação visual

A direcção é simples pixel art:

- módulos de plataforma de 16×16;
- foreground legível;
- fundos em camadas;
- parallax;
- personagem e elementos principais facilmente distinguíveis.

Não expandir o compositor por antecipação. Integrar assets reais primeiro. Um asset só é definitivo depois de verificado, licenciado e testado no runtime.

### 6. Qualidade de produto

Depois de existir conteúdo suficiente:

- melhorar menus e UX;
- rever responsividade;
- tornar bindings e fluxos principais descobríveis;
- garantir import/export e persistência coerentes;
- preparar uma experiência que não dependa de conhecimento interno do repositório.

### 7. Release 1.0

Só declarar 1.0 quando um utilizador externo conseguir:

```text
instalar → abrir → escolher → jogar → criar/editar → testar → guardar → voltar a abrir
```

e o catálogo oficial estiver validado e distribuível.

## Trabalho paralelo permitido

Gameplay, criação de níveis, playtesting, integração de assets e correcções do editor podem avançar em paralelo quando não criam dependências falsas entre si.

Infraestrutura nova só entra quando desbloqueia uma necessidade real de gameplay, conteúdo, validação ou release.

## O que não fazer

Não criar novos snapshots `STATUS_*`, `CURRENT_STATUS_*` ou `ROADMAP_*`.

Não criar um novo documento só para registar que uma tarefa terminou.

Não transformar cada bug, PR ou execução de CI num post-mortem ou work package permanente.

Não usar percentagens de documentação, linhas de código ou número de PRs como medida de progresso do jogo.

Não transformar o editor numa engine genérica: plataformas continuam a ser o principal objecto authored; spawn e objetivo final são estado derivado.

## Critério de progresso

O progresso relevante é medido por:

**conteúdo jogável → minutos de jogo → jogadores externos → problemas corrigidos → experiência pronta para release.**
