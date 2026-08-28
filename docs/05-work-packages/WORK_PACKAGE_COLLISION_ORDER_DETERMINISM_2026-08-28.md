# Work Package — Collision-order determinism

## Identificação

**Roadmap:** 9.6 — Base Engineering Gate  
**Subsistema:** Runtime / Physics  
**Work Package:** 9.6 — Collision-order determinism  
**Branch:** `test/9-6-collision-order-permutation-20260828-v2`  
**PR:** `#100`

## Objetivo

Determinar por evidência se `Level::resolveCollision()` produz o mesmo estado lógico quando o mesmo conjunto de contactos é apresentado em ordens diferentes.

## Escopo

### Inclui

- teste com múltiplas plataformas potencialmente sobrepostas ao mesmo corpo;
- comparação de duas ordens de armazenamento para o mesmo conjunto geométrico;
- comparação de posição, velocidade e estado grounded;
- decisão documentada limitada ao caso exercitado.

### Não inclui

- reescrita geral da física;
- alteração de coeficientes/regras de resposta;
- otimização espacial;
- tuning de gameplay;
- RenderSnapshot.

## Dependências

### Depende de

- `Level::resolveCollision()`;
- `PhysicsBody` / `PhysicsWorld`;
- `Tests/Unit/test_level_collision.cpp`;
- Gate 9.6 collision-order gap.

### Produz para

- Gate 9.6 — decisão sobre determinismo de contactos;
- eventual WP de correção, apenas se uma investigação futura demonstrar divergência adicional relevante.

### Consumidores afetados

- `Level`;
- testes de colisão;
- documentação de física/arquitetura.

### Dependências de validação

- Linux/Clang normal;
- ASan/UBSan;
- Windows CI.

## Decisões arquiteturais

```text
Problema/contexto:
resolveCollision() percorre plataformas por ordem de armazenamento e aplica mutações imediatamente.

Decisão:
Investigar a propriedade empiricamente antes de alterar a implementação.

Alternativas consideradas:
- assumir ordem irrelevante: rejeitada por falta de evidência;
- introduzir imediatamente um solver de contactos novo: rejeitada por scope excessivo sem contraexemplo;
- teste de permutação: escolhida como intervenção mínima.

Resultado:
O cenário exercitado não apresentou divergência observável entre A→B e B→A.
A conclusão permanece limitada ao cenário testado; não é assumida permutation invariance universal.

Condição de reabertura:
Um requisito mais forte ou um counterexample reproduzível em configuração adicional justifica novo WP antes de alterar o solver.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| caso testado não representar todos os contactos reais | média | alto | limitar a conclusão ao domínio exercitado | aberto |
| resultado positivo ser confundido com prova universal | média | alto | conclusão explicitamente limitada | mitigado |
| investigação evoluir para rewrite sem finding | baixa | alto | nenhum código de produção foi alterado | mitigado |

## Validação

### Testes automatizados

- executar duas ordens para o mesmo conjunto de plataformas;
- comparar posição, velocidade e grounded.

### Resultado observado

- Linux / Clang / C++20 / Headless Vulkan: `success`;
- Linux / Clang / ASan + UBSan / Headless Vulkan: `success`;
- Windows / Clang / C++20: `success`;
- Linux: 229/229 test cases e 1311/1311 assertions;
- Windows: evidence artifact produzido.

### Interpretação

Não foi observada divergência no conjunto de contactos exercitado. Isso demonstra apenas a invariância observada para esse caso, não uma propriedade universal sobre toda a gama de contactos possíveis.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [x] teste de permutação integrado;
- [x] resultado observado documentado em `docs/AUDITS/2026-08-28-collision-order-result.md`;
- [x] correção de produção não necessária para o caso exercitado;
- [x] testes/CI relevantes passaram;
- [x] roadmap/gate matrix reconciliados;
- [x] dívida residual classificada como alegação global ainda não demonstrada.

## Alterações durante a execução

A branch foi reconstruída sobre a `main` atual para evitar a baseline stale de #97. O resultado positivo não alterou a implementação de produção.

## Evidência / referências

- PR #100;
- PR #101;
- `docs/AUDITS/2026-08-28-collision-order-revalidation.md`;
- `docs/AUDITS/2026-08-28-collision-order-result.md`;
- `Game/Logic/Level.cpp`;
- `Tests/Unit/test_level_collision.cpp`;
- `docs/ROADMAP.md`;
- `docs/TECH_DEBT.md`.

## Fecho

**Resultado:** `validado — cenário específico`  
**Propriedade universal:** `não demonstrada`  
**Produção:** sem alteração  
**Próxima decisão:** seguir para a evidência residual de Vulkan/error-path, salvo novo requisito ou counterexample de collision-order.
