# Plano da branch atual

**Branch:** `refactor/font-renderer-decomposition`

**Bloco do roadmap:** `9.6 Base Engineering Gate — modularity`

**Work Package:** `FontRenderer decomposition`

## Objetivo

Reduzir a responsabilidade concentrada em `Game/Graphics/FontRenderer.cpp` através de uma divisão coesa e verificável, sem criar módulos artificiais nem alterar o contrato de rendering.

## Estado inicial confirmado

A `main` pós-PR #25 foi validada pelo CI #311 com source-size, Vulkan, build/tests e campaign todos verdes.

A inspeção do código confirmou que `Game/Graphics/FontRenderer.cpp` continua a conter baking CPU, upload Vulkan, criação de image/view/sampler/descriptor e lifecycle. Não existia `Game/Graphics/FontRendererGpu.cpp` em `main`; a descrição anterior que dizia o contrário foi corrigida antes da implementação.

## Decisão de fronteira

A análise confirmou que a gestão de recursos Vulkan do atlas é uma responsabilidade coesa e distinta do baking CPU/glyph metrics. A branch extraiu essa responsabilidade para `FontRendererGpu`, mantendo em `FontRenderer` o baking, métricas, lifecycle de alto nível e emissão de glyph draws.

Não foi criada uma abstração genérica de assets nem uma classe por cada operação Vulkan.

## Falha CI observada e resolução

A primeira execução após a extração falhou no build do jogo. Causa confirmada pelo log:

```text
Game/Graphics/FontRenderer.cpp:72:21: error: member access into incomplete type 'VulkanContext'
Game/Graphics/FontRenderer.cpp:86:32: error: member access into incomplete type 'VulkanContext'
Game/Graphics/FontRenderer.cpp:106:28: error: member access into incomplete type 'VulkanContext'
```

A causa foi a remoção indevida do include que fornece a definição completa de `VulkanContext` durante a decomposição. A correção foi manter `Graphics/VulkanContext.h` incluído na translation unit que utiliza os seus métodos. Não houve alteração da fronteira arquitetural para resolver a falha.

A execução seguinte, **CI #343**, terminou verde: source-size ✅, Vulkan ✅, build/tests ✅ e campaign ✅.

## Documentos obrigatórios

- `docs/DEVELOPMENT_PROTOCOL.md`
- `docs/PROJECT_MANAGEMENT.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/WORK_PACKAGE_FONT_RENDERER.md`
- `docs/CODE_SIZE.md`

## Escopo

- analisar coesão de baking, upload GPU, descriptors e lifecycle;
- escolher a fronteira mínima que reduza responsabilidade/coupling;
- preservar `STB_TRUETYPE_IMPLEMENTATION` numa única translation unit;
- preservar ownership e failure paths dos recursos Vulkan;
- ajustar testes/documentação conforme a nova fronteira.

## Fora de escopo

- mudanças de gameplay;
- `RenderSnapshot` geral;
- `SpriteRenderer`;
- `main.cpp`;
- `AssetManager`;
- adaptive difficulty/player modelling;
- otimizações sem profiling;
- divisão artificial baseada apenas em linhas.

## Dependências

**Depende de:**

- CI verde da `main`;
- renderer stack integrada;
- source-size policy ativa;
- testes existentes do renderer/Vulkan.

**Produz para:**

- revisão de `SpriteRenderer`;
- futura decomposição de `main.cpp`;
- manutenção de recursos de texto.

**Consumidores:**

- `RendererFacade`/text rendering;
- `TextPipeline`;
- testes Vulkan/rendering.

## WBS

```text
9.6 Base Engineering Gate
└── Modularity
    └── FontRenderer
        ├── inventariar responsabilidades
        ├── mapear ownership/dependências
        ├── comparar alternativas
        ├── extrair fronteira mínima coesa
        ├── preservar failure paths
        ├── validar build/tests/Vulkan/campaign
        └── atualizar arquitetura/dívida/roadmap
```

## Riscos

- ownership Vulkan ambíguo após extração;
- destruction paths incompletos;
- duplicate symbol de `stb_truetype`;
- nova interface com pouco benefício;
- regressão de sampler/atlas/descriptor;
- criação de ficheiros artificiais.

## Validação

```text
source-size gate
build game
build tests
Vulkan integration tests
167+ test cases sem regressão
campaign validation
```

Também verificar exatamente uma ocorrência de `STB_TRUETYPE_IMPLEMENTATION` e procurar todos os consumidores do `FontRenderer` antes/depois.

## Critério de saída

```text
FontRenderer.cpp abaixo do ERROR ou exceção arquitetural documentada
+ ownership claro
+ nenhuma dependência transitória desnecessária
+ build/tests/Vulkan/campaign verdes
+ documentação congruente com implementação
+ comportamento preservado
```

## Próximo work package

Depois de integrar esta branch e validar novamente `main`: **9.6 Base Engineering Gate — SpriteRenderer review/decomposition**.

`RenderSnapshot` geral continua bloqueado até o Gate de Engenharia estar fechado.