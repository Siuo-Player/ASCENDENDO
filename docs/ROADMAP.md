# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

Antes de implementar qualquer passo, consultar sempre:

- `docs/DEVELOPMENT_PROTOCOL.md` — processo de investigação, documentação e validação;
- `docs/PROJECT_MANAGEMENT.md` — WBS, dependências, riscos e gates;
- `docs/PRODUCT_DECISIONS.md` — decisões de produto/UX;
- `docs/BASE_ARCHITECTURE_AUDIT.md` — estado técnico e prioridades de hardening;
- `docs/WORK_PACKAGE_TEMPLATE.md` — contrato operacional de cada tranche;
- referências científicas/técnicas relevantes e os estudos de `PROJECT-STUDIES/ASCENDENDO`.

Não criar divergências silenciosas entre implementação, documentação e estudos.

## Fonte operacional e proveniência

`docs/ROADMAP.md` é a única fonte de verdade para a **ordem de execução**.

PRs e commits são o histórico. Branches antigas não são mantidas como laboratório permanente; uma experiência histórica pode ser recriada a partir do commit/PR necessário.

Para afirmações importantes distinguir:

```text
DECISION     escolha de produto/engenharia
REQUIREMENT  comportamento obrigatório
HYPOTHESIS   expectativa ainda testável
EVIDENCE     resultado observado
REFERENCE    fonte externa
HISTORICAL   estado anterior
```

## Estado de referência — 2026-08-26

`main` contém o hardening 9.6, a consolidação `RendererCore` → passes → `RendererFacade`, o contrato inicial de `RenderSnapshot` e as decomposições já validadas de `FontRenderer`, `SpriteRenderer`, `KeyBindings` tests e `Level` tests.

