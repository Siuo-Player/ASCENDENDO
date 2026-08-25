# Auditoria da base — 25/08/2026

## Objetivo

Esta auditoria regista a revisão posterior à integração da 9.4. O projeto melhorou significativamente com a migração incremental do renderer e com os snapshots do editor. Os problemas prioritários agora são sobretudo **fronteiras incompletas entre as novas abstrações, caminhos legados ainda ativos e regras documentadas que ainda não estão implementadas**.

A regra de manutenção é: primeiro fechar a base; só depois construir conteúdo significativo sobre ela.

As conclusões arquiteturais abaixo devem ser entendidas como **processos e propriedades desejáveis**, não como obrigação de usar nomes de classes específicos. A investigação sobre coupling, cohesion, architectural smells e maintainability dá suporte à redução de dependências e de componentes excessivamente grandes, mas não prova uma implementação nominal única. Estudos empíricos encontraram relações entre coupling/complexity/size e maintainability, e estudos recentes sobre architectural smells encontram relações negativas com testability/maintainability. Ver `docs/TECHNICAL_REFERENCES.md` e `docs/RESEARCH_INDEX.md` para as fontes.

## Estado atual

A direção arquitetural é considerada boa, mas a implementação ainda está intermédia:

```text
arquitetura-alvo
      ↓
implementação parcial
      ↓
adapters / caminhos legados / contratos incompletos
```

Isto não implica reescrever o motor. A estratégia continua sendo migração incremental, com testes antes e depois de cada fronteira.

## P0 — corrigir antes de novas funcionalidades significativas

### 1. Unificar o fluxo de input ✅

Toda a entrada de gameplay deve seguir uma única cadeia de significado:

```text
hardware → estado bruto → mapeamento configurável → ação de jogo → gameplay
```

A tranche inicial 9.6 eliminou o acesso direto a teclas físicas no gameplay e fez o movimento/salto respeitar o mesmo conjunto configurável de ações usado pelo restante do jogo.

### 2. Fechar o contrato do editor de uma única tela 🔄

A documentação estabelece um Level Editor fixo de `640x360`. A implementação anterior ainda possuía camera-pan e passava a câmera para componentes que já a ignoravam.

A branch `feat/9-6-editor-viewport-hardening` está a fechar este contrato:

```text
Level Editor = uma tela fixa 640x360
cursor        = logical → editor world
renderer      = editor world → logical screen
camera        = fora da transformação do Level Editor
```

A sessão e o renderer do editor já não recebem uma câmera e o runtime deixou de atualizar camera-pan no estado EDITOR. O teste cobre explicitamente que uma câmera externa não altera o cursor/preview.

### 3. Corrigir o fixed timestep ✅

O acumulador já não permite catch-up ilimitado após um frame muito longo. A tranche inicial de hardening:

- limita passos de simulação por frame;
- rejeita `NaN`/`Inf`;
- mantém o passo físico atual de 60 Hz;
- adiciona regressões para frames longos e tempo inválido.

A política concreta de limite é uma decisão de engenharia; o problema do catch-up ilimitado/spiral of death é o fundamento técnico.

### 4. Fechar ciclo de vida e erros de Vulkan

O fluxo de acquire/reset/submit/present tem de ser seguro quando uma operação termina cedo. A documentação oficial do Vulkan mostra explicitamente que recriar uma swapchain depois de `VK_ERROR_OUT_OF_DATE_KHR` pode causar deadlock se o fence for resetado antes de sabermos que haverá uma submissão; recomenda-se atrasar o reset até depois de confirmar a submissão. Também devem ser tratados explicitamente `VK_ERROR_OUT_OF_DATE_KHR` e `VK_SUBOPTIMAL_KHR`. Ver `docs/TECHNICAL_REFERENCES.md`.

Este ponto é um requisito técnico da API, não apenas uma preferência arquitetural.

### 5. Não assumir que graphics e present são a mesma queue

A especificação Vulkan exige que a queue usada para `vkQueuePresentKHR` tenha suporte à surface correspondente. Graphics e present podem coincidir, mas não devem ser assumidas como a mesma família em todos os devices.

A seleção de device deve verificar separadamente:

- graphics support;
- present support;
- extensões obrigatórias;
- features obrigatórias;
- capabilities/formats/present modes da surface.

Este ponto também vem diretamente do contrato técnico do Vulkan.

## P1 — consolidar as fronteiras arquiteturais

### Renderer / apresentação

A nova decomposição é mantida, mas a regra é de processo:

```text
modelo/runtime
      ↓
dados de apresentação compactos
      ↓
subcamadas de renderização
      ↓
Vulkan
```

O renderer não deve adquirir conhecimento de regras de gameplay, edição ou campanha simplesmente para conseguir desenhar.

