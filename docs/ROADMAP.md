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

## Estado de referência — 2026-08-27

`main` contém o hardening incremental 9.6, a consolidação `RendererCore` → passes → `RendererFacade`, o contrato inicial de `RenderSnapshot` e as decomposições validadas de `FontRenderer`, `SpriteRenderer`, testes de `KeyBindings` e testes de `Level`.

Também estão integrados:

- `GameStateMachine` e a sua utilização pelo `main.cpp`;
- `SimulationOrchestrator` e a delegação do fixed-step pelo entry point;
- ownership de `RendererFacade` baseado em `std::unique_ptr`;
- lifetime de GLFW protegido por RAII;
- política única de source-size em **KiB** (`40 KiB` warning / `48 KiB` hard limit), com teste próprio e checker canónico;
- `PresentationRuntime` ligado ao `main.cpp` sem ownership direto dos pipelines/renderers de apresentação;
- workflow Linux dedicado de ASan/UBSan que instrumenta também os objetos de produção `Game` usados pelos testes.

A migração geral de `RenderSnapshot` **continua bloqueada pelo Base Engineering Gate**: a presentation de gameplay ainda possui dependências diretas dos modelos de domínio.

### Evidência recente

- PR #37 — decomposição dos testes de `KeyBindings`, integrada após CI verde;
- PR #39 — decomposição dos testes de `Level`, integrada após CI verde;
- PR #41 — `GameStateMachine`, integrada após CI verde;
- PR #42 — contrato adicional de transições, integrado;
- PR #43 — wiring da `GameStateMachine` no `main.cpp`, integrado;
- PR #44 — fronteira `SimulationOrchestrator`, integrada;
- PR #45 — organização/indexação documental, integrada;
- PR #46 — wiring da simulação no `main.cpp`, integrado;
- PR #47 — ownership/lifetime work package, integrado;
- PR #48 — ownership RAII do `RendererFacade`, integrado;
- PR #49 — política KiB + GLFW RAII, integrado;
- PR #69 — wiring de `PresentationRuntime` no `main.cpp`, integrado;
- PR #70 — cobertura explícita Linux/Clang ASan + UBSan para produção + testes, integrado;
- PR #72 — primeira extração arquitetural de `main.cpp` para `logic::GameSession`, integrada após `Tests #746` e `Sanitizers #8` verdes;
- PR #85 — evidência Windows scoped: runner Windows `windows-2025-vs2026`, Clang/MSVC, Vulkan SDK + Lavapipe, GLFW pinned, `make game`, `make tests`, campanha e artifact de evidência validados no head final `d0d8a4954b73924d91bed444bb9034cb23e52fa6`.

