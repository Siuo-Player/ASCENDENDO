# Work Package — Runtime bootstrap boundary

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`
**Subsystem:** `Process / Runtime bootstrap`
**Work Package:** `9.6 Runtime bootstrap boundary`
**Branch:** `refactor/9-6-runtime-bootstrap-boundary`
**Predecessor:** PR #75 — `LevelData` boundary, merged as `d5d69a77cc78ee8079bd9d590b4e10c0e483b41f`

## Objetivo

Reduzir a responsabilidade de composição de dados de `main.cpp` sem criar uma `Application` nominal.

A fronteira proposta deve encapsular apenas o bootstrap determinístico que já possui coesão própria:

```text
process argv
   ↓
RuntimePaths
   ↓
user-data directories
   ↓
campaign discovery
   ↓
campaign identity
```

A criação de `GraphicsRuntime`, `PresentationRuntime`, `InputManager`, `Camera` e o loop/frame submission permanecem no entry point nesta tranche.

## Contexto / descoberta

Após `GameSession`, `PresentationRuntime` e `LevelData`, `main.cpp` já não possui as regras de gameplay. Contudo, ainda compõe diretamente:

- resolução de `RuntimePaths` a partir de `argv[0]`;
- criação das diretorias de dados do utilizador;
- derivação de strings de caminhos;
- carregamento de `campaign.txt` através de `CampaignLoader`;
- cálculo de `CampaignID`;
- construção de `GameSession` com esses resultados.

`RuntimePaths`, `CampaignLoader` e `CampaignID` já possuem responsabilidades próprias e testes independentes do CWD.

## Decisão arquitetural

Criar uma pequena fronteira de bootstrap de domínio/application-data, por exemplo `RuntimeBootstrap`, que devolve um resultado de composição simples e sem ownership gráfico:

```text
RuntimeBootstrap
├── RuntimePaths
├── campaign paths
├── campaign ID
├── runs path
└── controls path
```

A classe não deve:

- possuir GLFW/Vulkan;
- criar `GraphicsRuntime`;
- possuir `GameSession`;
- executar o frame loop;
- decidir regras de gameplay.

## Critério de qualidade

A nova fronteira só é válida se retirar conhecimento de bootstrap de `main.cpp` sem criar um objeto "god" ou simplesmente mover código sem reduzir coupling.

## Inclui

- encapsular bootstrap de `RuntimePaths` + campaign discovery/ID;
- definir resultado explícito para falhas de preparação/discovery;
- testes unitários sem GPU para a nova fronteira;
- manter resolução independente do current working directory;
- atualizar dependências e documentação.

## Não inclui

- `Application` genérica;
- loop principal;
- Camera/presentation coordination;
- RenderSnapshot;
- Vulkan lifecycle;
- Windows CI;
- alterações no formato `.lvl`.

## Dependências

```text
RuntimePaths ─────┐
CampaignLoader ────┼→ RuntimeBootstrap → main.cpp
CampaignID ───────┘
```

**Produz para:** `main.cpp`, `GameSession` construction.

**Consumidores afetados:** `main.cpp`, novos testes de bootstrap.

**Documentos afetados:** `ARCHITECTURE.md`, `ROADMAP.md`, `TECH_DEBT.md`, este WP.

## Invariantes

- nenhuma dependência de current working directory;
- nenhuma ownership de Vulkan/GLFW;
- campaign paths são derivados de `RuntimePaths`;
- uma campanha vazia/missing não é silenciosamente transformada numa campanha válida;
- `CampaignID` continua determinístico para o mesmo conjunto de levels;
- o bootstrap não duplica lógica existente nos serviços.

## Validação

- testes unitários da nova fronteira;
- testes existentes de `RuntimePaths` e `CampaignLoader`;
- testes Linux/headless;
- ASan/UBSan;
- comparação de comportamento de `main.cpp` antes/depois em resolução de paths/campaign.

## Definition of Ready

- [x] predecessor `LevelData` integrado em `main`;
- [x] `RuntimePaths` investigado;
- [x] `CampaignLoader` investigado;
- [x] `CampaignID` identificado como dependência de bootstrap;
- [x] ownership que permanece em `main.cpp` delimitado;
- [x] escopo negativo definido.

## Definition of Done

- [ ] fronteira possui responsabilidade observável e coesa;
- [ ] `main.cpp` deixa de compor diretamente paths/campaign identity;
- [ ] sem ownership Vulkan/GLFW na nova classe;
- [ ] testes novos + existentes verdes;
- [ ] ASan/UBSan verdes;
- [ ] documentação normativa sincronizada;
- [ ] PR integrada.

## Fecho esperado

Este WP é experimental dentro do Gate 9.6: se a extração demonstrar apenas movimentação de código sem redução real de coupling, deve ser revertida e a composição permanece em `main.cpp`.
