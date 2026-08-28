# Work Package — RenderSnapshot domain/presentation boundary

## Identificação

**Roadmap:** `Next Architecture Block after Gate 9.6`  
**Work Package:** `RenderSnapshot domain/presentation boundary`  
**Issue:** `#122`  
**Branch:** `docs/render-snapshot-wp-20260828`

## Estado

`IN INVESTIGATION`

## Contexto

Gate 9.6 foi formalmente fechado. A documentação canónica identifica a fronteira `RenderSnapshot` como o próximo bloco arquitetural. Os Studies confirmam que esta migração deve começar apenas depois do fecho formal do Gate e sem transformar arquitetura futura em requisito retroativo.

## Evidência atual

A implementação atual ainda atravessa a fronteira de presentation com tipos de domínio:

- `RendererFacade::drawFrame()` recebe `logic::Player` e `logic::Level`;
- `WorldRenderer::draw()` recebe diretamente `logic::Player`, `logic::Level` e `Camera`;
- `WorldRenderer.cpp` consulta `Level::platforms()`, `Level::hasFlag`, `Level::flagBounds`, `Player::position()` e `Player::facingDirection`;
- já existe `EditorRenderSnapshot`, demonstrando um contrato de dados independente de Vulkan para um caminho específico.

## Decisão metodológica

O snapshot geral deve ser um **value object de presentation**:

- sem `logic::Player`, `logic::Level`, `logic::Vec2` ou `Camera`;
- sem Vulkan handles/resources;
- sem ownership;
- sem ponteiros para estado mutável do runtime;
- sem regras de gameplay;
- apenas os dados realmente necessários pelos passes migrados.

A construção do snapshot pertence ao lado de runtime/composição e deve ser explícita, determinística e testável.

## Primeira fronteira

A primeira tranche migra somente o world/player path:

```text
logic::Player + logic::Level + camera state
                ↓
          snapshot builder
                ↓
          RenderSnapshot
                ↓
          WorldRenderer
```

O snapshot inicial contém apenas:

```text
camera { x, y }
player { x, y, width, height, facingLeft }
platforms[] { x, y, width, height }
flag { visible, x, y, width, height }
```

Não incluir velocidade, grounded, jump charge, nome de nível ou outras propriedades até existir consumidor de presentation que as justifique.

## Escopo

1. definir o contrato de dados;
2. implementar construção do snapshot a partir dos estados atuais;
3. migrar `WorldRenderer` para consumir o snapshot;
4. adaptar `RendererFacade` para passar snapshot em vez de `Player`/`Level` no world path;
5. preservar a API e semântica dos sistemas de gameplay fora desta fronteira;
6. adicionar testes de contrato/construção;
7. validar build, testes e CI;
8. atualizar `ARCHITECTURE.md`, `TECH_DEBT.md`, `ROADMAP.md` e este WP conforme a implementação real.

## Fora de escopo

- `Application` genérica;
- migração de todos os passes numa só mudança;
- `UiRenderer`, menus e editor, salvo alterações mínimas necessárias para manter o build;
- replay/persistence;
- Level schema/versioning;
- alterações de physics/gameplay;
- otimizações sem baseline medida.

## Critérios de saída

```text
snapshot independente de tipos de domínio
+ sem ownership Vulkan
+ construção determinística
+ WorldRenderer sem Player/Level
+ comportamento visual/funcional preservado
+ testes/build/CI passam
+ documentação corresponde ao código
```

## Riscos

| Risco | Mitigação |
|---|---|
| snapshot demasiado rico | adicionar apenas dados consumidos por presentation |
| duplicação de estado | construir um snapshot transitório por frame e não criar segundo modelo de gameplay |
| builder absorve regras de jogo | apenas copiar/transformar estado para representação de presentation |
| migração demasiado grande | limitar a primeira tranche ao world/player path |

## Validação

A baseline deve ser comparada antes/depois nos mesmos testes e workflows. Nenhuma alteração de rendering deve ser aceite apenas porque compila; a equivalência funcional do caminho migrado deve ser preservada.

## Dependências

- `Game/Logic/Player.h`
- `Game/Logic/Level.h`
- `Game/Graphics/Camera.h`
- `Game/Graphics/WorldRenderer.*`
- `Game/Graphics/RendererFacade.*`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/ROADMAP.md`
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_LATEST.md`
- `PROJECT-STUDIES/ASCENDENDO/CURRENT_STATE_2026-08-28_FINAL-RECONCILIATION.md`

## Próximo bloco

Depois desta fronteira estar estável, avaliar separadamente:

1. remaining presentation paths;
2. FontRenderer/SpriteRenderer responsibility work;
3. larger `main.cpp` composition responsibilities.

Nenhum desses pontos é implicitamente incluído nesta primeira migração.
