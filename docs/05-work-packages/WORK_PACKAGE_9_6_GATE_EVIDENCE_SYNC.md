# Work Package — 9.6 gate evidence/documentation sync

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `Infrastructure / Project Management`  
**Work Package:** `9.6 Gate Evidence Sync`  
**Branch:** `docs/9-6-gate-evidence-sync`  
**PR:** `<to be created>`

## Objetivo

Synchronize the normative project documentation with the state reached after PR #70 and remove a contradiction in the source-size policy documentation before the next 9.6 development tranche.

## Escopo

### Inclui

- record ASan/UBSan CI as completed Linux evidence after PR #70;
- update RoadMap and technical debt status without falsely closing the 9.6 Gate;
- align `DEVELOPMENT_PROTOCOL.md` with the current KiB-based source-size policy;
- preserve the distinction between observed evidence and remaining Windows/hardware evidence;
- keep the next architectural work package (`main.cpp` decomposition) explicit.

### Não inclui

- code/runtime changes;
- Windows CI implementation;
- Vulkan lifecycle changes;
- `main.cpp` decomposition;
- RenderSnapshot migration;
- changing the normative source-size thresholds.

## Dependências

### Depende de

- PR #70 merged into `main`;
- GitHub Actions evidence for `Tests` and `Sanitizers`;
- `docs/CODE_SIZE.md` as the current source-size policy;
- `docs/ROADMAP.md`, `docs/TECH_DEBT.md`, `docs/DEVELOPMENT_PROTOCOL.md` and `docs/ARCHITECTURE.md`.

### Produz para

- next 9.6 architectural tranche for `main.cpp` decomposition;
- future Base Engineering Gate review;
- future Windows CI work package.

### Consumidores afetados

- roadmap readers;
- future AI/human maintainers;
- CI/build documentation;
- main-loop decomposition work package.

### Dependências de validação

- merged PR #70;
- successful CI runs associated with sanitizer implementation;
- documentation consistency review.

## Decisões arquiteturais

**Problema/contexto:** `DEVELOPMENT_PROTOCOL.md` retained a historical 300/400 LOC policy while the repository's current `CODE_SIZE.md` and canonical checker use 40/48 KiB. This creates a contradiction in the normative documentation hierarchy.

**Decisão:** make `CODE_SIZE.md` + `Development/Tools/check_source_sizes.py` the operative source-size policy in the protocol, while retaining LOC as diagnostic information only.

**Alternativas consideradas:**

1. revert the repository to the old LOC policy — rejected because the current checker, tests and recent PR already establish the KiB policy;
2. leave the contradiction unresolved — rejected because a new branch would not have a reliable normative source of truth;
3. introduce a second checker/policy — rejected because it increases tooling divergence.

**Consequências:** the protocol becomes consistent with the current CI/tooling state; no source-size threshold changes are introduced.

**Condição de revisão/remoção:** revisit only if a future documented architectural decision deliberately changes the source-size metric.

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| Documentation changes accidentally imply the whole Gate is closed | baixa | alto | explicitly retain Windows, Vulkan/error-path and RenderSnapshot debt | mitigado |
| Historical references become misleading | média | médio | mark them as historical and point to the current policy | aberto |
| Next work package starts from stale status | média | médio | update RoadMap, TECH_DEBT and branch plan together | mitigado |

## Validação

### Testes automatizados

- existing PR #70 `Tests` and `Sanitizers` workflow evidence remains referenced;
- no source/build code changes are introduced by this WP.

### Validação manual

- verify documentation hierarchy has one source-size policy;
- verify 9.6 remains open for the explicitly listed P0/P1/P2 evidence.

### Profiling / métricas

- none; this is documentation synchronization.

### Failure paths

- ensure no document states that Windows CI or hardware matrix has been validated when it has not.

## Definition of Ready

- [x] objective and scope defined;
- [x] normative documents consulted;
- [x] critical dependencies identified;
- [x] exit criteria defined;
- [x] validation strategy defined;
- [x] relevant risks recorded.

## Definition of Done

- [ ] implementation/document changes completed within scope;
- [ ] documentation consistency reviewed;
- [ ] RoadMap/TECH_DEBT reflect the merged sanitizer evidence;
- [ ] protocol no longer contradicts the current source-size policy;
- [ ] next work package remains explicitly identified;
- [ ] PR ready for merge without hidden documentation work.

## Alterações durante a execução

```text
Descoberta:
A documentação normativa divergia da política já integrada no checker/CI.

Impacto:
A hierarquia documental podia levar uma nova branch a tratar LOC como gate enquanto o CI usava KiB.

Decisão tomada:
Sincronizar o protocolo com a política KiB existente, sem alterar o checker ou thresholds.

Documentos atualizados:
DEVELOPMENT_PROTOCOL.md
ROADMAP.md
TECH_DEBT.md
BRANCH_PLAN.md
```

## Evidência / referências

- PR #70 — `ci(9.6): add production ASan/UBSan coverage`;
- GitHub Actions `Tests` run `33025107053` — success;
- GitHub Actions `Sanitizers` run `33025107141` — success;
- `docs/CODE_SIZE.md` — current KiB policy;
- `Development/Tools/check_source_sizes.py` — canonical checker;
- `docs/ROADMAP.md` — execution order.

## Fecho

**Resultado:** em execução.  
**Critério de saída:** documentação hierarquicamente consistente com o estado de `main` após PR #70, sem fechar prematuramente o 9.6 Gate.  
**Dívida residual:** Windows CI, Vulkan lifecycle/queue verification, full presentation boundary and remaining 9.6 gate evidence.
