# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

Antes de implementar qualquer passo, consultar sempre as documentações relevantes e verificar se alguma decisão recente altera o plano:

- `docs/DEVELOPMENT_PROTOCOL.md` — processo obrigatório de investigação, documentação, validação e continuidade.
- `docs/PROJECT_MANAGEMENT.md` — WBS, dependências, riscos, gates e Definition of Ready/Done.
- `docs/PRODUCT_DECISIONS.md` — decisões de produto e UX; fonte de verdade para comportamento esperado.
- `docs/DESIGN_REFERENCES.md` — referências de jogos e ferramentas maduras.
- `docs/SCIENTIFIC_REFERENCES.md` — evidência académica de gameplay, level design, dificuldade, experiência e câmaras.
- `docs/TECHNICAL_REFERENCES.md` — C++, arquitetura, Vulkan, hardware, testes, packaging e sharing.
- `docs/RESEARCH_INDEX.md` — mapa entre subsistemas e referências.
- `docs/EDITOR_UX_SPEC.md` — critérios verificáveis do editor.
- `docs/CAMPAIGN_EDITOR_DESIGN.md` — comportamento e arquitetura do Campaign Editor.
- `docs/BASE_ARCHITECTURE_AUDIT.md` — auditoria atual da implementação e prioridades de hardening.
- `docs/WORK_PACKAGE_TEMPLATE.md` — contrato operacional de cada work package.

Quando uma implementação contrariar um destes documentos, atualizar primeiro a decisão/documentação correspondente; não criar divergências silenciosas.

## Fonte operacional e proveniência

`docs/ROADMAP.md` é a única fonte de verdade para a **ordem de execução atual**.

`README.md` e documentos históricos não definem a numeração operacional das fases. Números antigos de fases podem ser mantidos como histórico, mas não devem ser usados para abrir novas branches sem primeiro consultar este documento.

Para afirmações importantes, distinguir:

```text
DECISION     escolha de produto/engenharia
REQUIREMENT  comportamento obrigatório
HYPOTHESIS   expectativa ainda testável
EVIDENCE     resultado observado/testado
REFERENCE    fonte externa usada no raciocínio
HISTORICAL   estado de uma implementação anterior
```

## Estado de referência — 2026-08-26

`main` contém os principais blocos de hardening 9.6 e a consolidação `RendererCore` → passes → `RendererFacade`.

