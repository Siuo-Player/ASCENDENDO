# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável. Severidade:

- **P0** — impede evoluir com segurança ou pode causar comportamento incorreto em runtime/release.
- **P1** — deve ser tratado antes das fases seguintes que dependem dele.
- **P2** — melhoria importante de manutenção/performance.
- **P3** — limpeza posterior.

## Estado atual da migração do renderer

A implementação legada `Renderer.cpp/.h` já não está presente em `main`. A migração para `RendererCore` + passes + `RendererFacade` está integrada.

A próxima dívida arquitetural de presentation é a ausência de um `RenderSnapshot` geral: em `main`, `WorldRenderer` ainda recebe diretamente `Player` e `Level`. PR #20 foi encerrado/superseded; a migração continua uma responsabilidade do roadmap, mas não existe uma branch histórica ativa a manter.

## P0 — tratar antes de continuar a acumular complexidade

| Área | Problema | Ação | Critério de saída |
|---|---|---|---|
| CI / evidência | Run #281 falhou no step agregado de build/teste, mas a causa detalhada não está confirmada | obter diagnóstico observável antes de atribuir causalidade e, depois, corrigir/revalidar | causa classificada por evidência e nova execução documentada |
| Runtime | `main.cpp` acumula inicialização, bootstrap, frame coordination e apresentação | continuar extraindo responsabilidades incrementais por ownership real | `main.cpp` deixa de concentrar regras de runtime que já têm fronteira própria |
| Presentation | `RendererFacade`/passes ainda recebem modelos de domínio diretamente | introduzir `RenderSnapshot`/dados de apresentação | presentation recebe dados próprios de apresentação |
| Input | gameplay ainda pode consultar teclas físicas diretamente | migrar `Player` para `GameAction`/`KeyBindings` | nenhuma regra de gameplay depende de `Key::...` |
| Paths | runtime usa paths relativos ao current working directory | criar resolução de `executable root`, `asset root` e `user data root` | executar o EXE a partir de qualquer diretório suportado |
| Levels | runtime/editor têm modelos separados que representam o mesmo conteúdo | introduzir `LevelData` independente de Vulkan/GLFW | parser, editor e runtime convergem no mesmo modelo |
| Physics | fixed timestep sem limite de recuperação | limitar passos por frame e/ou `dt` | minimização/lag não provoca centenas de ticks num frame |
| Vulkan | seleção de queue/device assume demasiado sobre graphics/present | validar extension, capabilities e graphics/present queues separadamente | hardware com queues diferentes é suportado ou rejeitado explicitamente |

## P1 — tratar durante a consolidação pós-9.4

| Área | Problema | Ação | Estado atual |
|---|---|---|---|
| CI | só Linux é referência de build | adicionar Windows build/tests | aberto |
| CI | sanitizers existem no Makefile mas não eram um job obrigatório | job ASan + UBSan | **Linux concluído no PR #70** |
| CI | build/game/testes ainda estão agregados em parte do workflow | separar steps para observabilidade e diagnosticar cada fase | parcialmente mitigado; sanitizer já é workflow independente |
| Editor | não existe undo/redo | Command Pattern + stacks | aberto |
| Editor | drag deve ser uma operação lógica única | criar transações/comandos begin/update/end | aberto |
| Config | `Config.h` acumula domínios | separar progressivamente physics/render/window/editor/gameplay | aberto |
| Vulkan RAII | wrappers devem garantir ownership explícito | tornar recursos não-copiáveis e movíveis quando apropriado | aberto |
| Levels | formato textual não tem versionamento explícito | introduzir `VERSION` no formato | aberto |
| Campaign | `campaign.txt` mistura lista/ordem com futura metadata | definir `CampaignData` quando metadata for necessária | aberto |
| Tests | validator via `system()` pertence a integração/sistema, não unit | mover/categorizar teste | aberto |
| Gestão | work packages podem existir sem dependências/critério de saída explícitos | aplicar `docs/PROJECT_MANAGEMENT.md` e `docs/DEVELOPMENT_PROTOCOL.md` a cada bloco | processo ativo |
| Arquitetura | fronteiras podem ser alteradas sem atualizar WBS/roadmap | tratar mudanças arquiteturais como alteração de planeamento | processo ativo |
| Coordenação | consumidores e testes podem descobrir uma mudança de interface apenas no fim da branch | manter dependency map no work package | processo ativo |

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

