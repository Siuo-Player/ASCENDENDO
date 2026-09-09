# ASCENDENDO — Snapshot operacional — 09/09/2026

## Baseline exato

`main`: `621eeb2c252585a025e6494dfb1f6c438ffa3911`

Este snapshot reflete o `main` após PR #247 (adjacência diagonal) e PR #248 (cobertura das quatro orientações diagonais).

## Estado técnico

### Editor

O editor possui edição de entidades, save staged, validação síncrona e validação assíncrona sobre snapshots imutáveis com generation/STALE. A integração direta com o validator físico/campanha completo permanece separada.

### Apresentação semântica

A cadeia é:

```text
LevelData
→ PlatformPresentationRasterizer
→ RegionCell
→ 8-neighbour + cross-region signature
→ RenderSnapshot
→ WorldRenderer
```

O compositor preserva vizinhança cardinal e diagonal entre regiões modulares. Os contactos diagonais corner-only são recíprocos e tolerantes a pequeno drift sem alterar gameplay geometry.

### Runtime/playability

Existe cobertura runtime-grounded de start → jump → fixed timestep → collision → landing através do Player/PhysicsWorld/Level reais.

### Assets

Aprovação humana, provenance e elegibilidade técnica continuam separadas. Não existe promoção legítima de asset externo sem ficheiro binário exato, identidade verificável e gates correspondentes.

## CI

### Artifact finalization

O incidente `FinalizeArtifact 403` foi resolvido por rerun do mesmo job sem alteração de permissões ou código.

O rerun produziu o artifact `deterministic-capture-level-1-16-9` com ID `10115025267`; os restantes artifacts da matriz também ficaram disponíveis.

Conclusão: este caso é classificado como falha transitória/intermediária do serviço de artifacts.

Não foram introduzidas:

- permissões `actions: write` como pseudo-correção;
- colisão de nomes inexistente;
- `retry-max-attempts`, que não é input suportado pela action;
- wrapper externo de retry sem necessidade demonstrada.

Relatório detalhado: `ARTIFACT_FINALIZATION_INCIDENT_2026-09-09.md`.

### Actions

Os workflows Linux foram alinhados com `actions/checkout@v6`; deterministic capture usa `actions/upload-artifact@v6`. A atualização elimina a deriva observada entre workflows e os avisos relacionados com as actions v4 em runners atuais.

### Sanitizers

A execução ASan/UBSan instrumentada do game code existe e está válida em PRs, mas ainda não cobre diretamente o caminho de `push` para `main` porque o job está condicionado pelo evento.

Isto é dívida de CI separada, não parte da resolução do artifact 403.

## Próximos gates

1. **Fase 1.2:** verificar/reforçar swapchain recreation fail-closed.
2. **Fase 1.3:** consolidar camera bounds e estabilidade subpixel.
3. **Fase 1.4:** validar diretamente os mapas `NaoValidados`.
4. Só depois: profiling e otimizações da Fase 2.
5. Só depois da estabilidade estrutural: expansão de campanha/editor e validação visual humana.
6. Audio permanece workstream independente.

## O que não está concluído

- promoção de binários externos concretos para runtime;
- validação visual humana completa;
- props/environment curados em produção;
- validação física/campaign adicional que não esteja já coberta pelo tooling existente;
- sanitizer coverage equivalente no `push` de `main`;
- release hardening.

## Gates permanentes

```text
main = verdade operacional
CI ≠ qualidade visual
capture ≠ playtest
metadata ≠ binary
CC0 ≠ approval
semantic validation ≠ full physical validation
```
