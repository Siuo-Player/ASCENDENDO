# Work Package — Camera follow Lerp bound

## Context

`gfx::Camera::follow()` documentava e testava tracking vertical por Lerp, mas usava diretamente `speed * dt` como fator de interpolação.

## Finding

Para `dt` grande, `speed * dt` pode exceder `1.0`. Nesse caso:

```text
position += (target - position) * factor
factor > 1
→ extrapolação
```

Isso contradizia o contrato de Lerp e podia fazer a câmera ultrapassar o alvo.

## Decision

Limitar o fator a `[0, 1]`:

```cpp
const float alpha = std::clamp(speed * dt, 0.0f, 1.0f);
```

O comportamento no `PhysicsWorld::FIXED_STEP` permanece equivalente porque o fator normal já está no intervalo válido.

## Validation

Caracterização adicionada com `dt = 1.0f` e `speed = 5.0f`, demonstrando que a câmera não ultrapassa o alvo.

No head real da PR (#170), os três workflows obrigatórios terminaram com sucesso:

- Linux / Clang / C++20 / Headless Vulkan — success;
- Linux / Clang / ASan + UBSan / Headless Vulkan — success;
- Windows / Clang / C++20 — success.

As execuções também passaram por source-size, build/testes e campaign validation.

## Result

**COMPLETED — PR #170 merged by squash as `284d4c807569dc5960c349a67ce0ef87f0aed4ec`.**

A integração foi feita a partir do head efetivo `2a96eed38c7cc87e456c03a3b15d1f712d57d4ea`. O SHA anteriormente referido como `8feefa7869b0cab37986427776351c202de22ea4` já não era o head atual quando a PR foi validada.

## Remaining uncertainty

A correção estabelece apenas a propriedade de não-extrapolação do follow. Não demonstra, por si só, a qualidade de tracking, visibilidade futura ou adequação da câmara como filtro de informação.

A próxima investigação deve avaliar propriedades objetivas de camera/viewport e avançar apenas quando existir consumidor, risco reproduzível e teste capaz de demonstrar a propriedade.

## Out of scope

- mudança do offset vertical;
- tracking horizontal;
- mudança do sistema de coordenadas;
- nova abstração de câmera;
- mudança da política de fixed-step.