O contrato inicial de `RenderSnapshot` está integrado (PR #19), mas a migração geral **continua bloqueada pelo Base Engineering Gate**: a presentation de gameplay ainda possui dependências diretas dos modelos de domínio.

### Evidência recente

- PR #34 — source-size gate corrigido e integrado;
- PR #36 — RoadMap sincronizado com os estudos;
- PR #37 — decomposição dos testes de `KeyBindings`, integrada após CI verde;
- PR #39 — decomposição dos testes de `Level`, integrada após CI verde;
- CI validou **167/167 test cases e 901/901 assertions**, build do jogo e validação da campanha nos blocos recentes.

O estado deve continuar a ser tratado como evidência Linux/Clang/headless Vulkan. Windows, sanitizers e matriz de hardware continuam fora da evidência automática principal.

## Princípio estratégico

**Construir a base antes de enriquecer o jogo.**

Prioridades:

1. coerência entre arquitetura, código, documentação e estudos;
2. confiabilidade do modelo de nível e do runtime;
3. fronteiras únicas para input/estado;
4. presentation desacoplada do domínio;
5. determinismo e robustez da simulação;
6. Vulkan/CI/runtime resilientes;
7. versionamento e compatibilidade;
8. validação/playtesting explicáveis;
9. partilha/importação segura.

## 9.6 — Base Engineering Gate 🔒

O Gate fecha antes de nova feature significativa ou da migração geral de `RenderSnapshot`.

### WBS atual

```text
9.6 Base Engineering Gate
├── A — processo/documentação/evidência                 ✅
├── B — CI observability                                ✅ parcial
├── C — source-size enforcement                         ✅
├── D — modularidade                                    🔄
│   ├── FontRenderer                                   ✅
│   ├── SpriteRenderer                                 ✅
│   ├── KeyBindings tests                              ✅
│   ├── Level tests                                    ✅
│   └── main.cpp architectural decomposition           🔒 próximo
└── E — Gate review                                    🔒
```

### Regra de modularidade

Não dividir ficheiros apenas para baixar linhas. Uma divisão exige fronteira de responsabilidade clara e ganho em ownership, testabilidade, diagnóstico ou manutenção. O tamanho é apenas sinal de inspeção.

### Cadeia de evidência

```text
property
→ test/evidence
→ environment
→ result
→ artifact/documentation
```

CI verde isolado não fecha o Gate.

## 9.6 P1 — Fronteiras arquiteturais

### Concluído

- adapter de migração removido;
- renderer legado removido;
- `FontRenderer` decomposto;
- `SpriteRenderer` decomposto/revisto;
- testes de `KeyBindings` separados por responsabilidade;
- testes de `Level` separados em geometria, colisão/física e file loading.

### Próximo: `main.cpp` 🔒

`main.cpp` continua concentrando bootstrap gráfico, carregamento de campanha, configuração, criação do estado de runtime, state machine, simulation loop, editor transitions, persistence e render submission.

A decomposição será feita por responsabilidade real, não por número de linhas.

Alvo arquitetural inicial:

```text
Application/bootstrap
├── platform/Vulkan bootstrap
├── asset/config/campaign loading
└── runtime services

GameStateMachine
├── MENU
├── PLAYING
├── PAUSED
├── CREDITS
└── EDITOR

Frame/Runtime loop
├── input
├── fixed-step simulation
├── streaming
├── state transitions
└── presentation submission
```

O desenho final pode mudar após inspeção de ownership e dependências; esta estrutura é hipótese de trabalho, não decisão irrevogável.

### Dívida já descoberta nos testes

Os testes de `appendFromFile` usam nomes temporários fixos. Isto fica registado separadamente como trabalho de isolamento/concorrência; não deve ser misturado com a decomposição de `Level`.

### Depois de `main.cpp`

- RAII/ownership Vulkan;
- contrato comum de dados de nível entre parser/editor/validator/runtime;
- Undo/Redo transacional;
- separação de user data e source tree;
- limpeza de tooling duplicado.

## 9.6 P2 — Evidência transversal 🔒

- Windows build + tests no CI;
- `make game`/equivalente no Windows;
- ASan/UBSan quando suportado;
- replay regression tick-by-tick;
- property/invariant tests;
- malformed `.lvl` e error paths;
- swapchain/error-path tests;
- matriz mínima hardware/software;
- profiling antes de otimização.

Estes itens são importantes para o Gate completo, mas não devem bloquear uma decomposição estrutural pequena quando a propriedade já está claramente isolada e validada no ambiente suportado.

## 9.6 P1.9 — RenderSnapshot geral 🔒

Só começar depois da revisão do Gate e da decomposição necessária do entry point.

Objetivo: fazer a presentation consumir um modelo próprio, em vez de extrair estado diretamente de `Player`, `Level`, `GameState` e equivalentes.

```text
runtime/domain state
        ↓
RenderSnapshot
        ↓
RendererFacade / passes
```

### WBS

- identificar dados necessários por pass;
- separar estado persistente de estado de apresentação;
- criar snapshot de gameplay;
- integrar UI/editor sem duplicar ownership;
- migrar `RendererFacade`;
- testes de equivalência do frame antigo/novo;
- profiling/regressão.

### Critério de saída

```text
RendererFacade não precisa de modelos de domínio para extrair presentation data
+ ownership/imutabilidade claros
+ gameplay determinístico
+ equivalência coberta por testes
+ arquitetura/documentação atualizadas
```

## Investigação de dificuldade 🔬

Validade física, dificuldade motora/executiva, desempenho observado e dificuldade percebida são dimensões diferentes.

A primeira implementação autorizada é diagnóstico/análise, não adaptive difficulty.

```text
physics validity
    ↓
motor/executive difficulty
    ↓
player performance
    ↓
perceived difficulty
```

Agentes automáticos e métricas de PCG não substituem validação humana de player experience.

## Fase 9.7 — Level Editor UX 🔒

Só depois do Gate 9.6.

- controlos configuráveis/consultáveis;
- letterboxing/layout consistente;
- save/playtest/validate explícitos;
- playtest sem persistência acidental;
- feedback de salto/erro/trajectória;
- retorno seguro editor ↔ jogo.

Requisitos devem ser classificados como `must`, `should` ou `experimental`; usabilidade/discoverability continuam hipóteses até testes com utilizadores.

## Fase 9.8 — Seleção de campanhas 🔒

- seleção explícita mesmo com uma campanha;
- preview + número de níveis + validade;
- metadata separada da validade/runtime;
- metadata versionada para evitar estados obsoletos.

```text
campaign source
→ metadata extraction
→ version/validation
→ cached preview
```

Malformed/missing metadata precisa de testes explícitos.

## Fase 9.9 — Campaign Editor 🔒

- timeline vertical;
- thumbnails 16:9;
- drag + snap/reordenação;
- transições explícitas editor ↔ level editor;
- preview sequencial;
- validation runs em background;
- transition runs;
- diagnóstico visual.

Requisitos derivados dos estudos:

- reorder → save → reload preserva ordem;
- reordenação repetida determinística;
- invalididade preservada após reload;
- late-result/cancelamento race-safe;
- transition run identifica a fronteira da falha.

## Fase 10 — Level Data + save + validação 🔒

Começar por evolução de schema.

```text
parse
→ validate envelope
→ identify schema version
→ migrate known old version
→ validate migrated representation
→ normalize
→ runtime
```

Requisitos:

- `version` explícito;
- migrações documentadas;
- serialização determinística;
- persistido ≠ runtime;
- fixtures por versão suportada;
- revalidação pelo EXE.

Evidência:

- round-trip semântico;
- versão desconhecida rejeitada;
- campos obrigatórios ausentes rejeitados;
- valores fora do domínio rejeitados;
- migração old → current;
- malformed corpus/fuzzing posteriormente.

## Fase 11 — Conteúdo oficial + análise de campanhas 🔒

Só depois da infraestrutura ser confiável.

Conteúdo previsto: superfícies, perigos, elementos móveis/interativos, triggers/eventos, decoração e ferramentas de composição.

Número de níveis é decisão editorial. A hipótese relevante é qualidade da progressão:

```text
introdução
→ prática
→ variação
→ domínio
→ recuperação
→ escalada
→ avaliação
```

Analisar em escalas `salto → secção → nível → sequência → campanha`, separando validade física, dificuldade motora, desempenho observado e experiência percebida.

## Fase 12 — Partilha local e web 🔒

### Export/import

Conteúdo externo é não confiável e deve passar por:

```text
parse defensivo
→ limits
→ schema validation
→ semantic/game validation
→ canonical normalization
→ playable
```

### Biblioteca web

HTTP(S) inicialmente. O servidor pode rejeitar conteúdo; o EXE continua a autoridade final antes de jogar.

### Metadata

Separar:

```text
claimed_difficulty
observed_difficulty
validation_status
```

### Partilha direta

Só adicionar comunicação bidirecional se HTTP(S) + export/import forem insuficientes.

## Fase 13 — Release / Portable Build 🔒

Objetivo: artefacto Windows x64 reproduzível e copiável para máquina limpa, sem ambiente de desenvolvimento.

```text
fresh Windows machine
→ extract/copy
→ launch
→ first run
→ asset resolution
→ save/user data
→ replay/load
→ import level
→ clean shutdown
```

Guardar manifest das versões/artefactos usados.

## Regra de progressão entre branches/PRs

```text
INVESTIGAR
   ↓
DOCUMENTAR imediatamente
   ↓
IMPLEMENTAR numa branch curta/coesa
   ↓
TESTAR e recolher evidência
   ↓
PR
   ↓
MERGE em main
   ↓
apagar branch
   ↓
abrir próxima branch a partir do main atualizado
```

Branch encerrada sem merge não é preservada por padrão. Histórico de PR/commits substitui branches-laboratório permanentes.