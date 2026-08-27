# Work Package — Collision-order determinism

## Identificação

**Roadmap:** 9.6 — Base Engineering Gate  
**Subsistema:** Runtime / Physics  
**Work Package:** 9.6 — Collision-order determinism  
**Branch:** `docs/9-6-collision-order-investigation-20260828`  
**PR:** pending

## Objetivo

Determinar por evidência se `Level::resolveCollision()` produz o mesmo estado lógico quando o mesmo conjunto de contactos é apresentado em ordens diferentes.

## Escopo

### Inclui

- teste com múltiplas plataformas potencialmente sobrepostas ao mesmo corpo;
- comparação de permutações da ordem de `m_platforms`;
- comparação de posição, velocidade e estado grounded;
- decisão documentada baseada no resultado observado.

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
- Windows CI, se o teste for integrado.

## Decisões arquiteturais

```text
Problema/contexto:
resolveCollision() percorre plataformas por ordem de armazenamento e aplica mutações imediatamente.

Decisão inicial:
Investigar a propriedade empiricamente antes de alterar a implementação.

Alternativas consideradas:
- assumir ordem irrelevante: rejeitada por falta de evidência;
- introduzir imediatamente um solver de contactos novo: rejeitada por scope excessivo sem contraexemplo;
- teste de permutação: escolhida como intervenção mínima.

Condição de revisão:
Se existir divergência observável, desenhar primeiro o contrato de resolução antes de reescrever o algoritmo.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| teste não reproduzir contacto simultâneo relevante | média | médio | usar geometrias sobrepostas e várias permutações | aberto |
| resultado positivo ser confundido com prova universal | média | alto | limitar a conclusão ao domínio exercitado | aberto |
| investigação evoluir para rewrite sem finding | média | alto | nenhum código de produção antes de contraexemplo/decisão | mitigado |

## Validação

### Testes automatizados

- executar duas ou mais ordens para o mesmo conjunto de plataformas;
- comparar estado lógico após resolução.

### Failure paths

- contacto sem colisão;
- dois contactos sobrepostos;
- permutação que produz múltiplas mutações na mesma etapa.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] risco de scope creep registado.

## Definition of Done

- [ ] teste de permutação integrado;
- [ ] resultado observado documentado;
- [ ] correção implementada apenas se necessária;
- [ ] testes/CI relevantes passam;
- [ ] Gate matrix atualizada;
- [ ] dívida residual classificada.

## Alterações durante a execução

Nenhuma até ao momento. A implementação de produção permanece bloqueada enquanto a propriedade não tiver sido experimentalmente caracterizada.

## Evidência / referências

- `docs/AUDITS/2026-08-28-collision-order-investigation.md`;
- `Game/Logic/Level.cpp`;
- `Tests/Unit/test_level_collision.cpp`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-next-9-6-runtime-bootstrap-gate.md`.

## Fecho

**Resultado:** `em investigação`  
**Critério de saída:** resultado reproduzível de uma propriedade de permutação ou contraexemplo concreto  
**Dívida residual:** sem alteração de produção antes da decisão baseada em evidência
