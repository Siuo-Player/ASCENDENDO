# Work Package — Movement / Camera deterministic benchmark

## Context

PR #170 estabeleceu a propriedade de não-extrapolação de `Camera::follow()`. A auditoria seguinte não encontrou outra falha de produção com evidência suficiente para justificar alteração de runtime.

O study de 2026-08-29 recomenda um **Movement Feel Benchmark** determinístico: um conjunto pequeno de cenários que permita comparar alterações de movimento/câmara sem transformar hipóteses de design em regras arbitrárias.

## Decision

Criar uma primeira tranche pequena e independente:

```text
Development/AI_Validation/movement_camera_benchmark.py
```

O benchmark:

- não depende de Vulkan ou GLFW;
- não altera física, renderer ou comportamento de produção;
- replica apenas as fórmulas atuais necessárias para a validação;
- usa cenários determinísticos e resultados numéricos;
- pode emitir um relatório JSON para comparação posterior.

## Scenarios

```text
camera.follow large dt
camera.follow fixed-step tracking
camera.follow lower bound
camera worldToNDC mapping
viewport letterbox reference cases
viewport invalid dimensions
camera NDC finite reference grid
```

A referência de `follow()` usa o contrato já integrado em #170: fator limitado a `[0,1]`.

## Validation

Execução local esperada:

```text
python3 Development/AI_Validation/movement_camera_benchmark.py
```

Resultado não-zero indica falha de uma propriedade já conhecida. O relatório opcional pode ser produzido com:

```text
python3 Development/AI_Validation/movement_camera_benchmark.py --json report.json
```

## Scope boundary

Este benchmark não decide:

- look-ahead;
- dead/focus zones;
- camera lag ideal;
- difficulty;
- valores de game-feel;
- qualidade visual subjetiva.

Esses aspetos exigem experimentação de produto/playtesting e permanecem separados da validação matemática.

## Exit criteria

```text
benchmark determinístico executável
+ cenários sem dependências gráficas
+ propriedades explicitamente identificadas
+ resultado reproduzível
+ zero alteração ao runtime de produção
```

## Next decision

Usar o benchmark como base para futuras alterações de camera/movement. Adicionar um novo cenário apenas quando surgir uma propriedade operacional concreta ou uma regressão reproduzível.

O próximo passo de maior nível continua a ser um conjunto de stress scenes/render captures para validar visibilidade e comportamento da câmara em múltiplos viewports, sem inferir qualidade visual apenas dos testes matemáticos.
