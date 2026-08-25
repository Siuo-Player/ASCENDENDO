# Plano da branch atual

**Branch:** `feat/9-8-remove-legacy-renderer`

**Bloco do roadmap:** 9.6 — hardening/consolidação arquitetural

**Objetivo:** eliminar o renderer legado depois da migração para `RendererFacade`, retirar dependências de compatibilidade que já não têm consumidores legítimos e consolidar a documentação de arquitetura/gestão do projeto.

## WBS desta branch

```text
9.6 — consolidação de presentation
└── 9.8 — remover renderer legado
    ├── 9.8.1 — extrair GameState do renderer antigo
    ├── 9.8.2 — migrar testes/consumidores para RendererFacade
    ├── 9.8.3 — remover dependências de Renderer.h
    ├── 9.8.4 — retirar Renderer.cpp/.h quando não existirem referências
    ├── 9.8.5 — atualizar documentação de arquitetura/planeamento
    └── 9.8.6 — validação completa e PR
```

## Dependências

**Depende de:**

- 9.6 base hardening integrado;
- `RendererCore` estável;
- `RendererFacade` + passes especializados;
- `RendererFacadeAdapter` operacional durante a migração;
- testes de `RendererCore` existentes.

**Produz para:**

- futura remoção do adapter;
- `RenderSnapshot` geral;
- extração progressiva do loop principal;
- arquitetura de presentation sem API legada.

**Consumidores afetados:**

- `main.cpp` / adapter;
- integration tests do renderer;
- Makefile (por wildcard, nenhum source manual adicional é necessário);
- documentação de arquitetura/migração.

## Estado herdado — concluído

- Fases 1–8.
- 9.1–9.5.
- primeira tranche 9.6: input, timestep, viewport.
- `RendererCore`, `ShapeRenderer`, `WorldRenderer`, `UiRenderer`, `EditorRenderer` e `RendererFacade`.
- 9.7: lifecycle seguro de falhas/recriação no `RendererCore`, integrado via PR #14.

## Trabalho desta branch

### 9.8.1 — GameState

`GameState` deve ter header próprio e não depender do renderer histórico.

### 9.8.2 — consumidores

Todos os testes e consumidores legítimos devem usar `RendererFacade`/adapter durante a transição. Nenhum novo consumidor deve ser ligado a `Renderer` legado.

### 9.8.3 — dependências

Remover includes e APIs que apenas existem para suportar o renderer antigo.

### 9.8.4 — legado

Eliminar `Game/Graphics/Renderer.cpp` e `Game/Graphics/Renderer.h` apenas depois de confirmar que não existem referências de código, testes ou documentação operacional.

### 9.8.5 — documentação

Atualizar, nesta branch, quando afetados:

- `docs/ROADMAP.md`;
- `docs/ARCHITECTURE.md`;
- `docs/TECH_DEBT.md`;
- `docs/RESEARCH_INDEX.md`;
- `docs/TECHNICAL_REFERENCES.md`;
- `docs/PROJECT_MANAGEMENT.md`.

## Critério de saída

A branch só pode abrir PR quando:

```text
Renderer legado sem consumidores
+ GameState desacoplado
+ testes migrados
+ dependências antigas removidas
+ documentação atualizada
+ testes/CI validados
```

## Definition of Done

- Não existem includes de `Graphics/Renderer.h` fora de ficheiros explicitamente históricos.
- `Renderer.cpp`/`.h` já não são necessários para compilar produto ou testes.
- A nova stack continua a ser a única rota de renderização do runtime.
- Alterações arquiteturais relevantes estão documentadas com rationale e consequências.
- O work package permanece rastreável a este bloco do roadmap.

## Não entra nesta branch

- novo gameplay;
- novas mecânicas de nível;
- save/import final;
- networking/web sharing;
- nova arquitetura genérica de engine;
- otimizações prematuras sem profiling.

## Próximo work package após o merge

A prioridade seguinte é **9.6 P1 — eliminar o adapter de migração**, seguindo para `RenderSnapshot` geral apenas depois de a nova fachada ser a API direta do runtime.

Consultar `docs/PROJECT_MANAGEMENT.md` para a Definition of Ready/Done e regras de dependency awareness.