# Work Package — FontRenderer decomposition

**Branch:** `refactor/font-renderer-decomposition-clean`

**Roadmap block:** `9.6 Base Engineering Gate`

**Work Package:** `Modularity — FontRenderer decomposition`

## Objetivo

Reduzir a responsabilidade concentrada no subsistema de fonts sem criar ficheiros artificiais, separando a gestão/ownership dos recursos Vulkan da preparação CPU do atlas e emissão de glyph draws.

## Estado inicial confirmado

A `main` validada no run #311 continha o `FontRenderer` distribuído entre `FontRenderer.cpp` e `FontRendererGpu.cpp`, mas o segundo ainda implementava métodos e ownership pertencentes à classe `FontRenderer`. A fronteira de responsabilidade ainda não existia.

## Decisão

A fronteira escolhida é:

```text
FontRenderer
├── leitura/baking stb_truetype
├── métricas dos glyphs
└── emissão de text draws

FontRendererGpu
├── staging/upload
├── VkImage + memory
├── VkImageView
├── VkSampler
└── descriptor pool/set + bind
```

A divisão é por ownership e responsabilidade, não por número de linhas.

## Alternativas consideradas

1. Manter tudo em `FontRenderer`: rejeitado porque mistura preparação CPU com ownership Vulkan.
2. Separar CPU baking e GPU upload em abstrações independentes: possível, mas acrescentaria outra interface sem benefício demonstrado nesta tranche.
3. Criar hierarquia genérica de assets/fonts: fora de escopo e sobre-engineering.

## Escopo

- mover ownership e implementação específica de recursos Vulkan para `FontRendererGpu`;
- manter baking CPU, métricas e desenho em `FontRenderer`;
- preservar `STB_TRUETYPE_IMPLEMENTATION` numa única translation unit;
- preservar inicialização, cleanup, bind e desenho;
- preservar failure paths;
- validar build, testes, Vulkan e campanha.

## Fora de escopo

- gameplay/physics;
- `RenderSnapshot`;
- `SpriteRenderer`;
- `main.cpp`;
- `AssetManager`;
- adaptive difficulty/player modelling;
- otimizações sem profiling;
- divisão artificial baseada apenas em tamanho.

## Dependências

**Depende de:** renderer stack integrada, CI verde em `main`, source-size gate e testes Vulkan existentes.

**Produz para:** revisão de `SpriteRenderer`, futura decomposição de `main.cpp` e continuação da separação de presentation.

## Consumidores afetados

- rendering de texto através de `RendererFacade`/`TextPipeline`;
- testes de integração Vulkan/rendering;
- Makefile/source-size checker devido à mudança de implementação para os mesmos ficheiros.

## Riscos

- ownership Vulkan ambíguo;
- cleanup incompleto em failure paths;
- duplicate `STB_TRUETYPE_IMPLEMENTATION`;
- alteração acidental do comportamento visual;
- interface nova sem redução real de coupling.

## Incidente durante implementação

A primeira versão da extração falhou no CI por acesso a `VulkanContext` incompleto em `FontRenderer.cpp`. O log confirmou que a implementação ainda chamava métodos concretos de `VulkanContext` enquanto o ponto em causa só tinha a forward declaration disponível. A correção foi adicionar explicitamente `Graphics/VulkanContext.h` à implementation unit.

A PR original desta tranche (#29) ficou em estado `mergeable=false`, apesar de ter a mesma base de `main` e a implementação final validada. Foi fechada sem merge e esta branch limpa foi recriada a partir do `main` atual, reaplicando apenas o estado final da tranche.

## Validação final necessária

```text
source-size gate
build game
build tests
Vulkan integration tests
167/167 test cases
901/901 assertions
campaign validation
```

Também confirmar uma única definição de `STB_TRUETYPE_IMPLEMENTATION` e ausência de consumidores da implementação anterior.

## Critério de saída

```text
FontRenderer deixa de possuir ownership dos recursos Vulkan
+ FontRendererGpu possui ownership claro
+ failure paths preservados
+ build/tests/Vulkan/campaign verdes
+ documentação congruente
+ sem alteração funcional não intencional
```

## Próximo bloco dependente

Depois desta PR e de nova validação verde em `main`: `SpriteRenderer` review/decomposition.
