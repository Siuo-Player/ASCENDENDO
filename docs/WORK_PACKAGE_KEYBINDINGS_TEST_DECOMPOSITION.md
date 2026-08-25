# Work Package — KeyBindings test decomposition

**Roadmap:** `9.6 Base Engineering Gate`

## Discovery

O checker de source-size identificou `Tests/Unit/test_keybindings.cpp` com 305 linhas. A inspeção confirmou três responsabilidades de teste independentes:

1. comportamento base de `KeyBindings`;
2. persistência e compatibilidade de configurações;
3. integração `KeyBindings` ↔ `InputManager`.

O tamanho foi tratado como sinal para inspeção de coesão, não como motivo suficiente para dividir.

## Decision

Separar a cobertura por responsabilidade em:

- `Tests/Unit/test_keybindings_core.cpp`
- `Tests/Unit/test_keybindings_persistence.cpp`
- `Tests/Unit/test_keybindings_input.cpp`

O código de produção e os contratos públicos permanecem inalterados.

## Validation target

O baseline atual do repositório é 167 casos e 901 assertions. A decomposição deve preservar a cobertura funcional; alterações nas contagens só são aceitáveis quando explicadas por correção explícita de testes, não pelo rearranjo.

## Exit criteria

- ficheiro monolítico removido;
- três ficheiros com responsabilidades claras;
- nenhuma regressão de build/testes;
- nenhum novo warning de source-size decorrente desta decomposição;
- CI Linux/headless Vulkan e campaign verdes;
- próximo warning documentado antes de iniciar outra refatoração.
