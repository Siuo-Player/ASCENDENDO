# Plano da próxima branch

**Bloco do roadmap:** `9.6 Base Engineering Gate — modularity`

**Work Package:** `SpriteRenderer review/decomposition`

## Objetivo

Investigar e, se a análise confirmar uma fronteira coesa, reduzir a responsabilidade concentrada em `Game/Graphics/SpriteRenderer.cpp` sem criar módulos artificiais nem alterar o contrato de rendering.

## Estado inicial confirmado

O `main` atual já contém a decomposição integrada do `FontRenderer` e foi validado pelo CI #387 com source-size, Vulkan, build/tests e campaign todos verdes.

A inspeção de `SpriteRenderer.cpp/.h` confirmou quatro áreas distintas:

1. decode CPU de PNG via `stb_image`;
2. criação/upload de imagem Vulkan, staging, barriers, image view e sampler;
3. descriptor/lifecycle e ownership dos recursos GPU;
4. emissão de draw e preparação de `SpritePushConstants`.

A semelhança entre a área 2/3 e a fronteira extraída do `FontRenderer` é uma evidência para investigação, não uma obrigação de copiar a arquitetura.

O teste `Tests/Integration/test_sprite_pipeline.cpp` valida o contrato público (`init`, dimensões e estado inicializado) e não depende da implementação interna.

## Hipótese arquitetural

A hipótese a testar é que a gestão/upload dos recursos Vulkan pode ser separada da preparação CPU da imagem e do draw, mantendo ownership explícito e uma interface pequena.

Alternativas consideradas:

- manter a classe monolítica apesar do tamanho, se as responsabilidades forem fortemente acopladas;
- extrair apenas uma fronteira específica de recursos GPU;
- reutilizar diretamente `FontRendererGpu`, rejeitada por enquanto porque os formatos, samplers e estado do sprite são diferentes e não queremos um módulo genérico prematuro.

## Dependências

**Depende de:**

- `main` verde;
- decomposição de `FontRenderer` integrada;
- pipeline e testes Vulkan existentes.

**Produz para:**

- hardening de modularidade;
- futura decomposição de `main.cpp`;
- eventual unificação de infraestrutura GPU apenas se surgir evidência suficiente.

**Consumidores afetados:**

- `RendererFacade` / passes que usam `SpriteRenderer`;
- `Tests/Integration/test_sprite_pipeline.cpp`;
- `SpritePipeline` apenas através do contrato de descriptor/pipeline.

## Fora de escopo

- `RenderSnapshot` geral;
- `main.cpp`;
- animação de sprites/atlases multi-frame;
- `AssetManager`;
- adaptive difficulty/player modelling;
- otimizações sem profiling;
- abstração GPU genérica entre fontes e sprites.

## WBS

```text
9.6 Base Engineering Gate
└── Modularity
    └── SpriteRenderer
        ├── inventariar responsabilidades
        ├── mapear ownership/dependências
        ├── verificar consumidores
        ├── comparar alternativas
        ├── decidir fronteira
        ├── implementar apenas se coesa
        ├── preservar failure paths e sampler NEAREST
        ├── validar build/tests/Vulkan/campaign
        └── atualizar arquitetura/dívida/roadmap
```

## Riscos

- copiar uma abstração do `FontRenderer` que não seja adequada;
- separar ownership Vulkan incorretamente;
- regressão de `VK_FORMAT_R8G8B8A8_UNORM` ou sampler `NEAREST`;
- quebrar a inicialização/destruição Vulkan;
- criar uma classe GPU genérica sem necessidade real.

## Validação

```text
source-size gate
build game
build tests
Vulkan integration tests
167+ test cases sem regressão
campaign validation
```

Também verificar que `STB_IMAGE_IMPLEMENTATION` continua a existir numa única translation unit e que o contrato público do `SpriteRenderer` permanece estável.

## Critério de saída

```text
cohesion decision documented
+ selected boundary justified by responsibility/coupling
+ ownership explicit
+ no artificial module split
+ build/tests/Vulkan/campaign verdes
+ documentation matches implementation
+ source-size policy respected
```

## Próximo work package

Só depois de integrar e revalidar esta branch: revisão/decomposição dos testes grandes e depois `main.cpp`, conforme o roadmap atual.

`RenderSnapshot` geral continua bloqueado até o Gate de Engenharia estar fechado.
