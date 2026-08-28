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
Likely cause: missing direct include of the concrete Camera definition.
Confirmed cause: missing direct include of `Graphics/Camera.h` in `EditorRenderer.cpp`.
Scope relation: `EditorRenderer.cpp` is outside the intended #135/#136 change; the failure does not originate in the editor-Logic boundary change.
Fix: separate issue/PR to add the direct include.
Validation: rerun Linux normal, Linux ASan/UBSan and Windows after the fix.
```

## RenderSnapshot — primeira tranche concluída

A primeira tranche da fronteira `RenderSnapshot` foi integrada no PR #129 e completada com a remoção do acoplamento `RendererFacade → EditorSession` no PR #132.

## Shared Vulkan image upload — concluído

PR #133 integrou `Game/Graphics/VulkanImageUpload.h/.cpp` como primitive estreito para o lifecycle comum de criação/upload de imagens Vulkan de `FontRendererGpu` e `SpriteRendererGpu`.

### Estado

`DONE`

## Outras dívidas explicitamente adiadas

- semantic validation/schema/versioning de `LevelData` — Fase 10;
- replay persistence;
- live-input frame-rate independence;
- terminal/result replay;
- future presentation snapshots apenas quando houver benefício verificável.

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
13. Logic não deve depender de tipos concretos de presentation quando apenas os dados mínimos são necessários.

## Próximo passo

```text
corrigir EditorRenderer include blocker
→ concluir PR #136
→ varrer restantes dependências Logic → Graphics
```
