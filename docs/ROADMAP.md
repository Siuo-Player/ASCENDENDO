# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

Antes de implementar qualquer passo, consultar sempre as documentações relevantes e verificar se alguma decisão recente altera o plano:

- `docs/PRODUCT_DECISIONS.md` — decisões de produto e UX; fonte de verdade para comportamento esperado.
- `docs/DESIGN_REFERENCES.md` — referências de jogos e ferramentas maduras.
- `docs/SCIENTIFIC_REFERENCES.md` — evidência académica de gameplay, level design, dificuldade, experiência e câmaras.
- `docs/TECHNICAL_REFERENCES.md` — C++, arquitetura, Vulkan, hardware, testes, packaging e sharing.
- `docs/RESEARCH_INDEX.md` — mapa entre subsistemas e referências.
- `docs/EDITOR_UX_SPEC.md` — critérios verificáveis do editor.
- `docs/CAMPAIGN_EDITOR_DESIGN.md` — comportamento e arquitetura do Campaign Editor.
- `docs/BASE_ARCHITECTURE_AUDIT.md` — auditoria atual da implementação e prioridades de hardening.

Quando uma implementação contrariar um destes documentos, atualizar primeiro a decisão/documentação correspondente; não criar divergências silenciosas.

## Estado de referência

`main` contém a base integrada até à 9.4 + tranche documental 9.5. O renderer já foi dividido e o editor já possui snapshot próprio, mas a migração arquitetural ainda não está concluída.

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

Editor core + migração incremental do renderer, incluindo `LevelEditorDocument`, `EditorInteractionController`, `EditorSession`, `EditorRenderSnapshot`, `RendererCore`, `ShapeRenderer`, `WorldRenderer`, `UiRenderer`, `EditorRenderer`, `RendererFacade` e `RendererFacadeAdapter`.

**Referências:** `docs/EDITOR_UX_SPEC.md`, `docs/CAMPAIGN_EDITOR_DESIGN.md`, `docs/DESIGN_REFERENCES.md`, `docs/TECHNICAL_REFERENCES.md`.

### 9.5 — documentação/design ✅

Base de investigação científica/técnica, requisitos community-first, referências a Mario Maker/Jump King/Tiled/Godot/SuperTux, objetivos de campanhas de ~50 e 100–200 níveis, e contratos de UX/editor.

**Referências:** `docs/SCIENTIFIC_REFERENCES.md`, `docs/TECHNICAL_REFERENCES.md`, `docs/DESIGN_REFERENCES.md`, `docs/RESEARCH_INDEX.md`.

## Fase 9.6 — HARDENING DA BASE (próximo passo)

**Esta é a próxima fase de implementação. Não iniciar conteúdo novo significativo antes de fechar os P0 e o núcleo dos P1.**

### P0 — contradições funcionais e riscos graves

1. **Input único** — migrar `Player` e restante gameplay de teclas físicas para `GameAction`/`KeyBindings`.
2. **Editor de uma tela** — eliminar qualquer comportamento de camera-pan inconsistente; cursor e renderer devem usar a mesma transformação e respeitar o contrato `640x360`.
3. **Fixed timestep robusto** — limitar catch-up, definir política para minimização/frames longos e rejeitar `NaN`/`Inf`.
4. **Vulkan error lifecycle** — proteger o ciclo de fences/submit/present contra estados irrecuperáveis.
5. **Swapchain recreation** — tratar `VK_ERROR_OUT_OF_DATE_KHR`/`VK_SUBOPTIMAL_KHR` e reconstrução segura dos recursos dependentes.
6. **Queue/present support** — separar verificação de graphics e present queues; não assumir que são a mesma família.

**Referências:** `docs/BASE_ARCHITECTURE_AUDIT.md`; `docs/TECHNICAL_REFERENCES.md`; `docs/ARCHITECTURE.md`; `docs/RESEARCH_INDEX.md` (Input, Physics, Vulkan, Hardware).

### P1 — fechar as novas fronteiras arquiteturais

7. **Eliminar o adapter de migração** depois de todos os consumidores/testes passarem para `RendererFacade`.
8. **Eliminar o `Renderer` legado** depois da migração dos seus testes e consumidores.
9. **Criar `RenderSnapshot` geral** para gameplay/UI/editor; presentation não recebe `Player`/`Level`/`GameState` diretamente.
10. **Extrair `Application` / `GameStateMachine` / `Simulation`** de `main.cpp` de forma incremental.
11. **RAII/ownership Vulkan** — substituir `new/delete` evitáveis e garantir wrappers não-copiáveis/movíveis quando apropriado.
12. **Consolidar `LevelData`** como fronteira comum entre parser, runtime e editor.
13. **Undo/Redo transacional** — drag completo = uma operação lógica.
14. **Separar user data de source tree** e introduzir resolução de assets baseada na localização do executável.
15. **Unificar a política de source-size** e remover ferramentas legadas duplicadas.

**Referências:** `docs/ARCHITECTURE.md`, `docs/TECHNICAL_REFERENCES.md`, `docs/BASE_ARCHITECTURE_AUDIT.md`.

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

