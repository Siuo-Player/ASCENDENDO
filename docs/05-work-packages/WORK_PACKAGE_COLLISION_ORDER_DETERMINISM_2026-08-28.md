# Work Package — Collision-order determinism

## Identificação

**Roadmap:** 9.6 — Base Engineering Gate  
**Subsistema:** Runtime / Physics  
**Work Package:** 9.6 — Collision-order determinism  
**Branch:** `test/9-6-collision-order-permutation-20260828-v2`  
**PR:** pending

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
- eventual WP de correção, apenas se a experiência demonstrar ordem-dependência.

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

Condição de revisão:
Se existir divergência observável, desenhar primeiro o contrato de resolução antes de alterar o algoritmo.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| caso testado não representar todos os contactos reais | média | alto | limitar a conclusão ao domínio exercitado | aberto |
| resultado positivo ser confundido com prova universal | média | alto | conclusão explicitamente limitada | mitigado |
| investigação evoluir para rewrite sem finding | média | alto | nenhum código de produção antes de contraexemplo/decisão | mitigado |

## Validação

### Testes automatizados

- executar duas ordens para o mesmo conjunto de plataformas;
- comparar posição, velocidade e grounded.

### Failure paths

- contacto sem colisão;
- dois contactos sobrepostos;
- múltiplas mutações na mesma etapa.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [ ] teste de permutação integrado;
- [ ] resultado observado documentado;
- [ ] correção implementada apenas se necessária;
- [ ] testes/CI relevantes passam;
- [ ] Gate matrix atualizada;
- [ ] dívida residual classificada.

## Alterações durante a execução

A branch é deliberadamente baseada na `main` atual (`5edab5b...`). A PR anterior #97 foi fechada por stale baseline; este WP evita carregar a história de branch antiga.

## Evidência / referências

- `docs/AUDITS/2026-08-28-collision-order-investigation.md` (estudo original);
- `Game/Logic/Level.cpp`;
- `Tests/Unit/test_level_collision.cpp`;
- `docs/ROADMAP.md`;
- `docs/TECH_DEBT.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-next-9-6-runtime-bootstrap-gate.md`.

## Fecho

**Resultado:** `em investigação`  
**Critério de saída:** resultado reproduzível de uma propriedade de permutação ou contraexemplo concreto  
**Dívida residual:** sem alteração de produção antes da decisão baseada em evidência
