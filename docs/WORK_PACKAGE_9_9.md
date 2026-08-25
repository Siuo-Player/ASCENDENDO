# Work Package 9.9 — Remover o RendererFacadeAdapter

## Identificação

**Roadmap:** 9.6 P1 — novas fronteiras arquiteturais  
**Subsistema:** Presentation / Runtime integration  
**Work Package:** 9.6-P1.7 — remover adapter de migração  
**Branch:** `feat/9-9-remove-renderer-adapter`  
**PR:** a criar no fecho do work package

## Objetivo

Fazer `RendererFacade` passar a ser a API usada diretamente pelo runtime, eliminando a camada de compatibilidade `RendererFacadeAdapter`.

A remoção deve reduzir superfície arquitetural sem alterar comportamento funcional do renderer.

## Escopo

### Inclui

- migrar `main.cpp` e testes que ainda usam `RendererFacadeAdapter`;
- preservar a atualização do `EditorRenderSnapshot` sem esconder essa responsabilidade dentro de um adapter;
- eliminar `RendererFacadeAdapter.h/.cpp` quando não existirem consumidores;
- atualizar documentação e matriz de migração;
- adicionar testes que garantam a ligação direta runtime → `RendererFacade`.

### Não inclui

- `RenderSnapshot` geral;
- extração de `Application` / `GameStateMachine`;
- alteração de passes Vulkan;
- alterações de física ou gameplay;
- novas features do editor.

## Dependências

### Depende de

- PR #15 / 9.8 integrada;
- `RendererFacade` funcional e testada;
- `EditorRenderSnapshot` existente;
- testes de integração do renderer;
- contrato `GameState` separado do renderer legado.

### Produz para

- 9.6 P1.9 — `RenderSnapshot` geral;
- separação definitiva entre presentation e runtime;
- posterior extração do loop principal.

### Consumidores afetados

- `main.cpp`;
- testes de integração do renderer;
- documentação `RENDERER_MIGRATION.md`, `ARCHITECTURE.md`, `ROADMAP.md` e `TECH_DEBT.md`;
- Makefile apenas por descoberta automática dos fontes.

## Decisão arquitetural

O adapter foi útil durante a migração porque permitiu substituir o renderer antigo sem alterar imediatamente todos os consumidores.

Agora essa razão deixou de existir: o renderer antigo foi removido e `RendererFacade` é a única implementação válida.

Manter o adapter criaria uma camada sem função arquitetural própria e aumentaria o coupling conceitual entre API antiga e nova.

```text
Antes
runtime → RendererFacadeAdapter → RendererFacade

Depois
runtime → RendererFacade
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| perda da atualização do snapshot do editor | média | alto | mover explicitamente `renderSnapshot()` para o ponto de preparação do frame | aberto |
| divergência de `GameState` → `RenderState` | baixa | médio | testar todos os estados | aberto |
| consumidor oculto do adapter | baixa | alto | pesquisa global + build/testes | aberto |
| aumento temporário de responsabilidade no `main.cpp` | média | médio | limitar mudança ao cut-over; não extrair novas responsabilidades nesta branch | aberto |

## Validação

### Testes automatizados

- integração `RendererFacade` para `MENU`, `PLAYING`, `PAUSED`, `CREDITS` e `EDITOR`;
- teste de `EditorRenderSnapshot` em cada frame de editor relevante;
- pesquisa sem referências a `RendererFacadeAdapter` depois da remoção;
- build de `game` e `tests`.

### Failure paths

- `RendererFacade::init()` falha;
- `drawFrame()` devolve falha;
- snapshot do editor não está disponível;
- retorno `OUT_OF_DATE`/`SUBOPTIMAL` continua tratado pelo `RendererCore`.

### CI

O workflow Linux headless deve executar o build/teste. Atualmente não estão a ser publicados checks para os commits do repositório; esta ausência deve ser registada como limitação de validação, não como sucesso.

## Definition of Ready

- [x] 9.8 integrada;
- [x] renderer legado removido;
- [x] dependência do adapter conhecida;
- [x] objetivo e escopo definidos;
- [x] estratégia de validação definida;
- [x] riscos registados;
- [x] documentação de processo consultada.

## Definition of Done

- [ ] nenhum consumidor legítimo de `RendererFacadeAdapter`;
- [ ] `main.cpp` usa `RendererFacade` diretamente;
- [ ] testes usam `RendererFacade` diretamente;
- [ ] adapter eliminado;
- [ ] documentação normativa atualizada;
- [ ] testes/build executados com sucesso ou falhas registadas explicitamente;
- [ ] PR pronta para merge sem trabalho essencial oculto.

## Evidência

A ordem deriva da arquitetura atual do projeto e do princípio de reduzir dependências reais antes de introduzir uma nova fronteira de dados (`RenderSnapshot`). O racional de work-package e dependency awareness está formalizado em `docs/PROJECT_MANAGEMENT.md`.
