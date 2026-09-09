# `NaoValidados` — auditoria do corpus — 2026-09-09

## Finding

A árvore atual de `main` contém `Game/Assets/Levels/NaoValidados/`, mas o diretório tem apenas `.gitkeep`; não existem ficheiros `.lvl` concretos no corpus `NaoValidados` neste snapshot.

A árvore também confirma que os níveis concretos atualmente presentes estão fora desse diretório, em particular `inicio.lvl`, `precipicio.lvl` e `zigzag.lvl`, além de `campaign.txt`.

## Consequência

A tarefa de “validar os mapas `NaoValidados`” não pode ser executada como validação de conteúdo neste momento, porque o conjunto de entrada é vazio.

Isto deve ser classificado como **ausência de corpus**, não como:

- aprovação dos mapas;
- prova de playability;
- prova de correção física;
- prova de qualidade visual.

## Gate correto

Quando um ou mais `.lvl` forem adicionados a `NaoValidados`, a validação deve separar três camadas:

```text
1. validade estrutural
   → parsing + geometria finita + limites + sintaxe

2. playability / physics
   → start + movimento + collision + landing + reachability relevante

3. captura / visual
   → deterministic capture + inspeção visual humana
```

O `level_validator.py` existente cobre atualmente uma parte do primeiro nível (sintaxe, bounds e presença opcional de `NAME`) e a validação da campanha; não deve ser tratado como substituto dos níveis 2 e 3.

## Decisão

Não alterar o validator apenas para tornar o diretório “validável”. Não introduzir fixtures artificiais em `NaoValidados` para produzir métricas fictícias.

Quando o primeiro ficheiro real surgir, abrir uma tranche de validação dirigida que registe por ficheiro:

```text
arquivo
→ structural result
→ physics/playability result
→ capture result
→ disposition (promote / rework / reject)
```

## Estado operacional

`Fase 1.4` fica **bloqueada por corpus vazio**, não por falha de validação.
