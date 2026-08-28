# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`

**Work Package concluído:** `Presentation-owned configuration`

**Issue:** `#140`

**Branch de implementação:** `refactor/presentation-config-boundary-20260828`

## Resultado

O finding foi resolvido no PR #141. A configuração exclusivamente visual passou para `Game/Graphics/PresentationConfig.h`; `Game/Core/Config.h` ficou restrito a dimensões lógicas, aspect ratio, timestep, física, gameplay e `EDITOR_GRID_SNAP`.

A alteração preservou os valores numéricos usados pela apresentação e removeu apenas `CAMERA_SPEED` e `CAMERA_OFFSET_Y`, que não tinham consumidores efetivos na implementação atual.

```text
Core/Config
  → logical/gameplay semantics

Graphics/PresentationConfig
  → visual presentation policy
```

## Validação

- Linux / Clang / C++20 / Headless Vulkan: **success**
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**
- Windows / Clang / C++20: **success**
- source-size: **success**
- campaign validation: **success**

**Merge:** `6885ae63f0aacab16be1505643182d25378c1747`

## Próxima decisão

Voltar à auditoria de ownership/arquitetura. Só abrir nova tranche para coupling concreto com contrato mínimo verificável; a duplicação de parsing `CampaignLoader`/`CampaignID` permanece apenas potencial enquanto não houver divergência observável.
