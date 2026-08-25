# Migração do Renderer

## Objetivo

Substituir gradualmente o `Game/Graphics/Renderer.cpp` monolítico por módulos pequenos, mantendo o renderer antigo como referência temporária de comportamento.

Regra de tamanho para código:

- `< 30 KiB`: normal;
- `30–36 KiB`: warning; evitar adicionar responsabilidade;
- `> 36 KiB`: CI bloqueia e o ficheiro deve ser subdividido.

Aplica-se apenas a código C/C++ em `Game/` e `Tests/`. Documentação, dados, assets, logs e scripts não entram neste limite.

## Estratégia

O renderer antigo **não é refatorado em massa**. A nova stack foi construída em paralelo e cada estado é migrado individualmente.

```text
Renderer antigo (referência temporária)
        │
        ├── comparação de comportamento
        ▼
Nova rendering stack

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

## Estado atual

### `RendererCore` ✅

Responsável por recursos e sincronização Vulkan, sem conhecer gameplay, editor ou UI.

### `ShapeRenderer` ✅

Centraliza primitives, push constants, resolução lógica e binding da pipeline.

### `EditorRenderer` ✅

Recebe `EditorRenderSnapshot` e desenha a tela fixa do Level Editor. Não recebe diretamente `LevelEditorDocument`.

### `WorldRenderer` ✅ estruturalmente

Encapsula plataformas, FLAG e Player/sprite.

### `UiRenderer` ✅ estruturalmente

Encapsula timer, PAUSED, MENU e CREDITS.

### `RendererFacade` ✅ estruturalmente

Orquestra os passes e tem `RenderState` próprio.

### `RendererFacadeAdapter` ✅

Mantém compatibilidade com o runtime enquanto a migração termina.

## Cut-over atual

A 9.4 fechada pela PR #8 integrou o estado `EDITOR` na nova stack através de `RendererFacadeAdapter`.

Estado atual desejado:

| Estado | Implementação nova | Legado ainda necessário |
|---|---|---|
| EDITOR | ✅ | referência temporária |
| PLAYING | ⏳ | ✅ |
| PAUSED | ⏳ | ✅ |
| MENU | ⏳ | ✅ |
| CREDITS | ⏳ | ✅ |

Os próximos cut-overs devem seguir a mesma regra: construir → comparar → validar → substituir.

## Requisitos para a nova UI

A migração não deve perpetuar problemas de layout do renderer antigo. A nova UI deve respeitar:

- viewport lógico `640x360`;
- letterboxing sem deformação;
- layouts autoajustáveis para texto e número variável de opções;
- nenhuma informação importante cortada nas extremidades;
- rodapé contextual com ações essenciais;
- tamanhos de texto que possam ser reduzidos dentro de limites legíveis.

## Critério para remover o renderer antigo

Só remover `Renderer.cpp` quando:

1. todas as secções tiverem substituição funcional;
2. os testes existentes continuarem verdes;
3. PLAYING/PAUSED/MENU/CREDITS/EDITOR tiverem paridade visual aceitável;
4. o CI Linux Vulkan headless passar;
5. o CI Windows/build passar quando esse job estiver disponível;
6. não existir consumidor restante do renderer antigo.

## Regra importante

Não manter duas implementações como autoridades permanentes. O renderer antigo existe apenas como referência/fallback temporário.

Cada módulo novo deve ter responsabilidade clara e respeitar o limite de tamanho de código definido pelo projeto.
