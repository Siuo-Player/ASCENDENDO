# ASCENDENDO — Snapshot operacional — 03/09/2026

## Baseline exato

- `main`: `d61036d130ac45846654fd0eef63e7ad5c17744b`
- Estado: editor de entidades e core save/validation vertical slice integrados; apresentação semântica de plataformas permanece separada.
- PR #220: fechado por duplicação/supersession; PR #221, #222 e #224 estão merged.
- Não há assets finais visualmente aprovados por este snapshot.
- A avaliação visual humana continua pendente.

## Editor — estado real

`GameState::EDITOR` está integrado no motor.

### Manipulação de entidades

PLATFORM, SPAWN e FLAG são ferramentas explícitas. MOVE é transacional e cancelável; SPAWN respeita a plataforma inicial; FLAG é condicionado ao nível final.

### Persistência

O serializer `.lvl` canónico continua sendo a única representação persistida. O editor grava primeiro para um temporário completo e só depois substitui o destino; falhas de escrita deixam o destino protegido enquanto o temporário não estiver completo.

### Validação

O editor tem duas camadas distintas que não devem ser confundidas:

1. `validateEditorDocument` / `LevelEditorValidator`: validação síncrona do documento antes do save, com regras existentes de geometria/reachability.
2. `EditorValidationTask`: validação assíncrona de um snapshot imutável de `LevelData`, identificada por `generation` e `levelPath`, com estado `STALE` quando o documento muda.

A integração direta com `Development/AI_Validation/ai_validator.py` para validação física/campanha completa ainda não existe.

## Apresentação

A cadeia semântica de plataformas está implementada:

`LevelData → PlatformPresentationRasterizer → RegionCell → 8-neighbour/cross-region signature → RenderSnapshot → WorldRenderer → asset request/selector`

O sistema permanece presentation-only e não altera física/collision geometry.

## Assets e evidência

Existe metadata/provenance de candidatos e seleção deterministicamente limitada. Isto não constitui aprovação visual. Deterministic capture demonstra comportamento técnico reprodutível; não substitui revisão humana.

## Próximo trabalho

1. 9.6 gestão de campanha: ordem canónica, reorder tipo playlist, persistência e validação de referências.
2. Fechar, se necessário, a ponte entre validação assíncrona do editor e o validador físico/campanha completo.
3. Expandir a validação do compositor para corpus real mais amplo.
4. Só depois promover assets/props com evidência completa.
5. Audio Design permanece workstream separado.

## Fontes metodológicas consultadas

O estado metodológico mais recente observado no repositório `Siuo-Player-PROJECT-STUDIES` foi o commit `2fbc22c5a230b90a471493446f55af33c4c6788a` (`docs: record formal tranche completion`, 02/09/2026). O repositório de estudos não foi modificado.