As pontes de compatibilidade devem desaparecer quando já não houver consumidores legados. Não existe evidência científica de que um nome específico de classe seja obrigatório; a propriedade importante é que a dependência entre domínio e apresentação permaneça pequena e testável.

### Separação da apresentação

O editor já possui um snapshot intermédio. O gameplay ainda passa objetos de domínio diretamente para a apresentação.

Objetivo do processo:

```text
Runtime / Editor
      ↓
dados de renderização
      ↓
Renderer
```

O estado de renderização deve ser transitório, compacto e sem ownership de recursos Vulkan.

### Loop principal e responsabilidades

O loop principal ainda orquestra demasiados subsistemas. A extração deve ser incremental para reduzir responsabilidades, melhorar testabilidade e diminuir acoplamento, não para seguir um diagrama por obrigação.

Separar responsabilidades de:

- ciclo de vida da aplicação;
- transições de estado;
- simulação temporal;
- campanha/editor;
- apresentação.

### Ownership Vulkan

Preferir ownership explícito e RAII. Evitar `new/delete` manuais quando composição direta ou `std::unique_ptr` resolverem a mesma necessidade.

### Modelo comum de nível

Runtime, editor e parser devem convergir gradualmente para um modelo declarativo comum de dados de nível, para não manterem representações concorrentes da mesma informação.

### Undo/Redo

Operações editoriais devem evoluir para comandos transacionais. Um drag completo deve produzir uma única operação lógica no histórico.

## P1 — runtime e distribuição

### Paths

Separar:

```text
install/executable root → assets do jogo
user data root → settings/runs/saves/importados
```

Eliminar dependência do current working directory e evitar escrever runtime data em `Development/`.

### Source-size policy

Manter uma única ferramenta e uma única política. O limite de tamanho é um **guardrail de manutenção**, não uma medida científica de qualidade arquitetural. O importante é impedir monólitos e responsabilidades excessivamente concentradas.

## P2 — qualidade e CI

A matriz documentada deve evoluir para:

```text
Linux normal
Linux ASan/UBSan
Linux headless Vulkan
Linux campaign
Windows build + tests
Windows game build
```

Adicionar:

- replay regression tick-by-tick;
- property/invariant tests;
- malformed level tests;
- swapchain/error-path tests onde possível;
- editor undo/redo tests;
- hardware/software capability matrix;
- profiling de frame-time, não apenas FPS médio.

A investigação sobre game testing apoia automação com objetivos/oracles claros; não sustenta substituir todo o playtesting humano por agentes automáticos. Ver `docs/SCIENTIFIC_REFERENCES.md` e `docs/TECHNICAL_REFERENCES.md`.

## P2 — escalabilidade da representação de edição

`EditorRenderSnapshot` copia os AABB do documento para cada atualização. Isto é aceitável na escala atual, mas qualquer mudança para cache, incrementalidade ou estruturas mais complexas deve ser precedida por profiling.

A separação futura pode distinguir dados do documento, estado de interação e estado de validação se o snapshot ficar demasiado acoplado.

## Não tratar como prioridade imediata

- ECS completo;
- CCD universal;
- networking/WebSockets;
- render graph complexo;
- otimizações de instrução sem profiling;
- arquitetura genérica de engine.

Estas técnicas só entram quando um problema medido as justificar.

## Critério para sair da auditoria

A base fica pronta para construção de conteúdo quando:

1. input é totalmente orientado a ações; ✅
2. Level Editor tem uma única interpretação de viewport; 🔄
3. fixed timestep tem limite defensivo; ✅
4. swapchain/queues/error paths são robustos;
5. pontes e caminhos legados de apresentação deixam de ser necessários;
6. gameplay usa dados de apresentação em vez de objetos de domínio diretamente;
7. modelo de dados de nível é uma fronteira comum;
8. paths/runtime data são independentes do source tree;
9. CI cobre Windows + sanitizers + build do jogo conforme a capacidade da infraestrutura;
10. testes cobrem as novas fronteiras;
11. frame-time e custo da simulação são medidos para validar o objetivo de 60/120 FPS conforme o hardware.

## Relação com a investigação

Consultar sempre:

- `docs/TECHNICAL_REFERENCES.md` — Vulkan, C++, profiling, hardware, packaging e testing;
- `docs/SCIENTIFIC_REFERENCES.md` — física, replay, level design, dificuldade e playtesting;
- `docs/RESEARCH_INDEX.md` — subsistema → evidência;
- `docs/PRODUCT_DECISIONS.md` — comportamento pretendido;
- `docs/ARCHITECTURE.md` — arquitetura-alvo;
- `docs/ROADMAP.md` — ordem e gates.

A regra de evidência é separar claramente **requisito da tecnologia**, **evidência empírica**, **decisão de engenharia do ASCENDENDO** e **hipótese que só pode ser decidida por profiling/playtest**.
