# Plano da branch atual

**Bloco do roadmap:** `Fase 10 / presentation + camera validation`

**Work Package:** `Camera follow Lerp bound`

**Branch de implementação:** `fix/camera-follow-lerp-clamp-20260829`

**PR:** `#170`

## Contexto

`gfx::Camera::follow()` documentava tracking vertical por Lerp, mas usava diretamente `speed * dt` como fator. Para `speed * dt > 1`, a atualização tornava-se extrapolação.

## Decisão

Limitar o fator a `[0, 1]`:

```cpp
const float alpha = std::clamp(speed * dt, 0.0f, 1.0f);
```

Preserva o comportamento para o fixed-step normal e elimina overshoot causado por `dt` grande.

## Escopo

- `Game/Graphics/Camera.cpp`;
- teste regressivo em `Tests/Unit/test_camera.cpp` com `dt = 1.0f`;
- documentação do finding e da dívida técnica.

## Fora de escopo

- mudança do offset vertical;
- tracking horizontal;
- mudança do sistema de coordenadas;
- nova abstração de câmera;
- mudança da política de fixed-step.

## Validação

Head efetivamente validado: `2a96eed38c7cc87e456c03a3b15d1f712d57d4ea`.

```text
Linux / Clang / C++20 / Headless Vulkan       success
Linux / Clang / ASan + UBSan / Headless Vulkan success
Windows / Clang / C++20                       success
source-size                                    success
build + tests                                  success
campaign validation                            success
```

A PR #170 foi mergeada por squash como `284d4c807569dc5960c349a67ce0ef87f0aed4ec`.

## Resultado

`COMPLETED`

## Próxima decisão

Auditar propriedades objetivas de `Camera`/viewport:

```text
world → NDC
viewport boundaries
camera target tracking
camera lower bound
large/small viewport behavior
finite camera state
```

Não implementar uma segunda regra automaticamente. A passagem seguinte só avança quando existir propriedade operacional clara, consumidor afetado, risco/falha demonstrável e teste que a prove.

O estudo de 2026-08-29 também recomenda separar esta validação local de um futuro **Movement Feel Benchmark**, que deve ser determinístico e comparável entre alterações de movimento/câmera/VFX.
