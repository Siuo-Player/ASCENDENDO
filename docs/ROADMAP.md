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

## Next Modularity Block — Shared Vulkan image upload

**Issue:** #23  
**Implementation:** PR #133  
**Merge:** `e3871bc935dfa52124ec5244ddbb04714caec161`  
**Estado:** **COMPLETED**

### Descoberta

`FontRendererGpu.cpp` e `SpriteRendererGpu.cpp` duplicavam o lifecycle comum de upload de imagens Vulkan: staging, memory allocation, image creation/binding, command buffer one-shot, transitions, copy, image view, sampler e cleanup.

### Decisão

Escolhida a opção **B — primitive estreito de upload/creation de imagem Vulkan**. Não foi criado `TextureManager`, cache global ou asset abstraction.

```text
FontRendererGpu ─┐
                 ├→ VulkanImageUpload
SpriteRendererGpu┘
```

`VkFormat` e `VkFilter` continuam explícitos. Descriptor pools/sets continuam nos consumidores. O ownership dos handles devolvidos continua nos consumidores.

### Resultado

`Game/Graphics/VulkanImageUpload.h/.cpp` centraliza o lifecycle comum. Os consumidores preservam:

- Font: `R8_UNORM` + `LINEAR`;
- Sprite: `R8G8B8A8_UNORM` + `NEAREST`.

### Evidência

Os três workflows obrigatórios passaram no head validado de #133:

- Linux / Clang / C++20 / Headless Vulkan;
- Linux / Clang / ASan + UBSan / Headless Vulkan;
- Windows / Clang / C++20.

Também passaram source-size, full tests, headless Vulkan e campaign validation.

## Próximo alvo

Reavaliar **#22 `FontRenderer decomposition`** e a decomposição de `SpriteRenderer` agora que a infraestrutura Vulkan duplicada foi removida. A próxima decisão deve partir das responsabilidades que restaram, não de uma meta arbitrária de tamanho.

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