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

Quando uma implementação contrariar um destes documentos, atualizar primeiro a decisão/documentação correspondente; não criar divergências silenciosas.

## Estado de referência

`main` contém a base integrada até à 9.4: editor core + migração incremental do renderer. A branch `feat/9-5-editor-ux-campaign` é a tranche de documentação/design que prepara a implementação seguinte.

## Princípio estratégico: construir a base antes de enriquecer o jogo

O objetivo inicial é construir um jogo sólido e um editor de níveis suficientemente confiável para sobreviver nas mãos da comunidade. Tipos de conteúdo mais ricos entram depois sobre esta base.

Antes de multiplicar objetos, priorizar:

- confiabilidade do modelo de nível;
- compatibilidade e versionamento;
- playtest rápido e reversível;
- validação independente;
- diagnóstico explicável;
- análise de dificuldade e progressão;
- layouts previsíveis;
- partilha/importação segura;
- compatibilidade de hardware e runtime;
- profiling antes de otimização.

**Referências:** `docs/RESEARCH_INDEX.md` (mapa geral); `docs/SCIENTIFIC_REFERENCES.md` (level design, dificuldade, playtesting); `docs/TECHNICAL_REFERENCES.md` (arquitetura, Vulkan, QA, packaging, sharing); `docs/DESIGN_REFERENCES.md` (Tiled, Godot, SuperTux, Mario Maker e Jump King).

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

Nunca usar uma única referência como autoridade para uma decisão de design. Mario Maker, Jump King, Tiled, Godot e SuperTux são referências práticas complementares; a física do ASCENDENDO continua própria.

## O que já existe

- Fases 1–8: motor, física, campanha, UI, texto TTF, sprites, replay/save e validação.
- 9.1: `GameAction` + `KeyBindings`.
- 9.2: rato, window→logical e menus clicáveis.
- 9.3: `GameState::EDITOR`.
- `LevelEditorDocument`, `EditorInteractionController`, `EditorSession` e `EditorRenderSnapshot`.
- Nova stack de rendering: `RendererCore`, `ShapeRenderer`, `WorldRenderer`, `UiRenderer`, `EditorRenderer`, `RendererFacade` e `RendererFacadeAdapter`.
- Level Editor baseado numa única tela lógica `640x360`.
- Base separada de `CampaignEditor` e `LevelEditorValidator`.
- Test runner Windows e source-size gate.

**Referências para avaliar e preservar a base existente:** `docs/TECHNICAL_REFERENCES.md` (arquitetura, C++, Vulkan, testes, hardware); `docs/SCIENTIFIC_REFERENCES.md` (física/replay/playtesting); `docs/PRODUCT_DECISIONS.md` (invariantes de produto).

## Fase 9 — Edição e base comunitária

### 9.4 ✅ — Editor core + migração incremental do renderer

Fechada e integrada em `main`.

**Referências:** `docs/DESIGN_REFERENCES.md` (Tiled/Godot/SuperTux); `docs/EDITOR_UX_SPEC.md`; `docs/CAMPAIGN_EDITOR_DESIGN.md`; `docs/TECHNICAL_REFERENCES.md` (separação renderer/modelo e testes).

### 9.5 — Corrigir e completar a base antes de adicionar conteúdo

A 9.5 passa a ter prioridade de **consolidação**, não apenas de novas funcionalidades.

#### 9.5.a — Level Editor UX

- teclas acessíveis (`1/2/3` em vez de F-keys);
- painel de Controlos configurável/consultável;
- rodapé contextual;
- layouts autoajustáveis para texto e UI;
- fullscreen preservado + letterboxing correto;
- canvas `640x360` integralmente visível;
- guardar/playtest/validar explícitos;
- playtest sem persistir alterações automaticamente;
- indicador visual de força do salto;
- feedback de erro imediatamente compreensível.

