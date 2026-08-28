# Plano da branch atual

**Bloco do roadmap:** `Post-Gate 9.6 architecture / ownership boundaries`

**Work Package concluído:** `Semantic TickInput contract`

**Issue:** `#138`

**Branch de implementação:** `refactor/semantic-tick-input-boundary-20260828`

## Resultado

O finding foi resolvido no PR #139. `TickInput` passou a viver em `Game/Logic/TickInput.h`; `InputManager` continua a produzir o contrato semântico e `Player` passou a depender diretamente desse value object, sem incluir `InputManager.h`.

A fronteira resultante é:

```text
hardware / bindings
        ↓
InputManager
        ↓
semantic TickInput
        ↓
Player / simulation
```

## Validação

- Linux / Clang / C++20 / Headless Vulkan: **success**
- Linux / Clang / ASan + UBSan / Headless Vulkan: **success**
- Windows / Clang / C++20: **success**
- source-size: **success**
- campaign validation: **success**

**Merge:** `7da5af74e2ccc9c2a33d43cbfbcfacc6f5c04381`

## Próxima decisão

Voltar à revisão final de ownership/arquitetura. Só abrir nova tranche quando existir um finding concreto com coupling observável e contrato mínimo verificável. A auditoria atual observou duplicação de parsing entre `CampaignLoader` e `CampaignID`, mas sem divergência comportamental demonstrada; permanece apenas como dívida potencial.

```text
investigar
→ confirmar finding
→ documentar
→ implementar menor mudança suficiente
→ validar
→ reconciliar documentação
```