A evidência Linux agora cobre workflow normal, headless Vulkan, campanha e ASan/UBSan. A evidência Windows está demonstrada no runner selecionado pelo PR #85. A matriz mais ampla de hardware/capabilities e o endurecimento específico de Vulkan lifecycle/queues continuam sem encerramento global.

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
├── C — source-size enforcement                         ✅ KiB
├── D — modularidade                                    🔄
│   ├── FontRenderer                                    ✅
│   ├── SpriteRenderer                                  ✅
│   ├── KeyBindings tests                               ✅
│   ├── Level tests                                     ✅
│   ├── GameStateMachine boundary + wiring              ✅
│   ├── SimulationOrchestrator + wiring                 ✅
│   ├── PresentationRuntime + wiring                    ✅
│   ├── GameSession runtime/session boundary            ✅
│   └── remaining main.cpp decomposition                🔄
└── E — Gate review                                     🔒
```

### Regra de modularidade

Não dividir ficheiros apenas para baixar tamanho. Uma divisão exige fronteira de responsabilidade clara e ganho em ownership, testabilidade, diagnóstico ou manutenção. O tamanho físico em KiB é apenas sinal de inspeção e guardrail de manutenção.

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

### Concluído nesta tranche de base

- adapter de migração removido;
- renderer legado removido;
- `FontRenderer` decomposto;
- `SpriteRenderer` decomposto/revisto;
- testes de `KeyBindings` separados por responsabilidade;
- testes de `Level` separados em geometria, colisão/física e file loading;
- `GameStateMachine` isolada, testada e ligada ao `main.cpp`;
- fixed-step delegado a `SimulationOrchestrator`;
- ownership de `RendererFacade` convertido para RAII;
- lifetime global de GLFW convertido para RAII;
- checker de source-size unificado em KiB;
- `PresentationRuntime` ligado ao entry point, retirando a ownership direta da apresentação do `main.cpp`;
- `GameSession` isolada e ligada ao entry point, retirando a ownership e política de sessão do `main.cpp`.

### Próximo: continuar decomposição de `main.cpp` 🔒

A primeira fronteira foi validada: `GameSession` agora possui `GameStateMachine`, `CampaignRuntime`, `Level`, `PhysicsWorld`, `SimulationOrchestrator`, `Player` e `EditorSession`. `main.cpp` mantém process/GLFW lifetime, runtime-path bootstrap, composição gráfica/apresentação, input polling, `Camera` e frame submission.

A próxima investigação deve mapear o restante bootstrap/configuration antes de extrair uma nova fronteira. Não criar uma `Application` nominal sem uma responsabilidade de composição/lifecycle que seja claramente isolável.

Alvo de trabalho:

```text
Process/bootstrap
├── GLFW lifetime
├── RuntimePaths/config bootstrap
├── campaign discovery
└── runtime service composition

Frame composition
├── input polling
├── GameSession update
├── camera/presentation coordination
└── renderer submission
```

`GameStateMachine`, `SimulationOrchestrator` e `GameSession` são fronteiras existentes; não devem ser duplicadas dentro de outra classe.

### P0 técnico paralelo — Vulkan lifecycle/queues

Antes de fechar o Gate, verificar explicitamente:

- acquire/reset/submit/present sem deadlock em error paths;
- `VK_ERROR_OUT_OF_DATE_KHR` e `VK_SUBOPTIMAL_KHR` tratados;
- fence reset apenas quando existe caminho garantido para submissão;
- graphics queue e present queue não assumidas como a mesma family;
- capability matrix mínima para queues, extensions, features e surface.

Estes itens são requisitos técnicos do Vulkan e não apenas refatoração estética.

### Dívida já descoberta nos testes

Os testes de `appendFromFile` usam nomes temporários fixos. Isto fica registado separadamente como trabalho de isolamento/concorrência; não deve ser misturado com a decomposição de `Level`.

### Depois da decomposição de `main.cpp`

- contrato comum de dados de nível entre parser/editor/validator/runtime;
- Undo/Redo transacional;
- separação de user data e source tree;
- limpeza adicional de tooling duplicado, se algum existir;
- revisão do Base Engineering Gate.

## 9.6 P2 — Evidência transversal 🔒

- **Windows build + tests no CI** ✅ — PR #85, runner `windows-2025-vs2026`, com software Vulkan Lavapipe e artifact de evidência;
- `make game`/equivalente no Windows ✅ — PR #85;
- **Linux ASan/UBSan** ✅ — PR #70, workflow independente, com instrumentação dos objetos de produção `Game` usados pelos testes;
- replay regression tick-by-tick;
- property/invariant tests;
- malformed `.lvl` e error paths;
- swapchain/error-path tests;
- matriz mínima hardware/software;
- profiling antes de otimização.

O Linux sanitizer gap original está encerrado como evidência. A evidência Windows está encerrada **apenas no scope do runner/ambiente validado por #85**; não implica compatibilidade universal com qualquer configuração Windows. O Gate continua aberto porque replay determinístico, malformed/error paths, Vulkan failure-path/queue evidence, matriz mais ampla e arquitetura do entry point ainda não foram fechados.

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

O estudo atualizado acrescenta uma cautela importante: um agente automático é um **avaliador com perfil de capacidade/estilo conhecido**, não um substituto universal do jogador humano.

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

Branch encerrada deve ser removida; histórico permanece em Git/PR.