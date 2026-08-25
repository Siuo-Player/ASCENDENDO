# Migração do Renderer

## Objetivo

Substituir gradualmente o `Game/Graphics/Renderer.cpp` monolítico por módulos pequenos, mantendo o renderer antigo como referência de comportamento durante a migração.

A regra de tamanho para código é:

- < 30 KiB: normal;
- 30–36 KiB: warning e evitar adicionar responsabilidade;
- > 36 KiB: CI bloqueia e o ficheiro deve ser subdividido.

A regra aplica-se apenas a ficheiros de código C/C++ dentro de `Game/` e `Tests/`. Documentação, dados, assets, logs e scripts não entram neste limite.

## Estratégia

Não refatorar o renderer antigo em massa. O novo renderer é construído em paralelo.

```text
Renderer antigo (referência)
        │
        ├── comparação de comportamento
        │
        ▼
Novo rendering stack

RendererCore
    └── recursos Vulkan + acquire/submit/present + render pass

ShapeRenderer
    └── primitives / push constants

WorldRenderer
    └── plataformas + FLAG + Player/sprite

UiRenderer
    └── timer + PAUSED + MENU + CREDITS

EditorRenderer
    └── grid + plataformas + seleção + preview + cursor + HUD

RendererFacade
    └── orquestração dos passes
```

O renderer antigo continua no caminho de execução enquanto cada pass novo é comparado e validado.

## Estado atual da construção paralela

### `RendererCore` ✅

Construído de raiz com:

- criação/destruição de framebuffers;
- command pool e command buffers;
- semáforos/fence;
- acquire image;
- begin/end render pass;
- submit/present.

Não conhece Player, Level, menus nem editor.

### `ShapeRenderer` ✅

Substitui conceitualmente o lambda `drawRect()` do renderer antigo. Centraliza `PushConstants`, resolução lógica e binding da pipeline sólida.

### `EditorRenderer` ✅

Construído de raiz a partir do comportamento visual pretendido da secção `EDITOR`:

- grelha cullada pela câmera;
- plataformas;
- seleção;
- preview válido;
- cursor em cruz;
- HUD de ferramenta/tamanho.

Recebe `EditorRenderSnapshot`, não `LevelEditorDocument`.

### `WorldRenderer` ✅

Construído a partir do comportamento de `drawWorld()`:

- plataformas;
- FLAG;
- sprite do jogador;
- fallback do jogador para retângulo.

Não conhece GameState nem UI.

### `UiRenderer` ✅

Construído para separar:

- timer;
- PAUSED;
- MENU;
- CREDITS.

A geometria das caixas usa a mesma `MenuBoxLayout` do hit-test.

### `RendererFacade` ✅ estruturalmente

A fachada já orquestra `RendererCore + ShapeRenderer + WorldRenderer + UiRenderer + EditorRenderer` e possui `RenderState` próprio. Ainda **não é o renderer de produção**: primeiro precisamos validar paridade e depois substituir gradualmente o caminho antigo.

## Matriz de paridade

| Secção antiga | Novo módulo | Construção | Integração/paridade |
|---|---|---:|---|
| recursos/framebuffers/sync | `RendererCore` | ✅ | ⏳ CI/runtime |
| viewport/render pass | `RendererCore` | ✅ | ⏳ CI/runtime |
| `drawRect`/push constants | `ShapeRenderer` | ✅ | ⏳ CI |
| bloco `EDITOR` | `EditorRenderer` | ✅ | ⏳ runtime |
| `drawWorld` | `WorldRenderer` | ✅ | ⏳ runtime |
| timer | `UiRenderer` | ✅ | ⏳ runtime |
| PAUSED | `UiRenderer` | ✅ | ⏳ runtime |
| MENU | `UiRenderer` | ✅ | ⏳ runtime |
| CREDITS | `UiRenderer` | ✅ | ⏳ runtime |
| frame orchestration | `RendererFacade` | ✅ | ⏳ runtime |

## Migração sem risco

A migração segue três estados:

```text
1. Construir pass novo
       ↓
2. Comparar com renderer antigo + CI
       ↓
3. Trocar apenas aquele estado/pass para o novo
```

Durante a fase híbrida, o renderer antigo permanece como fallback. Nunca manteremos duas implementações como autoridade permanente.

## Próximos passos

1. Fazer o CI compilar a nova stack.
2. Criar o snapshot do editor no ponto real do runtime e ligar `EditorRenderer` ao estado `EDITOR`.
3. Substituir o bloco `EDITOR` antigo pela nova implementação e validar visualmente.
4. Substituir PLAYING/PAUSED pelo `WorldRenderer` + `UiRenderer`.
5. Substituir MENU/CREDITS.
6. Trocar `main.cpp` para `RendererFacade`.
7. Remover o caminho morto do `Renderer.cpp` antigo.
8. Só então eliminar o arquivo antigo, se a fachada não precisar dele como compatibilidade.

## Critério para remover o renderer antigo

Só remover `Renderer.cpp` quando:

1. todas as secções tiverem substituição funcional;
2. os testes existentes do renderer continuarem verdes;
3. PLAYING/PAUSED/MENU/CREDITS/EDITOR tiverem paridade visual aceitável;
4. o CI Linux Vulkan headless passar;
5. o CI Windows/build passar quando esse job estiver disponível;
6. não existir nenhum consumidor restante do renderer antigo.

## Regra importante

Não duplicar permanentemente lógica entre os dois sistemas. O renderer antigo é apenas uma referência temporária. Cada módulo novo deve ter responsabilidade clara e ficar abaixo do limite de tamanho de código definido pelo projeto.
