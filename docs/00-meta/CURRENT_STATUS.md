# ASCENDENDO — Estado actual

**Data de referência:** 2026-09-10
**Fonte:** `main` e alterações desta tranche.

## Produto

O núcleo jogável e os editores de nível/campanha já existem. O défice imediato é conteúdo validado por pessoas, não arquitectura adicional.

## Sistemas existentes

- Vulkan + GLFW e `RenderSnapshot`.
- Física fixed timestep a 60 Hz.
- Commitment Jump a 60° sem controlo aéreo.
- Câmara vertical e viewport lógico 640×360.
- Replay/save states e histórico de runs.
- Menu, pausa, créditos e bindings persistentes.
- `GameState::EDITOR` e `GameState::CAMPAIGN_EDITOR`.
- Validação síncrona e assíncrona do editor.
- Composição de plataformas 8-neighbour com adjacência cross-region.
- Swapchain recreation fail-closed.
- CI Linux/Windows e deterministic capture.

## Conteúdo actual

A campanha activa contém **15 níveis** em `Game/Assets/Levels/campaign.txt`.

A tranche 10–15 acrescenta seis níveis sem alterar a física nem introduzir novo tooling. Cada um usa plataformas compatíveis com a grelha actual e uma sequência ascendente dentro do envelope do salto usado pelo validador.

O `ai_validator.py --campaign` é um filtro de alcançabilidade algorítmica. Não prova diversão, dificuldade adequada, legibilidade, desempenho ou completabilidade humana.

## Assets

A proveniência continua separada da integração. O `PLATFORM_ASSET_REGISTRY.md` ainda contém candidatos externos sem binário runtime concreto. Isso é intencional: não declarar integração antes de existir o ficheiro exacto, dimensões verificadas, hash e revisão visual.

## Processo

Não adicionar novas auditorias, snapshots de roadmap ou infraestrutura de apresentação sem evidência concreta. Problemas transitórios ficam no desenvolvimento/log; engenharia nova deve responder a uma necessidade demonstrada pelo produto.

## Próxima porta de qualidade

```text
validação automática da campanha
→ build/runtime real
→ playtest humano externo
→ corrigir níveis com base nas falhas
→ só depois aprofundar apresentação/arte/performance
```
