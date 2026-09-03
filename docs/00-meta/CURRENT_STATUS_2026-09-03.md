# ASCENDENDO — Snapshot operacional — 03/09/2026

## Baseline exato

- `main`: `8f41f5e9aabaf5c5fb122a661efa4515dc208579`
- PR #225: merged — active roadmap pointer/reconciliation.
- PR #227: merged — dedicated pixel-art approval inbox; no artwork was promoted to runtime.
- PR #226: merged — campaign ordering persistence and reference validation.
- PR #228: open — real campaign compositor corpus validation.
- CI obrigatório mantido: `tests.yml`, `windows.yml`, `deterministic-capture.yml`.
- `Siuo-Player-PROJECT-STUDIES` continua apenas como fonte metodológica; não é modificado pelo projeto.

## Editor — estado real

`GameState::EDITOR` está integrado no motor.

### Manipulação de entidades

PLATFORM, SPAWN e FLAG são ferramentas explícitas. MOVE é transacional e cancelável; SPAWN respeita a plataforma inicial; FLAG é condicionado ao nível final.

### Persistência de níveis

O serializer `.lvl` canónico continua sendo a representação persistida. O editor grava primeiro para um temporário completo e só depois substitui o destino.

### Gestão de campanha — 9.6 concluído

`CampaignEditorDocument` representa uma playlist discreta em memória e mantém `campaign.txt` como fonte canónica da ordem. A tranche integrada acrescenta:

- reordenação de níveis;
- validação de referências existentes e regulares;
- rejeição de referências duplicadas, não-`.lvl`, campanhas vazias e escapes do diretório da campanha;
- persistência staged da ordem para `campaign.txt`;
- testes de reorder → save → reload e failure paths.

O editor não move ficheiros entre diretórios de validade; esse routing continua fora desta camada.

### Validação do editor

O editor tem duas camadas distintas:

1. `validateEditorDocument` / `LevelEditorValidator`: validação síncrona do documento antes do save.
2. `EditorValidationTask`: validação assíncrona de snapshot imutável de `LevelData`, identificada por `generation` e `levelPath`, com estado `STALE` quando o documento muda.

A integração direta com `Development/AI_Validation/ai_validator.py` para substituir ou ocultar essa camada ainda não foi introduzida. O validador físico/campanha continua com os seus próprios gates.

## Apresentação — 9.7 em validação

A cadeia semântica permanece:

`LevelData → PlatformPresentationRasterizer → RegionCell → 8-neighbour/cross-region signature → RenderSnapshot → WorldRenderer → asset request/selector`

A PR #228 amplia a evidência do compositor sobre o corpus real actualmente listado em `Game/Assets/Levels/campaign.txt`, verificando modularidade 16×16, footprint exacto e preservação das coordenadas contínuas sem alterar gameplay geometry.

Esta evidência estrutural não é aprovação visual.

## Assets — gate de aprovação humana

Existe agora `Game/Assets/Sprites/ART_APPROVAL_INBOX/` como ponto único de revisão visual.

Cada candidato é classificado como `PENDING`, `APPROVE`, `REJECT`, `REWORK` ou `LIMIT`. Provenance/licença e testes técnicos continuam necessários, mas a aprovação visual final é uma decisão humana separada.

Nenhum candidato externo foi importado/promovido apenas por estar no inbox.

## Próximo trabalho

1. Fechar PR #228 se todos os gates passarem.
2. Se necessário, só então fechar a ponte entre validação assíncrona do editor e o validador físico/campanha completo — sem duplicar o validador existente.
3. Após a evidência 9.7, tratar a escolha de assets pixel-art através do inbox e apenas depois promover os ficheiros explicitamente aprovados.
4. Construir a cena de props curada depois de existir uma base visual aprovada.
5. Fazer validação visual humana de editor/jogo/capturas.
6. Iniciar Audio Design como workstream separado.

## Fontes metodológicas consultadas

O estado metodológico mais recente observado no repositório `Siuo-Player-PROJECT-STUDIES` foi o commit `2fbc22c5a230b90a471493446f55af33c4c6788a` (`docs: record formal tranche completion`, 02/09/2026). O repositório de estudos não foi modificado.
