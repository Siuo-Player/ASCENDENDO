# Dívida técnica e revisão arquitetural

Este documento transforma a revisão de código atual em trabalho rastreável.

## Gate 9.6 — estado

**CLOSED**

O fecho formal segue a integração da revisão final do Gate (PR #118) e a confirmação formal (PR #119); não existe blocker técnico restante dentro do escopo 9.6.

### CI finding — PR #136

PR #136 revelou uma falha de compilação independente da sua alteração de Logic/editor:

```text
Run: 33197769287 / job 98939183984
Run: 33197769230 / job 98939183167
Run: 33197769316 / job 98939183648
Commit: 6b4a4a23eefaee81f0a9feccf886d9f83181c04f
Failing step: game build / instrumented test build
Observed evidence: Game/Graphics/EditorRenderer.cpp:39 instancia `const Camera fixedCamera{}` enquanto apenas `ShapeRenderer.h` fornece a forward declaration `class Camera;`.
Classification: CI build failure
Confirmed cause: missing direct include of `Graphics/Camera.h` in `EditorRenderer.cpp`.
Scope relation: `EditorRenderer.cpp` is outside the intended #135/#136 change.
Resolution: direct include added in PR #136 branch before merge.
Validation: Linux normal, Linux ASan/UBSan and Windows all passed on the validated PR head.
```

## EditorInteraction layer boundary — concluído

PR #136 removeu a dependência concreta `Game/Logic/EditorInteraction → Game/Graphics/Camera`.

O contrato passou a receber apenas `Vec2 cameraPosition`, preservando a transformação `logical + cameraPosition`. `EditorSession` também deixou de instanciar `gfx::Camera` para o editor de tela única e passa `{0,0}`. A mudança foi validada pelos três workflows obrigatórios.

## GameState ownership — concluído

O Issue #137 encontrou e resolveu o coupling:

```text
Game/Core/GameStateMachine.h
        ↓
Game/Graphics/GameState.h
```

`GameState` contém apenas estados de runtime e não dados de rendering. A definição canónica foi movida para `Game/Core/GameState.h`; `Game/Graphics/GameState.h` ficou como alias explícito de compatibilidade (`gfx::GameState = core::GameState`). `GameStateMachine`, `GameSession`, `main.cpp` e os testes internos passaram a usar diretamente `core::GameState`.

**Issue:** #137  
**PR:** #137  
**Merge:** `b9f0d0021bef341327bfde1cdd02d2be8171e0ba`  
**Estado:** **DONE**

### Validação

- Linux / Clang / C++20 / Headless Vulkan: success;
- Linux / Clang / ASan + UBSan / Headless Vulkan: success;
- Windows / Clang / C++20: success;
- source-size: success;
- campaign validation: success;
- `static_assert` confirmou identidade de tipo entre `gfx::GameState` e `core::GameState`.

## Semantic TickInput boundary — concluído

A auditoria pós-#137 encontrou um coupling menor mas concreto: `Game/Logic/Player.h` incluía `Game/Logic/InputManager.h` apenas para obter `TickInput`. `Player` não usa callbacks, key codes, mouse state, `GLFWwindow` ou `KeyBindings`; consome somente os cinco campos semânticos de um tick de simulação.

**Issue:** #138  
**PR:** #139  
**Branch:** `refactor/semantic-tick-input-boundary-20260828`  
**Merge:** `7da5af74e2ccc9c2a33d43cbfbcfacc6f5c04381`  
**Estado:** **DONE**

### Decisão

`TickInput` foi extraído para `Game/Logic/TickInput.h`. `InputManager` continua a produzir o contrato a partir do input físico/configurado, enquanto `Player` depende apenas do value object semântico.

O tipo permanece em `Logic`, não em `Core`, porque a evidência atual não justifica torná-lo um contrato transversal da aplicação.

### Validação

- Linux / Clang / C++20 / Headless Vulkan: success;
- Linux / Clang / ASan + UBSan / Headless Vulkan: success;
- Windows / Clang / C++20: success;
- source-size: success;
- campaign validation: success;
- `Player.h` deixou de incluir `InputManager.h`.

## Presentation configuration boundary — concluído

A auditoria pós-#139 encontrou `Game/Core/Config.h` a misturar constantes de Core/gameplay com configuração puramente visual. `WorldRenderer`, `EditorRenderer` e `RendererFacade` dependiam do header de Core para cores, clear colors e espaçamento visual.

Também foram identificadas `CAMERA_SPEED` e `CAMERA_OFFSET_Y` como constantes sem uso efetivo na implementação atual.

**Issue:** #140  
**PR:** #141  
**Branch:** `refactor/presentation-config-boundary-20260828`  
**Merge:** `6885ae63f0aacab16be1505643182d25378c1747`  
**Estado:** **DONE**

### Decisão

Foi criado `Game/Graphics/PresentationConfig.h` para configuração exclusivamente de presentation. O novo contrato contém cores de plataforma/jogador/bandeira, clear colors dos estados e espaçamento visual da grelha do editor.

`Core/Config.h` mantém dimensões lógicas, aspect ratio, timestep, física, gameplay e `EDITOR_GRID_SNAP`.

`CAMERA_SPEED` e `CAMERA_OFFSET_Y` foram removidos porque o código efetivo não os consome: `Camera::follow()` já possui default explícito de speed e calcula o offset a partir da altura lógica.

### Validação

- Linux / Clang / C++20 / Headless Vulkan: success;
- Linux / Clang / ASan + UBSan / Headless Vulkan: success;
- Windows / Clang / C++20: success;
- source-size: success;
- campaign validation: success;
- teste independente de `PresentationConfig.h` compilou e verificou valores representativos.

## Semantic LevelData geometry validation — em implementação

A auditoria da Fase 10 encontrou um gap concreto entre sintaxe e semântica: `LevelDataIO` rejeita tokens/linhas inválidos, mas podia aceitar `PLATFORM`/`FLAG` com largura ou altura zero/negativa, produzindo AABBs degeneradas ou invertidas antes do runtime.

**Issue:** #142  
**Branch:** `refactor/leveldata-semantic-validation-20260828`  
**Estado:** implementation in progress

### Decisão

Introduzir `Game/Logic/LevelDataValidator.h/.cpp` como boundary semântico independente. A primeira regra é estritamente geométrica:

```text
platform.width() > 0
platform.height() > 0
flag.width() > 0
flag.height() > 0
```

`CampaignRuntime` valida o documento imediatamente após `LevelDataIO::load()` e antes de `Level::appendFromData()`.

A tranche mantém schema/versioning, migration e bounds policy fora de escopo.

## RenderSnapshot — primeira tranche concluída

A primeira tranche da fronteira `RenderSnapshot` foi integrada no PR #129 e completada com a remoção do acoplamento `RendererFacade → EditorSession` no PR #132.

## Shared Vulkan image upload — concluído

PR #133 integrou `Game/Graphics/VulkanImageUpload.h/.cpp` como primitive estreito para o lifecycle comum de criação/upload de imagens Vulkan de `FontRendererGpu` e `SpriteRendererGpu`.

### Estado

`DONE`

## Outras dívidas explicitamente adiadas

- schema versioning/migration de `LevelData` — só após requisito real de compatibilidade/importação;
- semantic invariants adicionais de `LevelData` sem requisito/evidência;
- replay persistence;
- live-input frame-rate independence;
- terminal/result replay;
- future presentation snapshots apenas quando houver benefício verificável;
- eventual convergência entre `CampaignLoader` e o parsing usado por `CampaignID`, caso passe de duplicação atual para divergência observável.

## Regras preservadas

1. Renderer não lê input nem altera gameplay.
2. Gameplay não depende de teclas físicas.
3. `LevelData` não depende de Vulkan/GLFW.
4. Runtime não depende de CWD na claim exercitada.
5. `RuntimeBootstrap` é composição de startup, não `Application` genérica.
6. `LevelDataIO` continua parser/serializer, não schema authority.
7. Ordem de `Level::platforms()` não é assumida irrelevante sem evidência.
8. `ReplayManager` usa `TickInput`.
9. CI failure causes exigem evidência observável.
10. Implementation semantics e executable evidence continuam estados distintos.
11. Presentation recebe dados necessários para rendering, não o modelo mutável de gameplay/editor.
12. Shared Vulkan primitives devem permanecer estreitos e não absorver políticas específicas sem nova evidência.
13. Logic/Core não deve depender de tipos concretos de presentation quando apenas dados/contratos mínimos são necessários.
14. Gameplay deve depender de contratos semânticos de input, não do armazenamento/callbacks de input físico.
15. Configuração visual deve pertencer à camada de presentation; Core conserva apenas configuração necessária para contratos lógicos/gameplay.
16. LevelData semantic validation deve permanecer separada do parser sintático e ser aplicada antes do runtime consumir geometria.

## Próximo passo

```text
validar Issue #142
→ Linux normal + ASan/UBSan + Windows
→ merge e reconciliar documentação
→ voltar à auditoria final da Fase 10
```
