# Plano da branch atual

**Branch:** `feat/source-size-enforcement`

**Bloco do roadmap:** 9.6 Base Engineering Gate

**Work Package:** C — source-size enforcement + primeiro alvo de modularidade

## Objetivo

Fazer o checker de tamanho cumprir a política normativa baseada em linhas e resolver o primeiro erro real desencadeado pelo novo gate sem divisão artificial.

## Documentos obrigatórios

Consultar durante esta branch:

- `docs/DEVELOPMENT_PROTOCOL.md`
- `docs/PROJECT_MANAGEMENT.md`
- `docs/ROADMAP.md`
- `docs/ARCHITECTURE.md`
- `docs/TECH_DEBT.md`
- `docs/CODE_SIZE.md`

## Dependências

**Depende de:**

- PR #21 integrada em `main`;
- política de source-size documentada;
- decisão explícita de dividir por responsabilidade/cohesion.

**Produz para:**

- CI com enforcement real de linhas;
- work packages seguintes de `SpriteRenderer`, `main.cpp` e testes;
- gate de engenharia de base.

## WBS

```text
9.6 Base Engineering Gate
└── C — source-size enforcement
    ├── migrar checker KiB → linhas
    ├── incluir main.cpp
    ├── preservar warning/error semantics
    ├── identificar ficheiros que falham o novo gate
    ├── decompor FontRenderer por responsabilidade
    └── validar build/tests/source-size
```

## Resultado esperado da decomposição

`FontRenderer.cpp` separa:

- baking/estado de texto;
- upload Vulkan do atlas (`FontRendererGpu.cpp`);
- descriptor setup e desenho.

A divisão não deve alterar a API pública nem criar classes artificiais.

## Riscos

- esquecer o novo `.cpp` na build;
- introduzir duplicate symbols relacionados com `stb_truetype`;
- alterar lifecycle/ownership do atlas Vulkan;
- fazer uma divisão apenas para satisfazer a métrica.

## Validação

```text
python3 Development/Tools/check_source_sizes.py
make game
make tests-verbose
```

O CI deve executar o mesmo checker antes do build.

## Definition of Done

- [x] checker usa linhas;
- [x] `main.cpp` é incluído;
- [x] política de 300/400 é refletida no checker;
- [x] `FontRenderer.cpp` deixou de ser monólito acima do hard limit;
- [x] upload Vulkan está isolado por responsabilidade;
- [ ] CI verde;
- [ ] build/testes verdes;
- [ ] documentação de resultado atualizada.

## Não entra nesta branch

- `RenderSnapshot` geral;
- `Application`/`GameStateMachine`;
- Campaign Editor;
- save/import/share;
- otimizações sem profiling.

## Próximo work package

Depois de integrar esta branch: **D — modularidade**, começando por `SpriteRenderer.cpp`, seguido de `main.cpp` e dos ficheiros de teste que permanecerem acima do limite ou warning com motivo arquitetural claro.
