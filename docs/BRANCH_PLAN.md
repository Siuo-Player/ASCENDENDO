# Plano da branch atual

**Bloco do roadmap:** `9.6 Base Engineering Gate → D — modularity work packages`

**Work Package:** `D.0 — shared Vulkan image upload primitive`

**Issue:** `#23`

**Branch:** `refactor/shared-vulkan-image-upload-20260828`

## Base confirmada

`main` integra:

- `GameSession` como fronteira de estado de sessão sem ownership Vulkan/presentation;
- `GraphicsRuntime` como dono do stack gráfico;
- `PresentationRuntime` como dono dos recursos de texto/sprite;
- `gfx::RenderSnapshot` para o world/player path;
- `EditorRenderSnapshot` materializado na composição antes do rendering;
- Gate 9.6 formalmente `CLOSED`;
- PR #129 e PR #132 integrados com os três workflows obrigatórios verdes.

## Descoberta

`FontRendererGpu.cpp` e `SpriteRendererGpu.cpp` contêm duas implementações muito semelhantes do mesmo ciclo de vida Vulkan para imagens. A duplicação é concreta e independente do limite físico de ficheiro.

## Decisão arquitetural

Escolhida a opção **B — primitive estreito de upload/creation de imagem Vulkan**.

A opção A mantém duplicação de lifecycle/failure handling. A opção C (`TextureManager`) é especulativa para os consumidores atuais e adicionaria ownership/política não justificadas.

## Contrato pretendido

```text
FontRendererGpu ─┐
                 ├→ shared Vulkan image upload primitive
SpriteRendererGpu┘
```

O primitive cria a imagem, memória, image view e sampler, executa staging/upload e transitions comuns e devolve os handles ao consumidor. Não mantém ownership persistente depois do retorno.

`format` e `filter` permanecem explícitos. Descriptor pools/sets continuam nos consumidores.

## Validação

```text
baseline
→ primitive + failure cleanup
→ FontRendererGpu
→ SpriteRendererGpu
→ game build + tests
→ sanitizers
→ Windows
→ documentação final
```

## Critério de saída

```text
primitive estreito e justificado
+ ownership explícito
+ ambos os consumidores migrados sem regressão
+ failure paths limpos
+ testes/build/CI verdes
+ docs sincronizados
```

## Dívida / condição de revisão

Se o primitive começar a acumular políticas específicas de outros recursos, a abstração será revista antes de adicionar parâmetros genéricos.

## Próximo dependente

Após este WP, reconsiderar #22 `FontRenderer decomposition` e a decomposição de `SpriteRenderer` com base nas responsabilidades que permanecerem depois da remoção da infraestrutura Vulkan duplicada.