O contrato inicial de `RenderSnapshot` está integrado (PR #19), mas a migração geral **não** está concluída. A presentation de gameplay ainda possui dependências diretas de modelos de domínio e a continuação de `RenderSnapshot` permanece bloqueada pelo Base Engineering Gate.

PR #20 foi encerrado/superseded e não é uma tranche ativa. O histórico do PR/commits permanece suficiente para comparação; não manter branches históricas apenas para laboratório.

A decomposição de `FontRenderer` e `SpriteRenderer` já está integrada.

### Estado corrente do Base Engineering Gate

- **Source-size policy:** alinhada entre documentação e checker; `main.cpp` passou a ser coberto. PR #34 integrado.
- **Warnings após a correção do checker:** `SpriteRendererGpu.cpp`, `test_level.cpp` e `main.cpp` ainda requerem avaliação/refatoração quando a inspeção de coesão justificar.
- **Large test files:** `test_keybindings.cpp` está em decomposição por responsabilidade no PR #35; produção não é alterada nessa tranche.
- **Main loop:** `main.cpp` continua concentrando responsabilidades e permanece um alvo arquitetural posterior, não uma divisão por contagem de linhas.
- **CI:** Linux/Clang/headless Vulkan está funcional e produz evidência observável; Windows, sanitizers e matriz de hardware continuam incompletos.
- **RenderSnapshot geral:** bloqueado até o Gate fechar.

## Princípio estratégico

**Construir a base antes de enriquecer o jogo.**

O objetivo é construir um jogo sólido e um editor de níveis suficientemente confiável para sobreviver nas mãos da comunidade. Conteúdo novo entra em camadas sobre esta base.

Prioridades gerais:

- eliminar contradições entre arquitetura documentada e implementação;
- confiabilidade do modelo de nível;
- input e estado com fronteiras únicas;
- renderização desacoplada do runtime;
- determinismo e robustez da simulação;
- Vulkan/CI/runtime resilientes;
- compatibilidade/versionamento;
- playtest e validação explicáveis;
- profiling antes de otimização;
- partilha/importação segura.

## Princípio de evidência

Para decisões importantes, cruzar sempre que possível:

```text
paper/estudo específico
        +
documentação técnica oficial
        +
projeto/jogo maduro
        +
testes/profiling internos
        ↓
decisão ASCENDENDO
```

Nenhuma referência única é autoridade absoluta. As referências de Mario Maker, Jump King, Tiled, Godot e SuperTux são complementares; a física do ASCENDENDO continua própria.

## Fases concluídas

### Fases 1–8 ✅ — feature milestones

Motor, física, campanha, UI, texto TTF, sprites, replay/save e validação estão historicamente concluídos como funcionalidades. Isto **não** significa que todas as propriedades transversais estejam provadas; o hardening 9.6 existe precisamente para fechar essas lacunas.

### 9.1–9.3 ✅

`GameAction`/`KeyBindings`, rato/window→logical, menus clicáveis e `GameState::EDITOR`.

### 9.4 ✅

Editor core + migração incremental do renderer, incluindo `LevelEditorDocument`, `EditorInteractionController`, `EditorSession`, `EditorRenderSnapshot`, `RendererCore`, `ShapeRenderer`, `WorldRenderer`, `UiRenderer`, `EditorRenderer`, `RendererFacade` e o período transitório do adapter.

### 9.5 ✅ — design/evidence foundation

Base científica/técnica, requisitos community-first, contratos de UX/editor e referências práticas. Metas como “~50 níveis” ou “100–200 níveis” são **decisões editoriais**, não resultados científicos universais.

### 9.6 — hardening / consolidação ✅ por blocos; Gate de Engenharia 🔒

Foram integrados input por ações, fixed timestep defensivo, viewport do editor `640x360`, lifecycle/recriação de swapchain, distinção graphics/present, remoção do renderer legado, remoção do adapter, contrato inicial de `RenderSnapshot` e decomposição de `FontRenderer`/`SpriteRenderer`.

O Gate continua aberto porque ainda faltam refatorações estruturais e evidência transversal suficiente.

## Gate atual de engenharia — 2026-08-26 🔒

Antes de nova funcionalidade significativa ou da migração geral de `RenderSnapshot`, fechar este gate.

### WBS

```text
9.6 Base Engineering Gate
├── A — processo e continuidade documental
│   ├── DEVELOPMENT_PROTOCOL
│   ├── PROJECT_MANAGEMENT alignment
│   ├── architecture / roadmap / debt consistency
│   └── provenance / evidence rules
│
├── B — CI observability
│   ├── classify failures from actual workflow evidence
│   ├── recover diagnostics before causal claims
│   ├── document workflow boundaries
│   └── expand/revalidate CI evidence
│
├── C — source-size enforcement ✅
│   ├── line-based policy documented ✅
│   ├── checker migrated from KiB to lines ✅
│   ├── main.cpp included ✅
│   └── remaining warnings converted into justified work packages 🔄
│
├── D — modularity work packages 🔄
│   ├── FontRenderer decomposition ✅
│   ├── SpriteRenderer review/decomposition ✅
│   ├── KeyBindings test decomposition 🔄 (PR #35)
│   ├── Level test cohesion review 🔒
│   └── main.cpp architectural decomposition 🔒
│
└── E — gate review 🔒
    └── only then continue RenderSnapshot / Application extraction
```

### Regra para D — não fazer “line-count gaming”

Uma divisão só é autorizada quando existir uma fronteira de responsabilidade/cohesão clara e uma razão de testabilidade, ownership ou manutenção. O número de linhas é um **sinal de inspeção**, não o critério arquitetural.

### Regra de evidência do Gate

Cada item relevante deve deixar uma cadeia observável:

```text
property
→ test/evidence
→ environment
→ result
→ artifact/documentation
```

CI verde sozinho não fecha o Gate.

### Dependências

```text
A → B/C governance and observability
B/C → D implementation targets
D → E architectural continuation
E → RenderSnapshot / Application extraction
```

### Critério de saída

```text
process protocol reproducible
+ CI failures classified from evidence
+ source-size policy executable and synchronized with documentation
+ justified warnings have explicit work packages or are resolved
+ main.cpp decomposition follows architecture, not line count
+ each refactoring tranche has regression evidence
+ roadmap / architecture / debt match actual state
+ no unresolved base-hardening contradiction blocks the next RenderSnapshot tranche
```

## 9.6 P1 — fronteiras arquiteturais 🔄

7. **Eliminar o adapter de migração ✅** — `RendererFacadeAdapter.cpp` removido e ownership absorvido pela fachada.
8. **Eliminar o `Renderer` legado ✅** — `Renderer.cpp/.h` removidos.
9. **Criar `RenderSnapshot` geral 🔒** — contrato inicial integrado, mas a presentation ainda não é independente dos modelos de domínio.
10. **Extrair responsabilidades do loop principal 🔒** — decompor `main.cpp` por responsabilidades (`Application` / state machine / simulation ou equivalente), sem criar camadas artificiais.
11. **RAII/ownership Vulkan 🔒** — rever `new/delete` evitáveis e wrappers de ownership/copy/move.
12. **Consolidar modelo comum de dados de nível 🔒** — parser, editor, validator e runtime devem convergir num contrato comum.
13. **Undo/Redo transacional 🔒** — um drag completo deve constituir uma operação lógica.
14. **Separar user data de source tree 🔒** — resolução de assets relativa ao executável e diretórios de dados persistentes separados.
15. **Unificar tooling de source-size ✅/🔄** — política e checker estão alinhados; falta decidir e fechar eventuais ferramentas duplicadas e os warnings remanescentes.

## P2 — qualidade, compatibilidade e evidência transversal 🔒

16. Windows build + tests no CI.
17. `make game`/equivalente no CI Windows quando a infraestrutura permitir.
18. ASan/UBSan no CI quando suportado.
19. Replay regression **tick-by-tick**, não apenas por estado final.
20. Property/invariant tests para viewport, snap, física, editor e formatos.
21. Testes de malformed `.lvl` e error paths.
22. Testes de swapchain/error paths automatizáveis.
23. Matriz mínima de hardware/software documentada e validada.
24. Profiling antes de otimização, com cobertura representativa de hardware quando viável.

## Próximo bloco autorizado após o Gate — 9.6 P1.9 RenderSnapshot geral 🔒

Só começar quando o Gate atual estiver fechado e refletido em `main`.

Objetivo: fazer a presentation consumir um modelo de dados próprio, reduzindo a dependência direta de `Player`, `Level` e `GameState`.

### WBS

```text
9.6 P1.9 — RenderSnapshot
├── definir dados necessários por pass
├── separar estado persistente de estado de apresentação
├── criar snapshot de gameplay
├── integrar UI/editor snapshot sem duplicar ownership
├── migrar RendererFacade
├── testes de equivalência do frame
└── profiling/regressão
```

### Critério de saída

```text
RendererFacade não necessita dos modelos de domínio para extrair dados de apresentação
+ snapshot tem ownership/imutabilidade claros
+ gameplay continua determinístico
+ testes cobrem equivalência do caminho antigo/novo
+ documentação arquitetural atualizada
```

## Arquitetura de dificuldade — investigação 🔬

Validade física, dificuldade executiva/motora, desempenho observado e dificuldade percebida são camadas diferentes.

A primeira implementação autorizada é **análise/diagnóstico**, não adaptive difficulty. Antes de qualquer score é necessário definir quais variáveis do ASCENDENDO são observáveis, reproduzíveis e calibráveis.

### Dependências

```text
physics validity
    ↓
motor/executive difficulty
    ↓
player performance
    ↓
perceived difficulty
```

Adaptive difficulty só pode ser considerada depois de telemetry + player modelling + validação com playtesting. Um agente automático não substitui jogador humano e PCG metrics isoladas não provam player experience.

## Fase 9.7 — Level Editor UX 🔒

Só depois do Gate 9.6.

- painel de Controlos configurável/consultável;
- layouts autoajustáveis e letterboxing corretos;
- guardar/playtest/validar explícitos;
- playtest não persiste automaticamente;
- indicador visual de força do salto;
- feedback de erro e trajetória tentada;
- retorno seguro entre editor e jogo.

A especificação deve distinguir requisitos `must`, `should` e `experimental`; alegações de discoverability/usabilidade permanecem hipóteses até testes com utilizadores.

## Fase 9.8 — Seleção de campanhas 🔒

- `Começar` abre seleção mesmo com uma campanha;
- preview, número de níveis e validade;
- metadata separada da validade/runtime;
- metadata derivada/versionada para evitar estados obsoletos.

Fluxo recomendado:

```text
campaign source
→ metadata extraction
→ version/validation
→ cached preview
```

Malformed/missing metadata deve ter testes explícitos.

## Fase 9.9 — Campaign Editor 🔒

- timeline vertical;
- miniaturas compactadas 16:9;
- blocos arrastáveis + snap/reordenação;
- abrir Level Editor via mudança de estado;
- preview sequencial;
- validation runs em background;
- runs de transição entre níveis;
- diagnóstico visual de falhas.

### Critérios adicionais derivados dos estudos

- reorder → save → reload preserva a ordem;
- reordenação repetida é determinística;
- nível inválido continua marcado como inválido após reload;
- cancelamento/late-result de validação assíncrona é race-safe;
- um run de transição identifica exatamente a fronteira onde falhou.

## Fase 10 — Level Data + save + validação estáveis 🔒

Esta fase começa por **evolução de schema**, não apenas por novas features do editor.

### Pipeline normativo

```text
parse
→ validate envelope
→ identify schema version
→ migrate known old version
→ validate migrated representation
→ normalize
→ runtime
```

### Requisitos

- `version` explícito no `.lvl` como parte do contrato;
- migrações documentadas;
- modelo declarativo/extensível;
- serialização determinística;
- separação persistido/runtime;
- fixtures por versão suportada;
- importados/descarregados revalidados pelo EXE.

### Evidência obrigatória

- round-trip semântico;
- versão desconhecida rejeitada explicitamente;
- campos obrigatórios ausentes rejeitados;
- valores fora do domínio rejeitados;
- migração antiga → atual;
- determinismo de serialização;
- corpus de malformed data; fuzzing posteriormente.

## Fase 11 — Conteúdo oficial e análise de campanhas 🔒

Só depois da infraestrutura ser confiável.

- novas superfícies/comportamentos;
- perigos;
- elementos móveis;
- objetos interativos;
- triggers/eventos;
- decoração/identidade visual;
- ferramentas de composição.

### Progressão de campanha

Número de níveis é parâmetro editorial, não verdade científica.

A hipótese a validar é a qualidade da progressão:

```text
introdução
→ prática
→ variação
→ domínio
→ recuperação
→ escalada
→ avaliação
```

A análise deve existir em múltiplas escalas:

```text
salto
→ secção
→ nível
→ sequência de níveis
→ campanha
```

Separar sempre:

```text
validade física
motor/executive difficulty
desempenho observado
experiência percebida
```

### Difficulty analysis

A camada futura deve começar por análise explicável do nível/resultados, sem alterar silenciosamente a física ou o conteúdo authored.

## Fase 12 — Partilha local e web 🔒

### 12.1 Export/import

Pacote declarativo; extração controlada; conteúdo tratado como não confiável; validação obrigatória pelo EXE.

Pipeline:

```text
web/user file
→ parse defensivo
→ size/depth/resource limits
→ schema validation
→ semantic/game validation
→ canonical normalization
→ playable
```

### 12.2 Biblioteca online

HTTP(S) inicialmente. O servidor pode rejeitar conteúdo, mas o EXE continua autoridade final antes de jogar.

### 12.3 Metadata

Separar explicitamente:

```text
claimed_difficulty
observed_difficulty
validation_status
```

A metadata declarada pelo autor não é substituto da dificuldade observada.

### 12.4 Partilha direta

Adicionar comunicação bidirecional apenas se HTTP(S) + export/import se mostrarem insuficientes.

## Fase 13 — Release / Portable Build 🔒

Objetivo: artefacto Windows x64 copiável para outro computador dentro dos requisitos mínimos, sem ambiente de desenvolvimento.

### Critério de release

```text
fresh Windows machine
→ copy/extract package
→ launch
→ first run
→ asset resolution
→ save/user data
→ replay/load
→ import level
→ clean shutdown
```

Guardar manifest das versões/artefactos usados pela build.

A escolha entre ZIP/Xcopy, installer ou outro mecanismo é decisão de distribuição; o requisito técnico é que o artefacto seja reproduzível e não dependa do ambiente de desenvolvimento.

## Regra de progressão entre branches/PRs

Nenhum novo bloco começa antes de integrar a PR anterior em `main`.

Uma branch encerrada sem merge não é preservada por padrão. O histórico de PR/commits é a referência para comparação. Branch de laboratório só deve existir temporariamente quando houver uma experiência ativa que não possa ser feita diretamente a partir de um commit histórico.