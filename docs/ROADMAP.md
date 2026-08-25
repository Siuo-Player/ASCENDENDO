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

## Estado de referência

`main` contém a base integrada através do hardening 9.6 e da consolidação do renderer legado/adapter. `RendererCore` → passes → `RendererFacade` é a rota de presentation do runtime.

A fronteira geral `RenderSnapshot`, contudo, **ainda não está concluída em `main`**: `WorldRenderer` continua a receber diretamente `Player` e `Level`. PR #20 é a tranche ativa dessa migração.

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

**Referências gerais:** `docs/RESEARCH_INDEX.md`, `docs/SCIENTIFIC_REFERENCES.md`, `docs/TECHNICAL_REFERENCES.md`, `docs/DESIGN_REFERENCES.md`.

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

Nenhuma referência única é autoridade absoluta. Mario Maker, Jump King, Tiled, Godot e SuperTux são referências práticas complementares; a física do ASCENDENDO continua própria.

## Fases concluídas

### Fases 1–8 ✅

Motor, física, campanha, UI, texto TTF, sprites, replay/save e validação.

### 9.1–9.3 ✅

`GameAction`/`KeyBindings`, rato/window→logical, menus clicáveis e `GameState::EDITOR`.

### 9.4 ✅

Editor core + migração incremental do renderer, incluindo `LevelEditorDocument`, `EditorInteractionController`, `EditorSession`, `EditorRenderSnapshot`, `RendererCore`, `ShapeRenderer`, `WorldRenderer`, `UiRenderer`, `EditorRenderer`, `RendererFacade` e o período transitório do adapter.

### 9.5 ✅

Base de investigação científica/técnica, requisitos community-first, referências a Mario Maker/Jump King/Tiled/Godot/SuperTux, objetivos de campanhas de ~50 e 100–200 níveis, e contratos de UX/editor.

### 9.6 — hardening / consolidação ✅

A base passou pelas principais correções de input, fixed timestep, viewport, lifecycle Vulkan e reconstrução de swapchain. A migração de presentation foi consolidada até `RendererFacade`, incluindo a remoção do renderer legado e da implementação do adapter.

**Validação histórica relevante:** a tranche 9.4 tinha 162/162 testes e 849/849 assertions no Windows; a 9.6 adicionou invariantes e failure paths adicionais.

## Gate atual de engenharia — 2026-08-25 🔒

Antes de avançar para a implementação de nova funcionalidade ou para a continuação estrutural de `RenderSnapshot`, o repositório deve fechar este gate de base.

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
│   ├── classify current run #281
│   ├── recover diagnostics before causal claims
│   ├── document workflow boundaries
│   └── revalidate CI
│
├── C — source-size enforcement
│   ├── document byte/KiB policy
│   ├── keep checker aligned with 30/36 KiB enforcement
│   ├── decide explicitly whether main.cpp belongs in the checked roots
│   └── validate warnings/errors
│
├── D — modularity work packages
│   ├── FontRenderer decomposition
│   ├── SpriteRenderer review/decomposition
│   ├── main.cpp architectural decomposition
│   └── large test file decomposition
│
└── E — gate review
    └── only then continue RenderSnapshot / Application extraction
