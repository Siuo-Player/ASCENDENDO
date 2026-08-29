# Work Package — Camera follow Lerp bound

## Context

`gfx::Camera::follow()` documenta e testa um tracking vertical por Lerp. A implementação usava diretamente `speed * dt` como fator de interpolação.

## Finding

Para `dt` grande, `speed * dt` pode exceder `1.0`. Nesse caso:

```text
position += (target - position) * factor
factor > 1
→ extrapolação
```

Isso contradiz o contrato de Lerp e pode fazer a câmera ultrapassar o alvo.

## Decision

Limitar o fator a `[0, 1]`:

```cpp
const float alpha = std::clamp(speed * dt, 0.0f, 1.0f);
```

O comportamento para o `PhysicsWorld::FIXED_STEP` permanece equivalente porque o fator normal já está no intervalo válido.

## Validation

Adicionar caracterização com `dt = 1.0f` e `speed = 5.0f`, demonstrando que a câmera não ultrapassa o alvo.

Validar nos três workflows obrigatórios:

- Linux / Clang / C++20 / Headless Vulkan;
- Linux / Clang / ASan + UBSan / Headless Vulkan;
- Windows / Clang / C++20.

Também manter source-size e campaign validation verdes.

## Out of scope

- mudança do offset vertical;
- tracking horizontal;
- mudança do sistema de coordenadas;
- nova abstração de câmera;
- mudança da política de fixed-step.
