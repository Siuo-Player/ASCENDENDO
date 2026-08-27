# Work Package — Replay tick evidence

## Identificação

**Roadmap:** 9.6 — Evidência transversal  
**Subsistema:** Runtime  
**Work Package:** 9.6 — Replay tick evidence  
**Branch:** `fix/9-6-replay-tick-evidence-20260827`  
**PR:** #90

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
- criar teste de agrupamento diferente quando a sequência semântica de `TickInput` for a mesma;
- atualizar documentação de Gate 9.6 e dívida técnica para distinguir `TickInput` implementado de replay determinístico ainda não demonstrado.

### Não inclui

- formato persistente novo para replay;
- gravação em ficheiro ou interoperabilidade de replay;
- nova classe `Replay`/`ReplaySystem`;
- RenderSnapshot geral;
- mudança da física além do necessário para tornar o teste observável;
- alteração da política de LevelData/schema da Fase 10;
- alegação de que o input live amostrado por frames é independente da taxa de frames quando edge events são envolvidos.

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
- callers de playback/recording existentes, caso surjam;
- documentação do Gate 9.6.

### Dependências de validação

- Linux normal + headless Vulkan;
- Linux ASan/UBSan;
- Windows CI;
- testes determinísticos sem dependência de GLFW real.

## Decisões arquiteturais

```text
Problema/contexto:
ReplayManager gravava FrameInput diretamente do InputManager, apesar de o gameplay já consumir TickInput.

Decisão:
O replay armazena e reproduz a unidade semântica TickInput. A fonte do replay não consulta teclas físicas nem depende do agrupamento dos ticks em frames.

Alternativas consideradas:
- manter FrameInput: rejeitada, porque preserva a ambiguidade frame/tick;
- introduzir ReplaySystem novo: rejeitada por ausência de responsabilidade adicional demonstrada;
- persistir imediatamente em ficheiro: adiada, porque persistence é uma propriedade distinta (D7).

Consequências:
- replay passa a ter semântica compatível com o tempo da simulação;
- testes podem comparar estados por tick sem GLFW;
- persistence continua independente;
- frame grouping pode ser testado apenas depois de a entrada estar reduzida à sequência semântica de ticks.

Condição de revisão/remoção:
Rever se o modelo de `TickInput` mudar ou se uma futura persistência exigir uma representação serializável diferente.
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| replay antigo assume frame grouping | média | alto | migrar consumidor para `TickInput` | mitigado |
| estados comparados só por posição mascaram divergência | média | alto | snapshotar campos persistentes relevantes | mitigado |
| frame grouping ser confundido com live-input frame-rate independence | média | alto | documentar explicitamente a fronteira da propriedade | mitigado |
| persistence ser inferida do replay em memória | média | médio | manter D7 separado | mitigado |

## Validação

### Testes automatizados

- replay da mesma sequência em uma segunda execução produz estado idêntico após cada tick;
- edge events são mantidos como dados explícitos do tick;
- a mesma sequência semântica de ticks produz o mesmo estado independentemente do agrupamento externo dos calls;
- sequência vazia não produz ticks;
- playback além do fim é rejeitado sem mutar estado.

### Validação manual

- `Player` não consulta `InputManager` nem teclas físicas;
- playback não consulta input físico;
- `ReplayManager` não cria uma nova camada de runtime.

### Profiling / métricas

- nenhuma otimização é objetivo deste WP;
- custo adicional limita-se ao armazenamento de `TickInput` em memória e só será registado se relevante.

### Failure paths

- sequência vazia;
- playback além do fim;
- replay limpo/reiniciado sem deixar eventos ou índices antigos;
- persistence continua não validada por este WP.

## Definition of Ready

- [x] objetivo e escopo definidos;
- [x] documentos normativos consultados;
- [x] dependências críticas identificadas;
- [x] critério de saída definido;
- [x] estratégia de validação definida;
- [x] riscos relevantes registados.

## Definition of Done

- [x] implementação concluída dentro do escopo;
- [ ] testes relevantes passam no head final;
- [ ] failure paths relevantes foram exercitados;
- [ ] documentação normativa foi atualizada;
- [x] dependências alteradas foram revistas;
- [x] dívida técnica criada foi classificada;
- [ ] critério de saída foi demonstrado no head final;
- [ ] PR pronta para merge sem trabalho essencial oculto.

## Alterações durante a execução

```text
Descoberta:
ReplayManager usava FrameInput + InputManager diretamente.

Impacto:
A semântica tick-scoped de #88 não chegava ao replay existente.

Decisão tomada:
Tratar replay determinístico como trabalho próprio do Gate 9.6, separado de persistence.

Nova descoberta:
Um teste com dois ticks agrupados só prova invariância da mesma sequência semântica de TickInput; não prova que edge events do input live sejam independentes da taxa de frames.

Documentos atualizados:
Este WP e a evidência do Gate 9.6 devem refletir essa fronteira.
```

## Evidência / referências

- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-deterministic-input-replay-evidence-matrix.md`;
- `PROJECT-STUDIES/ASCENDENDO/RESEARCH_INBOX/2026-08-27-deterministic-replay-input-contract-study.md`;
- `Game/Logic/ReplayManager.*` no `main` antes deste WP;
- PR #88 — `TickInput`.

## Fecho

**Resultado:** `em execução`  
**Critério de saída:** evidência por tick + agrupamento semântico + environment/artifact record  
**Dívida residual:** persistence D7, completion/failure replay e live-input frame-rate independence permanecem separadas