**Referências e critérios:** `docs/EDITOR_UX_SPEC.md`; `docs/PRODUCT_DECISIONS.md`; `docs/DESIGN_REFERENCES.md` (Godot/Tiled/SuperTux/Jump King/Mario Maker); `docs/SCIENTIFIC_REFERENCES.md` (player experience, challenge, camera e feedback).

#### 9.5.b — Seleção de campanhas

- `Começar` abre seleção mesmo com uma única campanha;
- metadata de campanha separada da lista de níveis quando necessário;
- preview, número de níveis e estado de validade;
- preparação para múltiplas campanhas oficiais.

**Referências e critérios:** `docs/PRODUCT_DECISIONS.md` (seleção explícita); `docs/CAMPAIGN_EDITOR_DESIGN.md` (separação nível/campanha); `docs/SCIENTIFIC_REFERENCES.md` (progressão e experiência em escalas maiores); `docs/DESIGN_REFERENCES.md` (SuperTux/worldmap, Mario Maker/UGC).

#### 9.5.c — Campaign Editor

- timeline vertical;
- miniaturas 16:9 compactadas;
- blocos arrastáveis;
- snap/reordenação discreta;
- abertura do Level Editor via mudança explícita de estado;
- pré-visualização sequencial;
- vários runs de validação em background;
- pelo menos um agente por nível quando possível;
- pelo menos um agente de transição entre níveis;
- diagnóstico visual de falhas.

**Referências e critérios:** `docs/CAMPAIGN_EDITOR_DESIGN.md` (fonte operacional); `docs/DESIGN_REFERENCES.md` (SuperTux, Tiled/Godot, Jump King); `docs/SCIENTIFIC_REFERENCES.md` (progressão multi-escala, level patterns); `docs/PRODUCT_DECISIONS.md` (community-first).

#### 9.5.d — Validador e análise de level design

A validação física continua sendo a autoridade de jogabilidade, mas deve evoluir gradualmente de:

```text
possível / impossível
```

para:

```text
possível
↓
trajetória + ponto de falha
↓
margem de erro
↓
dificuldade estimada do salto
↓
análise da secção
↓
análise do nível
↓
análise da campanha
```

O design deverá considerar, conforme a literatura, padrões como guidance, safe zones, foreshadowing, layering, branching e pace breaking, além de distância/largura de gaps, mudanças de direção e progressão de challenge.

**Referências principais:** `docs/SCIENTIFIC_REFERENCES.md` — Smith/Cha/Whitehead (2008), Khalifa et al. (2019), Pedersen/Togelius/Yannakakis (2009), Aramini/Lanzi/Loiacono (2018), Fernández/Mikami/Kondo (2018), Tanagra e trabalhos de análise de UGC/Mario Maker; `docs/DESIGN_REFERENCES.md` — Mario Maker, Jump King e SuperTux como casos práticos; `docs/RESEARCH_INDEX.md` — Level design, Jump design, Difficulty, Progression, UGC, Authoring tools e Playtesting.

#### 9.5.e — Modelo de nível preparado para conteúdo futuro

Sem ainda enriquecer o catálogo, o modelo deve suportar extensão versionável para:

- superfícies/variantes de plataforma;
- perigos;
- elementos móveis;
- objetos interativos;
- triggers/zonas;
- decoração;
- metadata visual;
- regras/eventos de campanha.

A prioridade é compatibilidade aditiva e migração controlada.

**Referências e critérios:** `docs/TECHNICAL_REFERENCES.md` (modelo declarativo, compatibilidade, data-oriented design); `docs/SCIENTIFIC_REFERENCES.md` (separar autoria, análise e simulação); `docs/DESIGN_REFERENCES.md` (Tiled/SuperTux e evolução de formatos); `docs/PRODUCT_DECISIONS.md` (compatibilidade aditiva).

## Gate 9.6 — Correção da base existente antes de construir por cima

Esta fase é obrigatória e deve vir antes de conteúdo novo significativo.

### Código / arquitetura

