# Roadmap de desenvolvimento

## Regra de leitura — antes de cada branch

O roadmap orienta a ordem do trabalho, mas uma propriedade só muda de estado quando existe evidência suficiente para a alegação correspondente.

## Gate 9.6 — Base Engineering Gate

O Gate está **CLOSED** a partir da integração da revisão final do Gate (PR #118) e da confirmação de fecho formal (PR #119).

## RenderSnapshot

**Issue:** #122  
**WP:** `docs/05-work-packages/WORK_PACKAGE_RENDERSNAPSHOT_BOUNDARY_2026-08-28.md`  
**Estado:** **FIRST TRANCHE CONCLUÍDA (PR #129) + editor boundary concluída (PR #132)**

A primeira tranche separou o world/player path do domínio através de `gfx::RenderSnapshot` e `RenderSnapshotBuilder`. O editor passou a materializar `EditorRenderSnapshot` na composição e `RendererFacade` deixou de depender diretamente de `EditorSession`.

## Shared Vulkan image upload

**Issue:** #23  
**Implementation:** PR #133  
**Merge:** `e3871bc935dfa52124ec5244ddbb04714caec161`  
**Estado:** **COMPLETED**

A duplicação de lifecycle Vulkan entre `FontRendererGpu` e `SpriteRendererGpu` foi centralizada em `Game/Graphics/VulkanImageUpload.h/.cpp`, preservando `VkFormat`, `VkFilter`, descriptor policy e ownership nos consumidores.

## EditorInteraction layer boundary

**Issue:** #135  
**WP:** `docs/05-work-packages/WORK_PACKAGE_EDITOR_INTERACTION_LAYER_BOUNDARY_2026-08-28.md`  
**Implementation:** PR #136  
**Merge:** `4d587af1f8d4633e47d4c86b51fb503493f27550`  
**Estado:** **COMPLETED**

### Descoberta

`Game/Logic/EditorInteraction.h` incluía `Graphics/Camera.h` e expunha `cursorFromLogical(..., const gfx::Camera&)`. A implementação usava somente `camera.position`, criando uma dependência de presentation desnecessária dentro da lógica do editor. Durante a validação também foi observada a mesma dependência concreta em `EditorSession.cpp`; no modelo atual de editor de tela única, a posição necessária é `{0,0}`.

### Decisão

Substituir `const gfx::Camera&` por `const Vec2& cameraPosition` e remover o uso de `gfx::Camera` de `EditorSession`. A `Camera` permanece propriedade da presentation; a lógica recebe somente os dados mínimos necessários.

```text
logical cursor + camera position
→ world cursor
```

No editor de tela única, `EditorSession` passa `Vec2{0.0f, 0.0f}`.

### Evidência

- `cursorFromLogical()` preserva exatamente a soma `logical + cameraPosition`;
- characterization test foi migrado para `Vec2`;
- Linux / Clang / C++20 / Headless Vulkan: **success**;
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**;
- Windows / Clang / C++20: **success**;
- source-size e campaign validation: **success**;
- nenhum consumidor do contrato antigo permanece no build validado.

Durante o primeiro ciclo de CI foi encontrado e corrigido o include concreto ausente de `Graphics/Camera.h` em `EditorRenderer.cpp`; a correção necessária ficou no mesmo ciclo de implementação antes do merge.

### Fora de escopo

- mudança de `Camera`;
- mudança do sistema de coordenadas;
- mudança da interação do editor;
- nova abstração genérica de transformação.

## GameState ownership boundary

**Issue:** #137  
**WP:** `docs/05-work-packages/WORK_PACKAGE_CORE_GAMESTATE_BOUNDARY_2026-08-28.md`  
**Implementation:** PR #137  
**Merge:** `b9f0d0021bef341327bfde1cdd02d2be8171e0ba`  
**Estado:** **COMPLETED**

### Descoberta

`Game/Core/GameStateMachine.h` dependia de `Graphics/GameState.h`, embora `GameState` seja apenas um enum de estado de runtime. Isso colocava um contrato de Core sob ownership de Graphics sem necessidade.

### Decisão

A definição canónica passou para `Game/Core/GameState.h`. `Game/Graphics/GameState.h` ficou como alias explícito de compatibilidade (`gfx::GameState = core::GameState`), enquanto `GameStateMachine`, `GameSession`, `main.cpp` e os testes passaram a usar diretamente `core::GameState`.

```text
Core state contract
        ↓
GameSession / state machine
        ↓
Presentation
```

Os estados e as transições não foram alterados.

### Evidência

- Linux / Clang / C++20 / Headless Vulkan: **success**;
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**;
- Windows / Clang / C++20: **success**;
- source-size e campaign validation: **success**;
- `static_assert` confirma a identidade de tipo entre `gfx::GameState` e `core::GameState`;
- issue #137 fechado como completed após integração.

## Semantic TickInput boundary

**Issue:** #138  
**WP:** `docs/05-work-packages/WORK_PACKAGE_TICK_INPUT_BOUNDARY_2026-08-28.md`  
**Implementation:** `refactor/semantic-tick-input-boundary-20260828`  
**Merge:** `7da5af74e2ccc9c2a33d43cbfbcfacc6f5c04381`  
**Estado:** **COMPLETED**

### Descoberta

`Game/Logic/Player.h` incluía `Game/Logic/InputManager.h` apenas para conhecer `TickInput`. `Player` não utilizava a API de input físico; recebia somente cinco campos semânticos necessários para um tick de simulação.

### Decisão

`TickInput` foi extraído para `Game/Logic/TickInput.h`. `InputManager` continua a traduzir hardware/bindings para o value object e `Player` depende apenas desse contrato semântico.

```text
InputManager
    ↓ produz
TickInput
    ↓ consome
Player
```

O tipo permanece em `Logic`, porque a evidência não justifica transformá-lo num contrato transversal de `Core`.

### Evidência

- `Player.cpp` continua a consumir apenas os cinco campos do contrato;
- `test_player.cpp` constrói `TickInput` diretamente;
- `Player.h` deixou de incluir `InputManager.h`;
- Linux / Clang / C++20 / Headless Vulkan: **success**;
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**;
- Windows / Clang / C++20: **success**;
- source-size e campaign validation: **success**;
- issue #138 fechado como completed após integração.

## Presentation configuration boundary

**Issue:** #140  
**WP:** `docs/05-work-packages/WORK_PACKAGE_PRESENTATION_CONFIG_BOUNDARY_2026-08-28.md`  
**Implementation:** `refactor/presentation-config-boundary-20260828`  
**Estado:** **IN IMPLEMENTATION**

### Descoberta

`Game/Core/Config.h` mistura constantes de Core/gameplay com configuração puramente visual. `WorldRenderer`, `EditorRenderer` e `RendererFacade` dependem desse header para cores, clear colors e espaçamento visual.

`CAMERA_SPEED` e `CAMERA_OFFSET_Y` também não têm consumidores efetivos na implementação atual.

### Decisão

Criar `Game/Graphics/PresentationConfig.h` para os valores comprovadamente de presentation e manter em `Core/Config.h` apenas dimensões lógicas, aspect ratio, timestep, física, gameplay e `EDITOR_GRID_SNAP`.

```text
Core/Config
  → logical/gameplay semantics

Graphics/PresentationConfig
  → visual presentation policy
```

### Evidência inicial

- `WorldRenderer.cpp` usa apenas cores de plataforma/jogador/bandeira a partir de `Core/Config`;
- `EditorRenderer.cpp` usa cores e espaçamento visual da grelha, mantendo dimensões lógicas em Core;
- `RendererFacade.cpp` usa clear colors e continua a usar `TARGET_ASPECT` de Core;
- a caracterização independente de `PresentationConfig.h` preserva valores representativos.

## Próximo alvo — revisão final de ownership/arquitetura

Após o #140, voltar à auditoria global e não criar novas divisões de configuração ou abstrações sem coupling concreto e contrato mínimo verificável.

A inspeção de bootstrap observou duplicação de parsing entre `CampaignLoader` e `CampaignID`, mas ambos atualmente interpretam a mesma manifestação e ordem; isto permanece uma dívida potencial, não uma alteração automática.

Não reabrir o Gate 9.6 por propriedades futuras já adiadas, como replay persistence, terminal/result replay ou live-input frame-rate independence, sem novo requisito ou evidência.

## Fase 10

Semantic `LevelData` validation/schema/versioning permanece separado do bloco de presentation e só avança quando a evidência/requisito correspondente o justificar.

## Princípios de execução

```text
investigar
→ documentar
→ atualizar roadmap/architecture/tech-debt/WP
→ implementar
→ testar/validar
→ documentar resultado, falhas e próxima decisão
```

A decomposição de `main.cpp` é incremental e baseada em ownership/responsabilidade/testabilidade; não criar uma `Application` genérica apenas para reduzir linhas.
