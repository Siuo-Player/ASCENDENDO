# Work Package Template

Este template é o contrato operacional mínimo para qualquer bloco de desenvolvimento do ASCENDENDO.

A unidade de planeamento deve ligar o roadmap ao código e à validação. Não é necessário preencher detalhes incertos artificialmente; o documento deve ser refinado quando o trabalho revela nova informação.

## Identificação

**Roadmap:** `<fase / bloco>`  
**Subsistema:** `<Runtime | Editor | Presentation | Infrastructure | Sharing>`  
**Work Package:** `<ID + nome>`  
**Branch:** `<branch>`  
**PR:** `<#>`

## Objetivo

Descrever o resultado observável que este work package deve produzir.

## Escopo

### Inclui

- `<item>`

### Não inclui

- `<item>`

## Dependências

### Depende de

- `<work package / interface / decisão>`

### Produz para

- `<consumidor / work package>`

### Consumidores afetados

- `<código / testes / ferramenta / documentação>`

### Dependências de validação

- `<teste / ambiente / hardware / capability>`

## Decisões arquiteturais

Registar apenas decisões que condicionem outros componentes ou trabalho futuro.

```text
Problema/contexto:
Decisão:
Alternativas consideradas:
Consequências:
Condição de revisão/remoção:
```

## Riscos

| Risco | Probabilidade | Impacto | Mitigação | Estado |
|---|---|---|---|---|
| `<risco>` | baixa/média/alta | baixo/médio/alto | `<ação>` | aberto/mitigado |

## Validação

### Testes automatizados

- `<teste>`

### Validação manual

- `<fluxo>`

### Profiling / métricas

- `<métrica>`

### Failure paths

- `<falha que deve ser exercitada>`

## Definition of Ready

- [ ] objetivo e escopo definidos;
- [ ] documentos normativos consultados;
- [ ] dependências críticas identificadas;
- [ ] critério de saída definido;
- [ ] estratégia de validação definida;
- [ ] riscos relevantes registados.

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

Se aparecer informação que altere materialmente objetivo, arquitetura, dependências, risco, validação ou ordem do roadmap, registar aqui antes de continuar a expandir o escopo.

```text
Descoberta:
Impacto:
Decisão tomada:
Documentos atualizados:
``` 

## Evidência / referências

- `<paper / documentação oficial / projeto maduro / resultado interno>`

## Fecho

**Resultado:** `<concluído / bloqueado / parcialmente concluído>`  
**Critério de saída:** `<evidência>`  
**Dívida residual:** `<nenhuma / item>`