1. reduzir responsabilidades de `main.cpp`;
2. fechar definitivamente `Editor → RenderSnapshot → Renderer`;
3. eliminar input de gameplay baseado diretamente em `Key` onde `GameAction` já exista;
4. consolidar `LevelData` entre parser, runtime e editor;
5. separar configuração global por subsistema quando necessário;
6. tornar wrappers Vulkan explicitamente seguros quanto a ownership/move/copy;
7. remover código morto, placeholders e duplicações confirmadas;
8. manter ficheiros de código abaixo do limite de tamanho acordado e subdividir responsabilidades antes de chegarem a ser monólitos.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (C++/data-oriented design, ownership, arquitetura); `docs/RESEARCH_INDEX.md` (C++ architecture); `docs/PRODUCT_DECISIONS.md` (regras de manutenção do projeto).

### Física / gameplay

9. limitar catch-up do fixed timestep e documentar a política de tempo perdido;
10. testar invariantes de física e casos extremos;
11. rever resolução de colisões heurística quando os testes/novas mecânicas mostrarem limitações;
12. distinguir igualdade exata de `float` de comparação aproximada onde a API o exigir.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (physics/timestep/testing); `docs/SCIENTIFIC_REFERENCES.md` (jump metrics, player experience); `docs/RESEARCH_INDEX.md` (Physics, Jump design, Testing).

### Vulkan / rendering

13. validar queue families e present support separadamente;
14. selecionar physical device por capabilities/score e não só pelo tipo de GPU;
15. verificar extensões/features obrigatórias antes de criar o device;
16. revisar synchronization barriers e evitar sincronização excessiva;
17. preparar profiling com separação de CPU, GPU, bandwidth, overdraw e synchronization bubbles;
18. testar pelo menos Intel, NVIDIA, AMD e, quando viável, uma GPU com tile-based rendering;
19. usar feature detection e fallback paths em vez de assumptions por vendor.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (Vulkan synchronization, profiling, tile-based rendering, portability); `docs/RESEARCH_INDEX.md` (Vulkan, Hardware); documentação oficial Khronos/Vulkan indicada nesses documentos. O objetivo é aplicar práticas comprovadas sem sobre-engineering.

### Qualidade

20. Windows build + tests no CI;
21. `make game` no CI;
22. ASan/UBSan no CI quando suportado;
23. replay regression;
24. property/invariant tests para viewport, física, editor e formatos;
25. campanha completa validada automaticamente;
26. matriz mínima de hardware/software documentada.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (game testing, regression, hardware matrix); `docs/SCIENTIFIC_REFERENCES.md` (playtesting, difficulty analysis); `docs/RESEARCH_INDEX.md` (Testing, Playtesting, Hardware).

### Runtime / distribuição

27. paths independentes do current working directory;
28. separar development build, portable release archive e eventual installer;
29. release sem SDK/ambiente de desenvolvimento;
30. shaders/assets/runtime dependencies incluídos de forma determinística;
31. fallback amigável quando uma capability obrigatória estiver ausente.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (Packaging / EXE, portability, runtime paths, deployment); `docs/RESEARCH_INDEX.md` (Packaging, Hardware); documentação Microsoft referenciada nesses documentos.

## Fase 10 — Guardar + formatos estáveis + validação

Só começa após o Gate 9.6 verde.

- versão explícita do `.lvl`;
- `LevelData` declarativo e extensível;
- serialização determinística;
- separação entre dados persistidos e estado de runtime;
- validação em background;
- mapas inválidos nunca entram numa campanha jogável;
- mapas importados/descarregados revalidados pelo EXE;
- migrações de formato documentadas.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (modelo de dados, packaging, sharing e segurança); `docs/DESIGN_REFERENCES.md` (Tiled/SuperTux e evolução de formatos); `docs/PRODUCT_DECISIONS.md` (validação final pelo EXE); `docs/RESEARCH_INDEX.md` (Level design, Sharing, Packaging).

## Fase 11 — Conteúdo oficial e análise de campanhas

