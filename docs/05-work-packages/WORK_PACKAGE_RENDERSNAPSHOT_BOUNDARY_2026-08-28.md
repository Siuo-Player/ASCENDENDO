# Work Package — RenderSnapshot domain/presentation boundary

## Identificação

**Roadmap:** `Next Architecture Block after Gate 9.6`  
**Work Package:** `RenderSnapshot domain/presentation boundary`  
**Issue:** `#122`  
**Implementation branch:** `feat/render-snapshot-world-path-20260828`

## Estado

`DONE — first world/player presentation boundary`

## Contexto

Gate 9.6 foi formalmente fechado. A documentação canónica identificou a fronteira `RenderSnapshot` como o bloco arquitetural seguinte. Os Studies confirmaram que a migração deveria começar apenas depois do fecho do Gate e sem transformar arquitetura futura em requisito retroativo.

## Evidência inicial

Antes desta tranche, a presentation recebia diretamente `logic::Player` e `logic::Level` no caminho de rendering. `WorldRenderer` consultava plataformas, flag e estado do jogador através desses tipos. Já existia `Game/Graphics/RenderSnapshot.h`, um contrato de dados sem tipos de domínio.

## Decisão metodológica

Reutilizar o `Game/Graphics/RenderSnapshot.h` existente em vez de introduzir uma segunda representação. O snapshot permanece um **value object de presentation**:

- sem `logic::Player`, `logic::Level` ou `logic::Vec2`;
- sem `Camera` incorporada;
- sem Vulkan handles/resources;
- sem ownership;
- sem ponteiros para estado mutável do runtime;
- sem regras de gameplay;
- apenas os dados realmente necessários pelo world/player pass.

A `Camera` continua uma dependência de presentation separada. Isto evita contaminar o snapshot com estado de câmara e mantém a transformação world→NDC no subsistema de presentation.

## Contrato efetivamente utilizado

```text
RenderSnapshot
├── player { bounds, facingDirection }
├── platforms[] { x, y, width, height }
└── flag { visible, x, y, width, height }
```

Não foram adicionados velocidade, grounded, jump charge, nome de nível ou outras propriedades de gameplay.

## Implementação

A fronteira de composição é agora:

```text
logic::Player + logic::Level
            ↓
RenderSnapshotBuilder
            ↓
     gfx::RenderSnapshot
            ↓
RendererFacade / WorldRenderer
```

`RenderSnapshotBuilder` apenas copia/transforma estado existente. O `WorldRenderer` não inclui `Player.h` nem `Level.h` e não consulta o modelo mutável de gameplay.

`main.cpp` cria o snapshot apenas quando o estado é `PLAYING` ou `PAUSED`. Em MENU/CREDITS/EDITOR não existe cópia da geometria do mundo, porque esses caminhos não consomem `WorldRenderer`. Esta redução segue o princípio de evitar trabalho que não produz valor para o frame atual.

## Alterações

- `Game/Graphics/RenderSnapshotBuilder.h/.cpp` — adapter explícito runtime→presentation.
- `Game/Graphics/RendererFacade.h/.cpp` — world path passa a receber `RenderSnapshot`.
- `Game/Graphics/WorldRenderer.h/.cpp` — consome apenas snapshot + recursos de rendering + `Camera`.
- `main.cpp` — construção do snapshot na composição do frame.
- `Tests/Unit/test_render_snapshot.cpp` — caracterização do contrato e da cópia desacoplada.

## Fora de escopo

- `Application` genérica;
- migração de todos os passes numa só mudança;
- `UiRenderer`, menus e editor, salvo o necessário para o build;
- replay/persistence;
- Level schema/versioning;
- alterações de physics/gameplay;
- otimização de baixo nível sem baseline.

## Validação

O contrato é coberto por testes que verificam:

1. composição do snapshot a partir de `Player`/`Level`;
2. geometria de plataformas e flag;
3. direção visual do jogador;
4. independência do snapshot perante alterações posteriores no runtime;
5. propriedades básicas de value object/cópia.

O PR #129 passou os três workflows obrigatórios e foi integrado na `main`.

## Critérios de saída

```text
snapshot independente de tipos de domínio
+ sem ownership Vulkan
+ construção determinística
+ WorldRenderer sem Player/Level
+ comportamento preservado
+ testes/build/CI passam
+ documentação corresponde ao código
```

Todos os critérios da primeira tranche foram satisfeitos.

## Riscos observados

| Risco | Resultado |
|---|---|
| snapshot demasiado rico | evitado; só campos consumidos pelo world/player pass |
| duplicação de estado | evitada; reutilizado o único contrato `gfx::RenderSnapshot` |
| builder absorver regras de jogo | evitado; adapter apenas copia/transforma estado existente |
| trabalho desnecessário fora do world path | evitado; construção limitada a PLAYING/PAUSED |
| migração demasiado grande | evitada; primeira tranche limitada ao world/player path |

## Próxima decisão

Após a integração, os restantes presentation consumers devem ser avaliados individualmente. `UiRenderer`, editor e outros paths só devem receber snapshots próprios quando houver um benefício arquitetural/testável demonstrado. Não uniformizar interfaces apenas por estética.
