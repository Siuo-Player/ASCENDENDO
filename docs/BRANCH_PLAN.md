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

`FontRendererGpu.cpp` e `SpriteRendererGpu.cpp` contêm duas implementações muito semelhantes do mesmo ciclo de vida Vulkan para imagens:

```text
findMemoryType
→ staging buffer / host-visible memory
→ image allocation/binding
→ one-time command buffer
→ layout transition
→ vkCmdCopyBufferToImage
→ layout transition
→ image view
→ sampler
→ descriptor resource setup
→ cleanup
```

A duplicação é real independentemente do limite físico de ficheiro. As diferenças observadas são propriedades específicas dos consumidores:

- fonte: `VK_FORMAT_R8_UNORM` + filtro `LINEAR`;
- sprite: `VK_FORMAT_R8G8B8A8_UNORM` + filtro `NEAREST`.

## Decisão arquitetural

Escolhida a opção **B — primitive estreito de upload/creation de imagem Vulkan**.

### Alternativas

**A — manter duplicação**

Rejeitada: mantém duas implementações da mesma sequência de ownership/lifecycle e duplica futuras correções.

**B — primitive estreito**

Escolhida: remove a duplicação comprovada sem esconder as propriedades específicas de cada recurso.

**C — `TextureManager` genérico**

Rejeitada: introduziria cache, ownership e política de recursos não justificados pelos consumidores atuais.

## Contrato pretendido

```text
FontRendererGpu ─┐
                 ├→ shared Vulkan image upload primitive
SpriteRendererGpu┘
```

O primitive:

- cria a imagem, memória, image view e sampler;
- executa staging/upload e as transitions comuns;
- devolve os handles ao consumidor;
- não mantém ownership persistente depois do retorno;
- limpa integralmente recursos temporários em qualquer failure path;
- recebe `format` e `filter` explicitamente;
- mantém usage limitado ao caso atualmente comprovado (`TRANSFER_DST | SAMPLED`).

Os descriptor pools/sets continuam nos consumidores, porque o binding/layout de descriptor pertence aos pipelines específicos de fonte e sprite.

## Ownership

```text
shared primitive
    cria
    ↓
resource handles
    ↓
consumer owns
    ↓
consumer cleanup
```

Não existe `TextureManager`, cache global ou ownership escondido.

## Em escopo

- implementar o primitive estreito;
- migrar `FontRendererGpu`;
- migrar `SpriteRendererGpu`;
- manter exatamente os formatos/filtros atuais;
- testar preconditions e failure cleanup;
- integrar com os testes Vulkan existentes;
- documentar ownership/failure paths;
- atualizar arquitetura/dívida/WP.

## Fora de escopo

- `TextureManager`;
- asset manager;
- cache de texturas;
- alteração de sampler/filter semantics;
- mudança de shaders/pipelines;
- otimização de performance sem profiling;
- alteração de gameplay/editor.

## Dependências

- `VulkanContext` atual;
- `FontRendererGpu`;
- `SpriteRendererGpu`;
- `docs/CODE_SIZE.md`;
- `docs/DEVELOPMENT_PROTOCOL.md`;
- Issue #22 como consumidor posterior.

## Validação

### Before

- confirmar duplicação e diferenças entre os dois consumidores;
- preservar formato, filter e lifecycle atuais;
- validar baseline com os workflows existentes.

### After

- uma implementação comum do upload;
- ownership sem ambiguidades;
- descriptor setup permanece específico;
- texto e sprite mantêm comportamento;
- game build + tests + sanitizers + Windows verdes;
- source-size sem alterações artificiais.

## Critério de saída

```text
primitive estreito e justificado
+ ownership explícito
+ FontRendererGpu migrado
+ SpriteRendererGpu migrado
+ failure paths limpos
+ testes/build/CI verdes
+ docs sincronizados
```

## Dívida / condição de revisão

Nenhuma dívida aceite por omissão. Se o primitive começar a acumular políticas específicas de novos tipos de recurso, a abstração deve ser revista antes de adicionar parâmetros genéricos.

## Próximo dependente

Após este WP, reconsiderar #22 `FontRenderer decomposition` e a decomposição de `SpriteRenderer` apenas com base nas responsabilidades que permanecerem após a remoção da infraestrutura Vulkan partilhada.