Depois da infraestrutura ser confiável, começar a enriquecer o jogo:

- novas superfícies e comportamentos;
- perigos;
- elementos móveis;
- objetos interativos;
- triggers/eventos;
- decoração e identidade visual por zonas;
- ferramentas de composição de campanhas;
- análise de dificuldade e ritmo a várias escalas.

Objetivo de conteúdo oficial:

- várias campanhas;
- campanhas normais com média de aproximadamente **50 níveis**;
- campanhas especiais com **100–200 níveis**;
- níveis finais/opcionais extremamente difíceis;
- progressão baseada em aprendizagem, variação, recuperação e escalada de desafio, não apenas em aumentar números.

**Referências:** `docs/SCIENTIFIC_REFERENCES.md` (level-design patterns, player experience, progression, difficulty at multiple scales); `docs/DESIGN_REFERENCES.md` (Mario Maker, Jump King, SuperTux); `docs/RESEARCH_INDEX.md` (Progression, Difficulty, UGC, Extreme levels); `docs/PRODUCT_DECISIONS.md` (escala e objetivos de conteúdo oficial).

## Fase 12 — Partilha local e web

### 12.1 — Export/import local

Pacote compacto declarativo, extração controlada e validação obrigatória pelo EXE.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (web/sharing security, authority split, archive handling); `docs/PRODUCT_DECISIONS.md` (revalidação obrigatória no EXE); `docs/RESEARCH_INDEX.md` (Sharing).

### 12.2 — Biblioteca online

Site para upload/download de níveis/campanhas. HTTP(S) é suficiente inicialmente; WebSockets não são requisito.

A camada web trata conteúdo como não confiável. O EXE permanece a autoridade final de validade jogável.

Metadata futura pode incluir autor, versão do formato, hashes/IDs, descrição, dificuldade declarada/observada, versão do jogo, estatísticas de uso e reports/moderação.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (OWASP upload security, resumable uploads, UGC metadata); `docs/DESIGN_REFERENCES.md`/`docs/SCIENTIFIC_REFERENCES.md` (Mario Maker e comunidades UGC); `docs/RESEARCH_INDEX.md` (Sharing, Community); documentação OWASP/Microsoft indicada nesses documentos.

### 12.3 — Partilha direta

Só introduzir comunicação bidirecional quando existir uma necessidade real que HTTP(S) + export/import não resolvam.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (sharing architecture); `docs/PRODUCT_DECISIONS.md` (offline-first/HTTP(S) before WebSockets); `docs/RESEARCH_INDEX.md` (Sharing).

## Fase 13 — Release / Portable Build

Objetivo: um pacote Windows x64 que possa ser copiado para outro computador dentro dos requisitos mínimos e executado sem o ambiente de desenvolvimento.

Requisitos: executável + DLLs/assets necessárias, sem dependência do current working directory, sem downloads obrigatórios e com diagnóstico amigável de requisitos ausentes.

**Referências:** `docs/TECHNICAL_REFERENCES.md` (Packaging / EXE, portability, deployment models); `docs/RESEARCH_INDEX.md` (Packaging, Hardware); documentação Microsoft indicada nesses documentos.

## Regra de progressão entre branches/PRs

Nenhum novo bloco de desenvolvimento começa enquanto a PR anterior não estiver integrada em `main`.

Para cada passo:

1. consultar a documentação acima;
2. identificar erros/problemas imediatos;
3. corrigir a base antes de adicionar complexidade;
4. escrever testes relevantes;
5. implementar apenas o escopo do passo;
6. atualizar documentação e roadmap;
7. abrir PR própria;
8. validar;
9. fazer merge;
10. fechar a branch e criar a próxima a partir do `main` atualizado.

Toda decisão nova deve atualizar `PRODUCT_DECISIONS.md` e, quando alterar o plano, este roadmap. A documentação histórica pode ser atualizada em conjunto, mas estes documentos são a referência operacional.
