# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável. Severidade:

- **P0** — impede evoluir com segurança ou pode causar comportamento incorreto em runtime/release.
- **P1** — deve ser tratado antes das fases seguintes que dependem dele.
- **P2** — melhoria importante de manutenção/performance.
- **P3** — limpeza posterior.

## P0 — tratar antes da release e, quando indicado, antes de save/import

| Área | Problema | Ação | Critério de saída |
|---|---|---|---|
| Runtime | `main.cpp` acumula inicialização, estados, campanha, física, editor e persistência | extrair `Application`, `GameStateMachine` e `Simulation` incrementalmente | `main.cpp` deixa de possuir regras de gameplay/editor |
| Presentation | `Renderer.cpp` conhece `Player`, `Level`, `GameState`, menu e editor | introduzir `RenderSnapshot`/`EditorRenderData` | Renderer só consome dados de apresentação |
| Input | gameplay ainda pode consultar teclas físicas diretamente | migrar `Player` para `GameAction`/`KeyBindings` | nenhuma regra de gameplay depende de `Key::...` |
| Paths | runtime usa paths relativos ao current working directory | criar resolução de `executable root`, `asset root` e `user data root` | executar o EXE a partir de qualquer diretório suportado |
| Levels | runtime/editor têm modelos separados que representam o mesmo conteúdo | introduzir `LevelData` independente de Vulkan/GLFW | parser, editor e runtime convergem no mesmo modelo |
| Physics | fixed timestep sem limite de recuperação | limitar passos por frame e/ou `dt` | minimização/lag não provoca centenas de ticks num frame |
| Vulkan | seleção de queue/device assume demasiado sobre graphics/present | validar extension, capabilities e graphics/present queues separadamente | hardware com queues diferentes é suportado ou rejeitado explicitamente |

## P1 — tratar durante a consolidação pós-9.4

| Área | Problema | Ação |
|---|---|---|
| CI | só Linux é referência de build | adicionar Windows build/tests |
| CI | sanitizers existem no Makefile mas não são um job obrigatório | job ASan + UBSan |
| CI | `make game` não é necessariamente exercitado pelo pipeline de testes | validar build/link do jogo |
| Editor | não existe undo/redo | Command Pattern + stacks |
| Editor | drag deve ser uma operação lógica única | criar transações/comandos begin/update/end |
| Config | `Config.h` acumula domínios | separar progressivamente physics/render/window/editor/gameplay |
| Vulkan RAII | wrappers devem garantir ownership explícito | tornar recursos não-copiáveis e movíveis quando apropriado |
| Levels | formato textual não tem versionamento explícito | introduzir `VERSION` no formato |
| Campaign | `campaign.txt` mistura lista/ordem com futura metadata | definir `CampaignData` quando metadata for necessária |
| Tests | validator via `system()` pertence a integração/sistema, não unit | mover/categorizar teste |
| Source size | ficheiros de código demasiado grandes dificultam revisão e alterações seguras | aviso aos 30 KiB; subdividir por responsabilidade antes de 36 KiB | nenhum ficheiro C/C++ > 36 KiB; componentes centrais devem evitar a zona de aviso |
| Gestão | work packages podem existir sem dependências/critério de saída explícitos | aplicar `docs/PROJECT_MANAGEMENT.md` a cada bloco do roadmap | cada branch nova tem Ready/Done, dependências e validação definidos |
| Arquitetura | fronteiras podem ser alteradas sem atualizar WBS/roadmap | tratar mudanças arquiteturais como alteração de planeamento | arquitetura e WBS permanecem congruentes |
| Coordenação | consumidores e testes podem descobrir uma mudança de interface apenas no fim da branch | manter dependency map no work package | dependências críticas identificadas antes de implementação/merge |

## P2 — qualidade, cobertura e performance

| Área | Problema | Ação |
|---|---|---|
| Physics | `Vec2::operator==` usa igualdade exata de `float` | manter equality explícita e adicionar `almostEqual` |
| Collision | resolução de colisão está concentrada em `Level` e é heurística | separar geometria de resposta física quando a complexidade exigir |
| Collision | sem continuous collision detection | só introduzir se velocidades/entidades justificarem; manter limitação documentada |
| Rendering | fallback bitmap faz muitos draw calls/push constants por glyph | manter como fallback; preferir atlas/instancing |
| Assets | paths de sprites/fonts/shaders estão espalhados | criar `AssetManager`/AssetResolver quando a quantidade justificar |
| Tests | foco atual em unit tests, poucas propriedades | property-based tests para snap, viewport, física, parser e editor |
| Tests | pastas `Integration/System/Acceptance` e placeholders não representam cobertura real | criar categorias quando existirem testes; eliminar placeholders vazios |
| Tooling | `deps.py` é checker, não dependency manager | renomear para `check_deps.py`/`doctor.py` ou clarificar documentação |
| Build | Makefile já implementa muita lógica de plataforma | avaliar CMake + Ninja quando a matriz Windows/Linux crescer |
| Planeamento | evolução emergente pode aumentar retrabalho entre subsistemas | decompor por WBS e gates, refinando detalhes conforme a incerteza | work packages atravessam fronteiras com dependências conhecidas |
| Arquitetura | documentação pode registar estrutura sem preservar rationale | registar decisões arquiteturais relevantes, alternativas e consequências | decisões que condicionam várias fases têm contexto e critério de revisão |

## P3 — limpeza

- remover `.gitkeep` de diretórios que já tenham conteúdo;
- remover `test_placeholder.cpp` quando a infraestrutura tiver cobertura suficiente;
- retirar históricos de versão extensos dos `.cpp` e deixar no changelog/Git history;
- reduzir comentários que narram bugs antigos e manter comentários que expliquem invariantes atuais;
- separar documentação normativa de diário de desenvolvimento;
- evitar usar `Development/Runs/runs.csv` como parte do source tree de runtime.

## Regras de arquitetura derivadas da revisão

1. O renderer nunca lê input nem altera lógica.
2. Gameplay não conhece teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. O documento do editor continua testável sem GPU.
5. Um drag completo é uma operação lógica única para undo/redo.
6. Dados importáveis continuam declarativos e sem execução de código.
7. O EXE é a autoridade final de validação de mapas.
8. O current working directory não é uma dependência do runtime.
9. O CI testa o produto que será distribuído, não apenas os testes unitários.
10. Ficheiros C/C++ devem permanecer abaixo de 36 KiB; a partir de 30 KiB não devem receber novas responsabilidades sem um plano de subdivisão.
11. Cada work package deve ter dependências e critérios de saída explícitos.
12. Alterações que mudem fronteiras arquiteturais devem atualizar o planeamento e a documentação relevante.
13. Uma dependência técnica deve ser considerada também uma dependência de coordenação quando a sua alteração afeta consumidores, testes ou documentação.

## Portões do roadmap

Antes de **9.5 Save + Validar** devem estar resolvidos os pontos P0 que afetam o editor/níveis e pelo menos o fixed timestep/path model.

Antes de **11 Partilha/Biblioteca** devem estar resolvidos:

- `LevelData` + formato versionado;
- paths/assets/user data;
- validação robusta de input malformado;
- Windows CI;
- autoridade local do EXE;
- import/export declarativo e seguro.

Antes da **release portable** todos os P0 devem estar fechados e os P1 críticos devem ter critério de saída documentado.

## Governança do roadmap

A lista acima deve ser lida em conjunto com `docs/PROJECT_MANAGEMENT.md`. Um item de dívida não é apenas uma observação técnica: quando exige trabalho, deve tornar-se um work package rastreável no roadmap ou numa tranche de manutenção.
