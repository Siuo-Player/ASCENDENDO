# ASCENDENDO — Snapshot operacional — 09/09/2026

## Baseline exato

`main`: `2b6c1e8a1f8152e224b6bf806a398bd91d0db1af`

Este snapshot inclui a documentação consolidada do produto (PR #262), o fecho formal da Foundation no Sprint 20 (PR #256) e a integração do PR #261, que expôs a seleção de nível da Campaign Editor.

## Foundation

**Sprint 20 Foundation — CLOSED.**

A decisão de fecho foi documentada em PR #256. Os gates declarados passaram e a auditoria camera/viewport não encontrou uma falha operacional que justificasse nova alteração de contrato. A Foundation não deve voltar a ser usada como contentor para trabalho posterior sem nova evidência ou requisito.

## Estado técnico atual

### Editor de níveis

O editor possui edição de entidades, save staged, validação síncrona e validação assíncrona sobre snapshots imutáveis com generation/STALE.

### Campaign Editor

O modelo canónico de campanha suporta carregamento, seleção, reordenação, validação de referências e persistência de `campaign.txt`. A identidade do nível seleccionado é preservada através das operações já cobertas por regressões. PR #261 integrou a consulta null-safe do nível seleccionado no documento.

**Próximo passo:** consumir este modelo no fluxo visual/session real do editor.

### Apresentação semântica

A cadeia permanece:

```text
LevelData
→ PlatformPresentationRasterizer
→ RegionCell
→ 8-neighbour + cross-region signature
→ RenderSnapshot
→ WorldRenderer
```

A adjacência cardinal e diagonal cross-region e a recreation transacional de swapchain estão integradas e cobertas.

### Runtime/playability

Existe cobertura runtime-grounded para start → jump → fixed timestep → collision → landing através do Player/PhysicsWorld/Level reais.

### Assets

Aprovação humana, provenance e elegibilidade técnica continuam separadas. Nenhum asset externo deve ser promovido sem ficheiro binário exato, identidade verificável e gates correspondentes.

## CI

O `main` pós-PR #262 passou o workflow `Tests` e `Windows` em push. O PR #261, agora integrado, teve Linux normal, Linux ASan/UBSan, Windows e deterministic capture verdes antes do merge.

O incidente `FinalizeArtifact 403` continua registado como incidente operacional resolvido por rerun, sem alteração de permissões como pseudo-correção.

Sanitizer coverage específica no caminho de `push` para `main` continua uma tranche de CI separada.

## Sprint 21 — estado

**OPEN — Campaign Editor integration**

Objetivo: fazer o modelo 9.6 ser consumido pelo fluxo visual real do editor, mantendo `campaign.txt` como autoridade e sem duplicar semântica de campanha.

Tracking issue: **#263**  
Work package: `docs/05-work-packages/SPRINT_21_CAMPAIGN_EDITOR_INTEGRATION_2026-09-09.md`

## Próximos gates

1. Integração Campaign Editor → UI/session real.
2. Testes da boundary visual para seleção, reorder, load/save e failed-load preservation.
3. Deterministic evidence aplicável à nova integração.
4. Sanitizers/CI hardening no caminho de `main`, em tranche separada.
5. Só depois: profiling/performance e os restantes workstreams de produto.

## O que permanece explicitamente posterior

- seam de injeção de falhas Vulkan;
- promoção de binários externos concretos;
- validação visual humana completa;
- props/environment curados em produção;
- release hardening;
- funcionalidades online/comunidade;
- propriedades de replay anteriormente adiadas sem requisito novo.

## Gates permanentes

```text
main = verdade operacional
CI ≠ qualidade visual
capture ≠ playtest
metadata ≠ binary
CC0 ≠ approval
semantic validation ≠ full physical validation
Foundation ≠ backlog permanente
```