## Source-size work packages em preparação

Os alvos de source-size continuam rastreados como sinais de revisão estrutural:

```text
FontRenderer.cpp
    → 430 linhas históricas → decomposto

SpriteRenderer.cpp
    → 332 linhas históricas → investigar coesão antes de dividir

Tests/Unit/test_keybindings.cpp
    → 305 linhas históricas → decomposto por responsabilidade

Tests/Unit/test_level.cpp
    → 326 linhas históricas → decomposto por responsabilidade

main.cpp
    → ~330 linhas históricas → GameSession extraído; continuar bootstrap/frame composition sem split artificial
```

A política normativa atual está em `docs/CODE_SIZE.md`: `< 40 KiB` normal, `40–47.99 KiB` warning e `>= 48 KiB` error. LOC é diagnóstico apenas.

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
10. A política normativa de modularidade é `<40 KiB` normal, `40–47.99 KiB` warning e `>=48 KiB` error; LOC é diagnóstico.
11. Cada work package deve ter dependências e critérios de saída explícitos.
12. Alterações que mudem fronteiras arquiteturais devem atualizar o planeamento e a documentação relevante.
13. Uma dependência técnica deve ser considerada também uma dependência de coordenação quando a sua alteração afeta consumidores, testes ou documentação.
14. `RendererFacadeAdapter` não é uma API suportada do runtime.
15. Causas de falhas CI não são tratadas como confirmadas sem evidência observável.
16. `GameSession` é a fronteira atual para estado mutável de gameplay/editor/campanha; não deve absorver ownership de platform, Vulkan, camera ou presentation sem nova decisão documentada.

## Portões do roadmap

Antes de avançar para uma nova tranche arquitetural dependente de presentation, os gates de processo acima devem estar resolvidos ou explicitamente aceites como dívida com risco, condição de revisão e posição no roadmap.

Antes de **11 Partilha/Biblioteca** devem estar resolvidos:

- `LevelData` + formato versionado;
- paths/assets/user data;
- validação robusta de input malformado;
- Windows CI;
- autoridade local do EXE;
- import/export declarativo e seguro.

Antes da **release portable** todos os P0 devem estar fechados e os P1 críticos devem ter critério de saída documentado.

## Governança do roadmap

A lista acima deve ser lida em conjunto com `docs/PROJECT_MANAGEMENT.md` e `docs/DEVELOPMENT_PROTOCOL.md`. Um item de dívida não é apenas uma observação técnica: quando exige trabalho, deve tornar-se um work package rastreável no roadmap ou numa tranche de manutenção.

## Evidência recente — sanitizer

PR #70 adicionou o workflow `.github/workflows/sanitizers.yml`. No commit de implementação `c450f62cd5f3bac4f37e768a34ec17bbcb4a08cd`, o workflow normal `Tests` e o workflow `Sanitizers` terminaram com sucesso. O workflow sanitizer força `CXXFLAGS_REL` a uma configuração ASan/UBSan, fazendo com que a biblioteca `Game` ligada aos testes seja instrumentada, sem alterar as flags normais do build release.

Windows CI permanece uma dívida independente; não se deve inferir sucesso ou falha sem uma estratégia de dependências validada.

## Evidência recente — GameSession

PR #72 extraiu a primeira fronteira de sessão de `main.cpp`. `GameSession` agora possui `GameStateMachine`, `CampaignRuntime`, `Level`, `PhysicsWorld`, `SimulationOrchestrator`, `Player` e `EditorSession`; `main.cpp` permanece responsável pela composição do processo, graphics/presentation, input polling, `Camera` e submissão de frames.

A implementação foi validada por `Tests #746` (`33026510334`) e `Sanitizers #8` (`33026512992`), ambos concluídos com sucesso incluindo build, testes, headless Vulkan e campaign validation. Durante a branch houve uma falha de compilação observável (`33026463141`) causada pela remoção acidental do include explícito de `Logic/RunHistory.h`; a correção foi feita na mesma branch e os checks seguintes ficaram verdes.
