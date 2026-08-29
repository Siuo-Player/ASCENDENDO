# Plano da branch atual

**Bloco do roadmap:** `Fase 10 / presentation + camera validation`

**Estado atual:** `Camera follow Lerp bound` e `Movement / Camera deterministic benchmark` já estão integrados em `main`.

## Últimas tranches integradas

### Camera follow Lerp bound

**Implementation:** branch `fix/camera-follow-lerp-clamp-20260829`  
**PR:** `#170`  
**Merge:** `284d4c807569dc5960c349a67ce0ef87f0aed4ec`

`gfx::Camera::follow()` passou a limitar o fator de Lerp a `[0,1]`, eliminando extrapolação quando `speed * dt > 1`.

### Movement / Camera deterministic benchmark

**Implementation:** branch `feat/movement-camera-benchmark-20260829`  
**PR:** `#172`  
**Merge:** `749ec9f3977ee42331b46600a1eaf314e4def8b9`

Foi integrada a primeira tranche do benchmark determinístico de movimento/câmara:

- camera follow com `dt` grande;
- tracking em fixed-step;
- lower bound da câmara;
- world → NDC;
- referência de letterbox em múltiplos viewports;
- dimensões de viewport inválidas;
- finitude da grelha de referência;
- relatório JSON opcional.

O benchmark é independente de Vulkan/GLFW e não altera o runtime de produção. Serve como oráculo numérico para futuras alterações de camera/movement.

## Próxima prioridade do roadmap

**Não voltar a implementar a tranche já concluída.** O próximo trabalho deve ser escolhido a partir do estado real de `main` e da evidência mais recente dos Studies.

A prioridade imediata é a continuação de **presentation + camera validation**, começando por **stress scenes / render captures para múltiplos viewports**. O objetivo é validar propriedades de visibilidade e comportamento observável que os testes matemáticos não conseguem provar.

```text
Movement / Camera benchmark       DONE
          ↓
stress scenes / render captures  NEXT
          ↓
human playtesting protocol
          ↓
difficulty / progression calibration
          ↓
procedural assistance
          ↓
player-conditioned generation
```

Antes de abrir uma nova branch:

```text
1. ler docs/ROADMAP.md
2. comparar com ASCENDENDO/main atual
3. consultar PROJECT-STUDIES/ASCENDENDO/
4. identificar uma propriedade ou objetivo concreto
5. confirmar consumidor, risco e validação
6. só então implementar
```

## Regra de escopo

Não criar uma nova regra de camera, validator ou abstração apenas por consistência estética. Um finding novo precisa de propriedade operacional clara, risco/falha demonstrável e teste/validação adequado.

Não criar `Application` genérica, schema/versioning, nova política geral de bounds ou outras abstrações adiadas sem requisito concreto.
