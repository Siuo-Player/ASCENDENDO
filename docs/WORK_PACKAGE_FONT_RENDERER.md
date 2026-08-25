# Work Package — FontRenderer decomposition

**Branch:** `refactor/font-renderer-decomposition`

**Roadmap block:** `9.6 Base Engineering Gate`

**Work Package:** `Modularity — FontRenderer decomposition`

## Objetivo

Reduzir a responsabilidade concentrada em `Game/Graphics/FontRenderer.cpp` sem criar divisões artificiais, preservando exatamente o contrato de rendering existente e mantendo `STB_TRUETYPE_IMPLEMENTATION` numa única translation unit.

## Estado inicial confirmado

O histórico do incidente de source-size identifica `FontRenderer.cpp` com 430 linhas.

A inspeção de `main` após a integração da PR #25 confirmou que **não existe** atualmente `FontRendererGpu.cpp`. A implementação de baking CPU, upload Vulkan, descriptor allocation e lifecycle continua no próprio `FontRenderer.cpp`.

Esta correção do histórico é importante: trabalhos futuros devem partir do estado efetivamente presente no repositório, não de uma descrição anterior da conversa.

## Escopo

- analisar responsabilidades atuais do `FontRenderer`;
- separar responsabilidades apenas quando houver coesão clara;
- isolar, se a análise confirmar a fronteira, o código específico de upload/recursos Vulkan;
- preservar o baking `stb_truetype` e a sua única definição da implementação;
- manter contrato de inicialização, cleanup, drawing e descriptor usage;
- adicionar/ajustar testes apenas quando a nova fronteira puder ser validada de forma determinística;
- atualizar arquitetura, dívida e plano de branch com o resultado real.

## Fora de escopo

- alterar a aparência tipográfica;
- alterar atlas dimensions/pixel height sem necessidade arquitetural;
- alterar sampler/filtering por preferência estética;
- substituir `stb_truetype`;
- introduzir `AssetManager`;
- alterar `RendererFacade`/`RenderSnapshot`;
- otimizações Vulkan não justificadas por profiling;
- subdividir ficheiros apenas para satisfazer o limite de linhas.

## Dependências

**Depende de:**

- `RendererCore` / `RendererFacade` estáveis;
- CI verde em `main`;
- política de source-size integrada;
- testes atuais do renderer e Vulkan.

**Produz para:**

- `SpriteRenderer` review;
- decomposição arquitetural de `main.cpp`;
- manutenção futura dos recursos de texto;
- eventual `RenderSnapshot` geral, sem depender diretamente dele.

## Consumidores afetados

- `RendererFacade` / text rendering pass;
- `TextPipeline`;
- testes de integração relacionados com rendering/Vulkan;
- Makefile/source-size checker apenas se surgirem novos ficheiros.

## Análise de responsabilidades a realizar antes da implementação

Classificar o código atual nas seguintes responsabilidades e verificar coupling entre elas:

```text
Font metrics / baking
        ↓
CPU atlas representation
        ↓
GPU image + staging upload
        ↓
image view / sampler / descriptor
        ↓
FontRenderer lifecycle
        ↓
glyph drawing / push constants
```

A existência dessas categorias não implica automaticamente um ficheiro por categoria. A fronteira final será escolhida pela coesão e pelo ownership dos recursos.

## Alternativas

### A — manter tudo no `FontRenderer`

Aceitável apenas se a análise mostrar que as responsabilidades são fortemente coesas e a divisão aumentaria coupling.

### B — extrair apenas GPU resource/upload

Candidato principal. Cria uma fronteira pequena e testável entre preparação do atlas e recursos Vulkan.

### C — extrair CPU font baking e GPU upload separadamente

Mais modular, mas só deve ser escolhido se os tipos/ownership puderem permanecer simples e o custo de interfaces não superar o benefício.

### D — criar uma hierarquia genérica de font/asset abstractions

Fora de escopo. Seria sobre-engineering para esta tranche.

## Riscos

- ownership de `VkImage`/`VkDeviceMemory` ficar ambíguo;
- destruição incompleta em failure paths;
- duplicar `STB_TRUETYPE_IMPLEMENTATION`;
- aumentar o número de interfaces sem reduzir coupling real;
- modificar o comportamento do sampler ou atlas sem intenção;
- criar ficheiros pequenos artificialmente.

## Validação

Antes de merge:

```text
source-size gate ✅
build game ✅
build tests ✅
167+ tests / assertions sem regressão
Vulkan integration tests ✅
campaign validation ✅
```

Além disso:

- procurar todas as referências à implementação atual;
- confirmar que existe exatamente uma definição de `STB_TRUETYPE_IMPLEMENTATION`;
- validar failure paths de criação/alocação quando a responsabilidade for extraída;
- comparar comportamento visual/funcional sem alterar o contrato.

## Critério de saída

```text
FontRenderer.cpp está abaixo do gate de ERROR ou existe exceção arquitetural explícita;
+ cada responsabilidade extraída tem ownership claro;
+ nenhum consumidor transitório foi introduzido;
+ testes/build/campaign passam;
+ documentação reflete a estrutura real;
+ não há alteração de comportamento não justificada.
```

## Próximo bloco dependente

Depois desta PR e de nova validação verde em `main`: revisão/decomposição de `SpriteRenderer.cpp` antes de iniciar a decomposição de `main.cpp`.
