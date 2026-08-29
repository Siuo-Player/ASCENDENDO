# Camera follow Lerp debt — 2026-08-29

**Status:** RESOLVED

`Camera::follow()` usava `speed * dt` diretamente como fator de interpolação. Para `speed * dt > 1`, a atualização podia extrapolar o alvo.

A correção limita o fator a `[0,1]` e adiciona um teste de regressão com `dt = 1.0s`.

PR #170 foi integrada por squash como `284d4c807569dc5960c349a67ce0ef87f0aed4ec` após os três gates obrigatórios terminarem com sucesso no head efetivamente validado `2a96eed38c7cc87e456c03a3b15d1f712d57d4ea`.

O problema era local à câmera e não alterou fixed-step, offset, coordenadas ou gameplay.

A qualidade de camera/viewport continua uma questão de investigação separada: não assumir novas políticas sem propriedade operacional, consumidor afetado e teste demonstrável.
