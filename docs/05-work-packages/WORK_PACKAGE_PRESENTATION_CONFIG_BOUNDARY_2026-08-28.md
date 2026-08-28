# Work Package — Presentation configuration boundary

## Identificação

**Roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`  
**Work Package:** `Presentation-owned configuration`  
**Issue:** `#140`  
**Implementation branch:** `refactor/presentation-config-boundary-20260828`

## Contexto

`Game/Core/Config.h` mistura constantes de Core/gameplay com configuração puramente visual. Os renderers gráficos dependem desse header para cores, clear colors e parâmetros visuais do editor.

## Evidência

Dependências concretas observadas:

```text
WorldRenderer.cpp   → Core/Config.h → COLOR_*
EditorRenderer.cpp  → Core/Config.h → COLOR_* / grid visual
RendererFacade.cpp  → Core/Config.h → CLEAR_*
```

`Camera.cpp` ainda precisa legitimamente de `LOGICAL_WIDTH/HEIGHT`, que permanecem em Core.

Também foram identificadas `CAMERA_SPEED` e `CAMERA_OFFSET_Y` como configurações sem uso efetivo na implementação atual: o speed efetivo está no default de `Camera::follow()` e o offset é calculado diretamente a partir da altura lógica.

## Decisão

Criar `Game/Graphics/PresentationConfig.h` como owner de configuração exclusivamente visual.

Mover para lá:

- cores de plataformas, jogador e bandeira;
- clear colors de estados de presentation;
- espaçamento visual da grelha do editor.

Manter em `Core/Config.h`:

- dimensões lógicas e aspect ratio;
- fixed timestep e física;
- dimensões/velocidade/salto do jogador;
- `EDITOR_GRID_SNAP`, por ser regra de edição e não parâmetro visual.

Remover `CAMERA_SPEED` e `CAMERA_OFFSET_Y`, que não têm consumidores efetivos no código atual.

## Invariante

A mudança é de ownership/configuração, não de comportamento. Os valores numéricos usados pela apresentação devem permanecer idênticos.

## Escopo

- criar `PresentationConfig.h`;
- migrar consumidores gráficos comprovados;
- limpar `Core/Config.h` dos valores de apresentação;
- caracterizar a existência e os valores representativos do novo header;
- atualizar documentação.

## Fora de escopo

- alterar valores visuais;
- redesenhar o sistema de configuração;
- mover dimensões lógicas ou física para Graphics;
- alterar Camera ou viewport semantics;
- introduzir runtime settings genéricos.

## Validação

```text
presentation config compile characterization
→ full Linux tests/headless Vulkan
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
Graphics consumers no longer require presentation constants from Core/Config
+ Core/Config keeps only core/gameplay/editor-semantic config
+ visual values unchanged
+ CI mandatory gates green
+ documentation synchronized
```

## Próxima decisão

Após esta tranche, voltar à auditoria global. Não continuar a dividir configuração sem uma fronteira de ownership observável.
