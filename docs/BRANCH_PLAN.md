# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`

**Work Package:** `Presentation-owned configuration`

**Issue:** `#140`

**Branch de implementação:** `refactor/presentation-config-boundary-20260828`

## Contexto

A auditoria pós-#139 encontrou `Game/Core/Config.h` a misturar constantes de Core/gameplay com configuração puramente visual. `WorldRenderer`, `EditorRenderer` e `RendererFacade` dependiam do header de Core para cores, clear colors e espaçamento visual.

## Descoberta

A separação real é:

```text
Core/Config
  → dimensões lógicas / timestep / física / gameplay / regras semânticas

Graphics/PresentationConfig
  → cores / clear colors / apresentação visual do editor
```

Também existem `CAMERA_SPEED` e `CAMERA_OFFSET_Y` em `Core/Config.h` sem uso efetivo na implementação atual.

## Decisão

Criar `Game/Graphics/PresentationConfig.h` e mover apenas constantes comprovadamente de presentation. Manter `LOGICAL_WIDTH`, `LOGICAL_HEIGHT`, `TARGET_ASPECT`, física/gameplay e `EDITOR_GRID_SNAP` em Core.

Remover apenas `CAMERA_SPEED` e `CAMERA_OFFSET_Y`, pois o código atual usa um default explícito para speed e calcula o offset diretamente.

## Escopo

- adicionar `Graphics/PresentationConfig.h`;
- migrar `WorldRenderer`, `EditorRenderer` e `RendererFacade`;
- limpar `Core/Config.h` dos valores visuais;
- caracterizar o novo header;
- atualizar documentação.

## Fora de escopo

- alterar valores visuais;
- alterar semântica da Camera/viewport;
- mover dimensões lógicas ou física para Graphics;
- criar sistema genérico de settings;
- modificar `EDITOR_GRID_SNAP`.

## Validação

```text
presentation config compile characterization
→ Linux normal
→ ASan/UBSan
→ Windows
→ source-size/campaign validation
```

## Critério de saída

```text
Graphics consumers no longer take presentation policy from Core/Config
+ core constants remain in Core
+ all presentation numeric values preserved
+ no behavior change
+ CI mandatory gates green
+ documentation synchronized
```

## Estado atual

`IMPLEMENTED — pending PR/CI validation`

## Próxima decisão

Depois do #140, voltar à auditoria de ownership e só abrir nova tranche para um coupling real com evidência suficiente.
