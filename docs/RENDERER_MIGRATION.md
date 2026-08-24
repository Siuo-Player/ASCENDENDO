# Migração do Renderer

## Objetivo

Substituir gradualmente o `Game/Graphics/Renderer.cpp` monolítico por módulos pequenos, mantendo o renderer antigo como referência de comportamento durante a migração.

A regra de tamanho para código é:

- < 30 KiB: normal;
- 30–36 KiB: warning e evitar adicionar responsabilidade;
- > 36 KiB: CI bloqueia e o ficheiro deve ser subdividido.

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
    └── recursos Vulkan + acquire/submit/present

ShapeRenderer
    └── primitive rectangles / push constants

WorldRenderer
    └── plataformas + FLAG + Player/sprite

UiRenderer
    └── timer + PAUSED + MENU + CREDITS

EditorRenderer
    └── grid + plataformas + seleção + preview + cursor + HUD
```

Quando um módulo novo atingir paridade funcional com a secção correspondente do renderer antigo, essa secção deixa de ser executada pelo renderer antigo.

## Ordem de migração

1. `RendererCore` — lifecycle e frame synchronization.
2. `ShapeRenderer` — primitive comum.
3. `EditorRenderer` — primeira área nova e testável sem conhecer o documento.
4. `WorldRenderer` — PLAYING/PAUSED world pass.
5. `UiRenderer` — HUD, MENU, PAUSED e CREDITS.
6. `RendererFacade` — orquestra todos os módulos com uma API compatível.
7. `main.cpp` passa a depender da fachada nova.
8. `Renderer.cpp` antigo fica sem consumidores; é removido apenas depois de uma CI com paridade verde.

## Matriz de paridade

| Secção antiga | Novo módulo | Estado |
|---|---|---|
| recursos/framebuffers/sync | `RendererCore` | ✅ construído |
| `drawRect`/push constants | `ShapeRenderer` | ✅ construído |
| bloco `EDITOR` | `EditorRenderer` | ✅ construído, integração pendente |
| `drawWorld` | `WorldRenderer` | ✅ construído, integração pendente |
| timer | `UiRenderer` | ✅ construído, integração pendente |
| PAUSED | `UiRenderer` | ✅ construído, integração pendente |
| MENU | `UiRenderer` | ✅ construído, integração pendente |
| CREDITS | `UiRenderer` | ✅ construído, integração pendente |
| frame orchestration | `RendererFacade` | ⏳ próximo |

## Critério para remover o renderer antigo

Só remover `Renderer.cpp` quando:

1. todas as secções tiverem substituição funcional;
2. os testes existentes do renderer continuarem verdes;
3. os estados PLAYING/PAUSED/MENU/CREDITS/EDITOR tiverem paridade visual aceitável;
4. o CI Linux Vulkan headless passar;
5. o CI de Windows/build também passar quando esse job estiver disponível.

## Regra importante

Não duplicar permanentemente lógica entre os dois sistemas. O renderer antigo é apenas uma referência temporária. Cada módulo novo deve ter uma responsabilidade clara e ficar abaixo do limite de tamanho de código definido pelo projeto.
