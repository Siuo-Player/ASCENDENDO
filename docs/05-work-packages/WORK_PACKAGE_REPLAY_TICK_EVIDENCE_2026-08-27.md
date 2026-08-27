# Work Package — Replay tick evidence

## Identificação

**Roadmap:** 9.6 — Evidência transversal  
**Subsistema:** Runtime  
**Work Package:** 9.6 — Replay tick evidence  
**Branch:** `fix/9-6-replay-tick-evidence-20260827`  
**PR:** próxima PR deste branch

## Objetivo

Alinhar o subsistema `ReplayManager` com a unidade semântica `TickInput` introduzida no PR #88 e produzir evidência reproduzível de determinismo por tick, sem criar uma nova abstração `Replay` e sem iniciar a migração geral de `RenderSnapshot`.

O resultado observável esperado é:

```text
mesmo estado inicial
+ mesma sequência de TickInput por tick
→ mesmo estado de simulação por tick
```

## Escopo

### Inclui

- substituir a representação de replay que depende diretamente de `InputManager` por uma sequência explícita de `TickInput`;
- manter save-state/rewind existentes sem ampliar o modelo de ownership;
- permitir playback consumindo `TickInput` diretamente;
- criar testes que comparem estado por tick entre duas execuções da mesma sequência;
- criar teste de repartição de ticks em frames diferentes quando a sequência semântica for a mesma;
- atualizar documentação de Gate 9.6 e dívida técnica para distinguir `TickInput` implementado de replay determinístico ainda não demonstrado.

### Não inclui

- formato persistente novo para replay;
- gravação em ficheiro ou interoperabilidade de replay;
- nova classe `Replay`/`ReplaySystem`;
- RenderSnapshot geral;
- mudança da física além do necessário para tornar o teste observável;
- alteração da política de LevelData/schema da Fase 10.

## Dependências

### Depende de

- PR #88 / contrato `TickInput`;
- `SimulationOrchestrator` fixed-step atual;
- estado atual de `ReplayManager` e seus testes;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-deterministic-input-replay-evidence-matrix.md`.

### Produz para

- Gate 9.6 — evidência de determinismo/replay;
- futura persistência de replay, caso venha a ser aprovada;
- eventual RenderSnapshot, que exige gameplay deterministicamente reproduzível.

### Consumidores afetados

- `ReplayManager`;
- `Tests/Unit/test_replay.cpp`;
- callers de playback/recording, caso existam;
- documentação do Gate 9.6.

### Dependências de validação

- Linux normal + headless Vulkan;
- Linux ASan/UBSan;
- Windows CI;
- testes determinísticos sem dependência de GLFW real.

## Decisões arquiteturais

```text
Problema/contexto:
ReplayManager ainda grava FrameInput diretamente do InputManager, apesar de o gameplay já consumir TickInput.

Decisão:
O replay deve armazenar e reproduzir a unidade semântica TickInput. A fonte do replay não deve voltar a consultar teclas físicas nem depender do agrupamento dos ticks em frames.

Alternativas consideradas:
- manter FrameInput: rejeitada, porque preserva a ambiguidade frame/tick;
- introduzir ReplaySystem novo: rejeitada por ausência de responsabilidade adicional demonstrada;
- persistir imediatamente em ficheiro: adiada, porque persistence é uma propriedade distinta (D7).

Consequências:
- replay passa a ter semântica compatível com o tempo da simulação;
- testes podem comparar estados por tick sem GLFW;
- persistence continua independente.

Condição de revisão/remoção:
Rever se o modelo de `TickInput` mudar ou se uma futura persistência exigir uma representação serializável diferente.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| replay antigo assume frame grouping | média | alto | migrar consumidor para `TickInput` | aberto |
| estados comparados só por posição mascaram divergência | média | alto | snapshotar campos persistentes relevantes | aberto |
| frame repartition test usar sequências semanticamente diferentes | média | alto | construir explicitamente a mesma sequência de ticks | aberto |
| persistence ser inferida do replay em memória | média | médio | manter D7 separado | mitigado |

## Validação

### Testes automatizados

- replay da mesma sequência em duas instâncias produz estado idêntico após cada tick;
- edge events são associados ao tick correto;
- repartição `1+1+1+1` versus `2+2` preserva a sequência semântica e os estados;
- terminal state/completion, quando parte do estado simulado testável, é reproduzível.

### Validação manual

- inspeccionar que nenhum gameplay path chama `Key::...` ou `InputManager` a partir de `Player`;
- verificar que playback não consulta input físico.

### Profiling / métricas

- nenhuma otimização é objetivo deste WP;
- registar apenas custo adicional relevante do armazenamento de `TickInput`, se observável.

### Failure paths

- sequência vazia;
- playback além do fim;
- estado inicial inválido, se suportado pela API atual;
- replay limpo/reiniciado sem deixar eventos ou índices antigos.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [ ] implementação concluída dentro do escopo;
- [ ] testes relevantes passam;
- [ ] failure paths relevantes foram exercitados;
- [ ] documentação normativa foi atualizada;
- [ ] dependências alteradas foram revistas;
- [ ] dívida técnica criada foi classificada;
- [ ] critério de saída foi demonstrado;
- [ ] PR pronta para merge sem trabalho essencial oculto.

## Alterações durante a execução

```text
Descoberta:
ReplayManager atual ainda usa FrameInput + InputManager diretamente.

Impacto:
A semântica tick-scoped de #88 não chega ao replay existente.

Decisão tomada:
Tratar replay determinístico como trabalho próprio do Gate 9.6, separado de persistence.

Documentos atualizados:
Este WP; ROADMAP.md; TECH_DEBT.md; auditoria/validação deste WP.
```

## Evidência / referências

- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-deterministic-input-replay-evidence-matrix.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-deterministic-replay-input-contract-study.md`;
- `Game/Logic/ReplayManager.*` no `main` antes deste WP;
- PR #88 — `TickInput`.

## Fecho

**Resultado:** `em execução`  
**Critério de saída:** evidência por tick + frame-repartition test + environment/artifact record  
**Dívida residual:** persistence D7 e broader replay evidence permanecem separadas
