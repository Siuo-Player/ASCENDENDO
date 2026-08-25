# Plano da branch atual

**Branch:** `feat/9-9-remove-renderer-adapter`

**Bloco do roadmap:** 9.6 P1 — novas fronteiras arquiteturais

**Work Package:** 9.6-P1.7 — eliminar o `RendererFacadeAdapter`

## Objetivo

Fazer `RendererFacade` passar a ser a API usada diretamente pelo runtime, removendo a última camada de compatibilidade da migração do renderer.

## Documentos obrigatórios

Antes e durante esta branch consultar:

- `docs/PROJECT_MANAGEMENT.md`
- `docs/WORK_PACKAGE_9_9.md`
- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/TECH_DEBT.md`
- `docs/RENDERER_MIGRATION.md`

## Dependências

**Depende de:**

- 9.8 integrada;
- `RendererCore`, passes e `RendererFacade` estáveis;
- `GameState` independente do renderer legado;
- testes existentes da nova stack.

**Produz para:**

- `RenderSnapshot` geral;
- apresentação sem adapters;
- futura extração de `Application`/loop principal.

## WBS

```text
9.6 P1
└── Presentation
    └── 9.6-P1.7 remover adapter
        ├── migrar runtime
        ├── migrar testes
        ├── preservar EditorRenderSnapshot
        ├── remover RendererFacadeAdapter
        ├── atualizar documentação
        └── validar build/testes/failure paths
```

## Riscos críticos

- perder a atualização do `EditorRenderSnapshot` durante o cut-over;
- deixar consumidores ocultos do adapter;
- criar acoplamento adicional no `main.cpp`.

Mitigação: mover explicitamente a preparação do snapshot para o ponto de apresentação, procurar referências globalmente e não extrair novas responsabilidades nesta branch.

## Definition of Ready

- [x] objetivo definido;
- [x] escopo incluído/excluído definido;
- [x] dependências conhecidas;
- [x] riscos identificados;
- [x] estratégia de validação definida;
- [x] documentação de processo consultada.

## Critério de saída

```text
main/testes usam RendererFacade diretamente
+ nenhum consumidor do adapter
+ adapter removido
+ EditorRenderSnapshot preservado
+ testes/build executados ou falhas registadas
+ documentação atualizada
```

## Não entra nesta branch

- `RenderSnapshot` geral;
- Campaign Editor;
- save/import;
- novas mecânicas;
- networking;
- otimizações sem profiling.

## Próximo work package

Depois de integrar esta branch: **9.6 P1.9 — `RenderSnapshot` geral**, transformando a apresentação numa fronteira de dados explícita para gameplay, UI e editor.