```

### Dependências

```text
A → B/C documentation and governance
B/C → D implementation targets
D → E architectural continuation
E → 9.6 P1.9 RenderSnapshot
```

### Critério de saída

```text
process protocol merged and reproducible
+ CI failures classified from evidence
+ source-size policy is implemented and validated
+ oversized/warning files have documented WPs and progress
+ main.cpp decomposition follows architecture, not metric gaming
+ tests validate each refactoring tranche
+ roadmap / architecture / debt reflect actual state
+ next RenderSnapshot block has no unresolved base-hardening contradiction
```

## 9.6 P1 — fronteiras arquiteturais 🔄

7. **Eliminar o adapter de migração ✅** — `RendererFacadeAdapter.cpp` removido, snapshot/editor ownership absorvido por `RendererFacade` e runtime migrado para `RendererFacade`.
8. **Eliminar o `Renderer` legado ✅** — `Renderer.cpp/.h` removidos em 9.8.
9. **Criar `RenderSnapshot` geral 🔄** — presentation ainda recebe `Player`/`Level` diretamente em `WorldRenderer`/`RendererFacade`; esta tranche só prossegue depois do Gate atual de engenharia.
10. **Extrair responsabilidades do loop principal 🔄** — reduzir o acoplamento de `main.cpp` através de `Application` / `GameStateMachine` / `Simulation`, depois de estabilizar `RenderSnapshot`.
11. **RAII/ownership Vulkan** — substituir `new/delete` evitáveis e garantir wrappers não-copiáveis/movíveis quando apropriado.
12. **Consolidar modelo comum de dados de nível** entre parser, runtime e editor.
13. **Undo/Redo transacional** — drag completo = uma operação lógica.
14. **Separar user data de source tree** e introduzir resolução de assets baseada na localização do executável.
15. **Unificar a política de source-size** e remover ferramentas legadas duplicadas.

**Referências:** `docs/ARCHITECTURE.md`, `docs/TECHNICAL_REFERENCES.md`, `docs/BASE_ARCHITECTURE_AUDIT.md`, `docs/PROJECT_MANAGEMENT.md`, `docs/DEVELOPMENT_PROTOCOL.md`.

### P2 — qualidade e compatibilidade

16. Windows build + tests no CI.
17. `make game` no CI.
18. ASan/UBSan no CI quando suportado.
19. Replay regression tick-by-tick.
20. Property/invariant tests para viewport, snap, física, editor e formatos.
21. Testes de malformed `.lvl` e error paths.
22. Testes de swapchain/error paths onde forem automatizáveis.
23. Matriz mínima de hardware/software documentada e validada.
24. Profiling em pelo menos Intel/NVIDIA/AMD e, quando viável, uma GPU tile-based.

## Próximo bloco autorizado após o Gate — 9.6 P1.9 RenderSnapshot geral

Só começar quando o Gate atual estiver fechado e a tranche documental/CI estiver refletida em `main`.

Objetivo: fazer com que a presentation consuma um modelo de dados próprio, reduzindo a dependência direta de `Player`, `Level` e `GameState`.

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

### Dependências

- `RendererFacade` estável;
- `EditorRenderSnapshot` existente;
- `GameState` extraído do renderer legado;
- testes de integração do renderer;
- Gate atual de engenharia fechado.

### Critério de saída

```text
RendererFacade não necessita dos modelos de domínio para extrair dados de apresentação
+ snapshot tem ownership/imutabilidade claros
+ gameplay continua determinístico
+ testes cobrem equivalência do caminho antigo/novo
+ documentação arquitetural atualizada
```

## Fase 9.7 — Level Editor UX

Só depois de 9.6 verde.

- painel de Controlos configurável/consultável;
- rodapé contextual;
- layouts autoajustáveis;
- fullscreen + letterboxing corretos;
- guardar/playtest/validar explícitos;
- playtest não persiste automaticamente;
- indicador visual de força do salto;
- feedback de erro e trajetória tentada;
- retorno seguro entre editor e jogo.

## Fase 9.8 — Seleção de campanhas

- `Começar` abre seleção mesmo com uma campanha;
- preview, número de níveis e validade;
- metadata separada quando necessário;
- preparação para várias campanhas.

## Fase 9.9 — Campaign Editor

- timeline vertical;
- miniaturas 16:9 compactadas;
- blocos arrastáveis + snap/reordenação;
- abrir Level Editor via mudança de estado;
- preview sequencial;
- runs em background por nível;
- runs de transição entre níveis;
- diagnóstico visual de falhas.

## Fase 10 — Level Data + save + validação estáveis

- versão explícita do `.lvl`;
- modelo declarativo/extensível de dados de nível;
- serialização determinística;
- separação entre persistido/runtime;
- validação em background;
- importados/descarregados revalidados pelo EXE;
- migrações documentadas.

## Fase 11 — Conteúdo oficial e análise de campanhas

Depois da infraestrutura ser confiável:

- novas superfícies/comportamentos;
- perigos;
- elementos móveis;
- objetos interativos;
- triggers/eventos;
- decoração/identidade visual;
- ferramentas de composição;
- análise de dificuldade/ritmo em salto → secção → nível → campanha.

## Fase 12 — Partilha local e web

### 12.1 Export/import

Pacote declarativo, extração controlada e validação obrigatória pelo EXE.

### 12.2 Biblioteca online

HTTP(S) inicialmente; site trata conteúdo como não confiável; EXE continua autoridade final.

### 12.3 Partilha direta

Só adicionar comunicação bidirecional quando HTTP(S) + export/import forem insuficientes.

## Fase 13 — Release / Portable Build

Objetivo: pacote Windows x64 copiável para outro computador dentro dos requisitos mínimos, sem ambiente de desenvolvimento.

## Regra de progressão entre branches/PRs

Nenhum novo bloco começa antes de integrar a PR anterior em `main`.

Para cada passo:

1. consultar `DEVELOPMENT_PROTOCOL.md` e documentação relevante;
2. verificar problemas imediatos da base;
3. documentar descobertas/decisões antes da alteração correspondente;
4. corrigir a base antes de adicionar complexidade;
5. escrever testes relevantes;
6. implementar apenas o escopo do passo;
7. atualizar documentação e roadmap;
8. abrir PR própria;
9. validar;
10. fazer merge;
11. fechar a branch e criar a próxima a partir do `main` atualizado.

Toda decisão nova deve atualizar `PRODUCT_DECISIONS.md` e, quando alterar o plano, este roadmap. O WBS/risco/dependências do bloco devem acompanhar a mesma alteração.
