# ASCENDENDO — Artifact finalization 403 — incidente e resolução

**Data:** 2026-09-09  
**Workflow afetado:** `Deterministic Capture Evidence`  
**Execução observada:** `34250688093`  
**Job originalmente afetado:** `Linux / deterministic PPM capture (16-9, level 1)`

## Sintoma

O step `actions/upload-artifact@v4` transferiu o conteúdo para blob storage, calculou o digest e falhou somente ao finalizar o artifact com:

```text
Failed to FinalizeArtifact: Received non-retryable error
(403) Forbidden: Error from intermediary
```

Não houve falha de compilação, captura ou geração do PPM.

## Evidência decisiva

O mesmo job foi executado novamente sem alterar o código, o `GITHUB_TOKEN` ou as permissões do workflow.

A segunda tentativa:

- terminou com sucesso;
- finalizou o artifact `deterministic-capture-level-1-16-9`;
- criou o artifact ID `10115025267`;
- preservou `permissions: contents: read`;
- utilizou a mesma action `actions/upload-artifact@v4`.

As restantes combinações da matriz também terminaram com sucesso.

Conclusão operacional: o incidente foi classificado como **falha transitória/intermediária do serviço de artifacts**, não como erro de autorização do workflow.

## Hipóteses rejeitadas

### `actions: write`

Não foi adicionada. A permissão declarada `contents: read` não impediu a criação/finalização numa repetição do mesmo job.

Não usar aumento de permissões como tratamento genérico para este padrão de falha.

### Colisão de nomes da matrix

Não foi encontrada. Os nomes incluem `level` e `aspect`:

```text
deterministic-capture-level-${{ matrix.level }}-${{ matrix.aspect }}
```

O erro original ocorreu depois da transferência dos bytes, durante `FinalizeArtifact`, e não durante a criação inicial do artifact.

### Fork externo

O branch usado no incidente pertence ao próprio repositório `Siuo-Player/ASCENDENDO`; não era um head de fork externo.

### `retry-max-attempts`

Não suportado pelo `actions/upload-artifact`. O PR #246, que propunha esse input, foi fechado sem merge.

## Procedimento de recuperação

Quando ocorrer novamente o mesmo padrão:

```text
upload de blobs = sucesso
FinalizeArtifact = 403
```

1. não aumentar permissões apenas por causa do 403;
2. não alterar nomes dos artifacts sem evidência de colisão;
3. repetir apenas o job falhado;
4. confirmar que o artifact é posteriormente criado e finalizado;
5. só introduzir uma política de retry no workflow depois de existir evidência repetida que justifique essa complexidade.

## Correção durável aplicada

A configuração de permissões foi mantida mínima.

Os workflows Linux foram modernizados para as versões atuais das actions:

- `actions/checkout@v6`;
- `actions/upload-artifact@v6` no deterministic capture.

A atualização reduz a deriva entre workflows e elimina os avisos de execução das actions v4 em runners que já utilizam Node 24. Esta modernização **não é apresentada como correção do 403**; a resolução do incidente foi o rerun bem-sucedido.

## Limites da evidência

Este documento regista um caso reproduzido por rerun e, portanto, estabelece causalidade operacional forte para este incidente específico. Não prova que todos os futuros 403 de `FinalizeArtifact` sejam sempre transitórios; um erro futuro deve ser diagnosticado pelo padrão exato do log.

## Regra permanente

```text
403 em FinalizeArtifact após upload bem-sucedido
≠ autorização do GITHUB_TOKEN por defeito

primeiro: rerun do job falhado
só depois: alterar CI, se houver evidência repetida
```