**Referências:** `docs/TECHNICAL_REFERENCES.md`, `docs/SCIENTIFIC_REFERENCES.md`, `docs/RESEARCH_INDEX.md`.

### Critério de saída da 9.6

A base só é considerada pronta quando:

```text
GameAction completo
+ editor 640x360 coerente
+ timestep defensivo
+ Vulkan error/swapchain robustos
+ graphics/present capabilities verificadas
+ adapter/renderer legado encaminhados para remoção
+ RenderSnapshot geral
+ LevelData comum
+ paths/user-data corretos
+ CI/quality gates essenciais
+ testes das novas fronteiras
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

**Referências:** `docs/EDITOR_UX_SPEC.md`, `docs/PRODUCT_DECISIONS.md`, `docs/DESIGN_REFERENCES.md`, `docs/SCIENTIFIC_REFERENCES.md`.

## Fase 9.8 — Seleção de campanhas

- `Começar` abre seleção mesmo com uma campanha;
- preview, número de níveis e validade;
- metadata separada quando necessário;
- preparação para várias campanhas.

**Referências:** `docs/PRODUCT_DECISIONS.md`, `docs/CAMPAIGN_EDITOR_DESIGN.md`, `docs/SCIENTIFIC_REFERENCES.md`, `docs/DESIGN_REFERENCES.md`.

## Fase 9.9 — Campaign Editor

- timeline vertical;
- miniaturas 16:9 compactadas;
- blocos arrastáveis + snap/reordenação;
- abrir Level Editor via mudança de estado;
- preview sequencial;
- runs em background por nível;
- runs de transição entre níveis;
- diagnóstico visual de falhas.

**Referências:** `docs/CAMPAIGN_EDITOR_DESIGN.md`, `docs/DESIGN_REFERENCES.md`, `docs/SCIENTIFIC_REFERENCES.md`, `docs/PRODUCT_DECISIONS.md`.

## Fase 10 — Level Data + save + validação estáveis

- versão explícita do `.lvl`;
- `LevelData` declarativo/extensível;
- serialização determinística;
- separação entre persistido/runtime;
- validação em background;
- importados/descarregados revalidados pelo EXE;
- migrações documentadas.

**Referências:** `docs/TECHNICAL_REFERENCES.md`, `docs/DESIGN_REFERENCES.md`, `docs/PRODUCT_DECISIONS.md`, `docs/RESEARCH_INDEX.md`.

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

Objetivos oficiais:

- várias campanhas;
- campanhas normais com média de ~50 níveis;
- campanhas especiais de 100–200 níveis;
- níveis finais/opcionais extremamente difíceis;
- progressão baseada em aprendizagem, variação, recuperação e escalada de desafio.

**Referências:** `docs/SCIENTIFIC_REFERENCES.md` (level design, difficulty, progression, UGC); `docs/DESIGN_REFERENCES.md` (Mario Maker, Jump King, SuperTux); `docs/RESEARCH_INDEX.md`; `docs/PRODUCT_DECISIONS.md`.

## Fase 12 — Partilha local e web

### 12.1 Export/import

Pacote declarativo, extração controlada e validação obrigatória pelo EXE.

### 12.2 Biblioteca online

HTTP(S) inicialmente; site trata conteúdo como não confiável; EXE continua autoridade final.

Metadata futura: autor, versão, hash/ID, descrição, dificuldade declarada/observada, versão do jogo, estatísticas e reports/moderação.

### 12.3 Partilha direta

Só adicionar comunicação bidirecional quando HTTP(S) + export/import forem insuficientes.

**Referências:** `docs/TECHNICAL_REFERENCES.md`, `docs/PRODUCT_DECISIONS.md`, `docs/SCIENTIFIC_REFERENCES.md`, `docs/DESIGN_REFERENCES.md`, `docs/RESEARCH_INDEX.md`.

## Fase 13 — Release / Portable Build

Objetivo: pacote Windows x64 copiável para outro computador dentro dos requisitos mínimos, sem ambiente de desenvolvimento.

Requisitos: executável + DLLs/assets necessárias, paths independentes do current working directory, sem downloads obrigatórios e diagnóstico amigável.

**Referências:** `docs/TECHNICAL_REFERENCES.md`, `docs/RESEARCH_INDEX.md`, documentação Microsoft indicada nesses documentos.

## Regra de progressão entre branches/PRs

Nenhum novo bloco começa antes de integrar a PR anterior em `main`.

Para cada passo:

1. consultar documentação relevante;
2. verificar problemas imediatos da base;
3. corrigir a base antes de adicionar complexidade;
4. escrever testes relevantes;
5. implementar apenas o escopo do passo;
6. atualizar documentação e roadmap;
7. abrir PR própria;
8. validar;
9. fazer merge;
10. fechar a branch e criar a próxima a partir do `main` atualizado.

Toda decisão nova deve atualizar `PRODUCT_DECISIONS.md` e, quando alterar o plano, este roadmap. A documentação histórica pode ser atualizada em conjunto, mas estes documentos são a referência operacional.