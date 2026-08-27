# Work Package — 9.6 gate evidence/documentation sync

## Identificação

**Roadmap:** `9.6 Base Engineering Gate`  
**Subsystem:** `Infrastructure / Project Management`  
**Work Package:** `9.6 Gate Evidence Sync`  
**Branch:** `docs/9-6-gate-evidence-sync`  
**PR:** `#71`

## Objetivo

Synchronize the normative project documentation with the state reached after PR #70 and remove contradictions in the source-size and renderer migration documentation before the next 9.6 development tranche.

## Escopo

### Inclui

- record ASan/UBSan CI as completed Linux evidence after PR #70;
- update RoadMap and technical debt status without falsely closing the 9.6 Gate;
- align `DEVELOPMENT_PROTOCOL.md` with the current KiB-based source-size policy;
- remove the stale claim that PR #20 is an active RenderSnapshot implementation record;
- preserve the distinction between observed evidence and remaining Windows/hardware/Vulkan evidence;
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
- `docs/ROADMAP.md`, `docs/TECH_DEBT.md`, `docs/DEVELOPMENT_PROTOCOL.md`, `docs/BRANCH_PLAN.md` and `docs/ARCHITECTURE.md`.

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
- documentation consistency review;
- PR #71 review/merge validation.

## Decisões arquiteturais

**Problema/contexto:** `DEVELOPMENT_PROTOCOL.md` retained a historical 300/400 LOC policy while the repository's current `CODE_SIZE.md` and canonical checker use 40/48 KiB. The protocol also described PR #20 as the active RenderSnapshot implementation record although current architecture documentation marks it closed/superseded.

**Decisão:** make `CODE_SIZE.md` + `Development/Tools/check_source_sizes.py` the operative source-size policy in the protocol, while retaining LOC as diagnostic information only; describe PR #20 as historical and use current architecture/work-package records for the active migration state.

**Alternativas consideradas:**

1. revert the repository to the old LOC policy — rejected because the current checker, tests and recent PR already establish the KiB policy;
2. leave the contradictions unresolved — rejected because a new branch would not have a reliable normative source of truth;
3. introduce a second checker/policy — rejected because it increases tooling divergence.

**Consequências:** the protocol and roadmap now describe the same operational policy and current renderer migration state; no source-size threshold or runtime behavior changes are introduced.

**Condição de revisão/remoção:** revisit only if a future documented architectural decision deliberately changes the source-size metric or RenderSnapshot migration strategy.

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| Documentation changes accidentally imply the whole Gate is closed | baixa | alto | explicitly retain Windows, Vulkan/error-path and RenderSnapshot debt | mitigado |
| Historical references become misleading | média | médio | replace stale active-status claims with historical wording and current sources | mitigado |
| Next work package starts from stale status | média | médio | update RoadMap, TECH_DEBT and branch plan together | mitigado |

## Validação

### Testes automatizados

- PR #70 `Tests` and `Sanitizers` workflow evidence remains valid;
- no source/build code changes are introduced by this WP.

### Validação manual

- verify documentation hierarchy has one source-size policy;
- verify 9.6 remains open for the explicitly listed P0/P1/P2 evidence;
- verify no current document claims Windows or hardware capability validation that has not occurred.

### Profiling / métricas

- none; this is documentation synchronization.

### Failure paths

- avoid claiming CI/root-cause knowledge not supported by logs;
- avoid treating the merged sanitizer evidence as proof of Windows/hardware coverage.

## Definition of Ready

- [x] objective and scope defined;
- [x] normative documents consulted;
- [x] critical dependencies identified;
- [x] exit criteria defined;
- [x] validation strategy defined;
- [x] relevant risks recorded.

## Definition of Done

- [x] implementation/document changes completed within scope;
- [x] documentation consistency reviewed;
- [x] RoadMap/TECH_DEBT reflect the merged sanitizer evidence;
- [x] protocol no longer contradicts the current source-size policy;
- [x] stale PR #20 active-status claim removed from the protocol;
- [x] next work package remains explicitly identified;
- [x] PR #71 created and ready for merge without hidden documentation work.

## Alterações durante a execução

```text
Descoberta:
A documentação normativa divergia da política já integrada no checker/CI e continha uma referência histórica incorreta sobre PR #20.

Impacto:
A hierarquia documental podia levar uma nova branch a tratar LOC como gate enquanto o CI usava KiB, ou a procurar um PR histórico como se ainda fosse a implementação ativa.

Decisão tomada:
Sincronizar o protocolo, roadmap, dívida e branch plan com a política/evidência atuais sem alterar runtime.

Documentos atualizados:
DEVELOPMENT_PROTOCOL.md
ROADMAP.md
TECH_DEBT.md
BRANCH_PLAN.md
WORK_PACKAGE_9_6_GATE_EVIDENCE_SYNC.md
```

## Evidência / referências

- PR #70 — `ci(9.6): add production ASan/UBSan coverage`;
- GitHub Actions `Tests` run `33025107053` — success;
- GitHub Actions `Sanitizers` run `33025107141` — success;
- `.github/workflows/sanitizers.yml`;
- `docs/CODE_SIZE.md` — current KiB policy;
- `Development/Tools/check_source_sizes.py` — canonical checker;
- `docs/ARCHITECTURE.md` — current renderer migration state;
- `docs/ROADMAP.md` — execution order.

## Fecho

**Resultado:** concluído documentalmente; aguarda integração da PR #71.  
**Critério de saída:** documentação hierarquicamente consistente com o estado de `main` após PR #70, sem fechar prematuramente o 9.6 Gate.  
**Dívida residual:** Windows CI, Vulkan lifecycle/queue verification, full presentation boundary, replay/property/failure-path evidence and remaining 9.6 gate evidence.
