# Camera follow Lerp debt — 2026-08-29

`Camera::follow()` usava `speed * dt` diretamente como fator de interpolação. Para `speed * dt > 1`, a atualização pode extrapolar o alvo.

A branch `fix/camera-follow-lerp-clamp-20260829` limita o fator a `[0,1]` e adiciona um teste de regressão com `dt = 1.0s`.

O problema é local à câmera e não altera fixed-step, offset, coordenadas ou gameplay.
