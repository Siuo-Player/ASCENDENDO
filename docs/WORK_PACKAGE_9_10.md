# Work Package 9.10 — RenderSnapshot geral

## Objetivo
Separar o domínio da presentation: `RendererFacade` e passes devem consumir dados próprios de apresentação, não `Player`, `Level` ou `GameState`.

## WBS

```text
9.10 — RenderSnapshot
├── 9.10.1 — contrato de dados de apresentação ✅
├── 9.10.2 — testes de value semantics ✅
├── 9.10.3 — builder no boundary aplicação → presentation
├── 9.10.4 — migrar WorldRenderer
├── 9.10.5 — migrar RendererFacade
├── 9.10.6 — remover overloads com domínio
└── 9.10.7 — equivalência + profiling + documentação
```

## Dependências

- 9.9 integrada: `RendererFacade` é a API pública de presentation;
- `EditorRenderSnapshot` existente para o editor;
- `WorldRenderer` atualmente recebe `Player`/`Level` diretamente.

## Decisão arquitetural

`RenderSnapshot` contém apenas dados necessários para desenhar o frame:

- bounding box do jogador;
- direção do jogador;
- retângulos das plataformas;
- presença e bounding box da flag.

Não deve conter ponteiros para objetos de domínio, referências Vulkan ou estado mutável de gameplay.

## Risco principal

Uma migração incompleta pode duplicar regras de apresentação ou introduzir divergência entre a renderização antiga e a nova. Por isso a conversão deve acontecer num boundary explícito e a saída exige testes de equivalência.

## Critério de saída

```text
RendererFacade recebe RenderSnapshot
+ WorldRenderer recebe RenderSnapshot
+ não existem parâmetros Player/Level em presentation
+ GameState é convertido antes da presentation
+ testes cobrem equivalência do caminho antigo
+ documentação arquitetural atualizada
```

## Estado

Esta PR contém apenas o contrato + testes do snapshot. A migração dos passes será uma tranche separada se a revisão mostrar necessidade de dividir ainda mais a alteração.
