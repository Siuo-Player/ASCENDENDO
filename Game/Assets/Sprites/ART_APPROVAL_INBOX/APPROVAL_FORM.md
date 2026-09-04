# Approval form — preenchimento humano

Preenche depois de veres os candidatos visuais. Uma linha por ID é suficiente.

```text
ART-001: APPROVE / REJECT / REWORK / LIMIT
ART-002: APPROVE / REJECT / REWORK / LIMIT
ART-003: APPROVE / REJECT / REWORK / LIMIT
ART-004: APPROVE / REJECT / REWORK / LIMIT
ART-005: APPROVE / REJECT / REWORK / LIMIT
ART-006: APPROVE / REJECT / REWORK / LIMIT
ART-007: APPROVE / REJECT / REWORK / LIMIT
```

## Estado do inbox

Os IDs acima são espaços de decisão, não afirmam que existam sete ficheiros binários concretos no repositório.

Antes de qualquer aprovação, cada ID tem de apontar para:

- um ficheiro visual concreto ou preview verificável;
- source e licença/provenance verificáveis;
- escala e dimensões exactas;
- função proposta (`PLATFORM`, `PLAYER`, `FLAG`, `PROP`, etc.);
- compatibilidade técnica documentada;
- estado final humano (`APPROVE`, `REJECT`, `REWORK`, `LIMIT`).

Não preencher `APPROVE` por inferência a partir de um registry, nome de pacote ou licença CC0. Candidato sem ficheiro visual concreto permanece não aprovado.

## Comentários opcionais

Para `REWORK` ou `LIMIT`, indica a condição necessária (por exemplo: nova paleta, outro tamanho, apenas fundo, apenas plataforma, etc.).

## Nota

Uma aprovação aqui não altera automaticamente o jogo. Depois da aprovação será feita uma alteração de integração separada, sujeita aos gates técnicos e de provenance.
