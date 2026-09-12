# AGENTS.md — regras de trabalho do ASCENDENDO

Este ficheiro é uma orientação operacional para agentes de IA que trabalham no repositório. O código e os assets no `main` são a fonte técnica de verdade; `docs/ROADMAP.md` é o único roadmap vivo. Se uma decisão externa deste prompt entrar em conflito com evidência posterior no repositório, investigar a evidência antes de alterar o contrato.

## Antes de alterar

1. Ler `docs/ROADMAP.md`.
2. Ler a documentação da área tocada (`PRODUCT_VISION`, `ARCHITECTURE`, `LEVEL_FORMAT`, `TESTING`, decisões relevantes).
3. Consultar Issues/PRs relacionados e confirmar o estado real no `main`.
4. Identificar a Issue canónica. Se não existir, criar uma antes de implementar uma mudança de produto.

## Processo

- Investigar → implementar → branch → PR → CI → corrigir → CI novamente.
- Não fazer commits directos em `main` quando uma branch/PR for apropriada.
- Não declarar uma feature concluída apenas porque compila ou passa um teste isolado.
- Uma etapa só está concluída com testes/regressões relevantes e CI real.
- Não criar snapshots `STATUS_*`, `CURRENT_STATUS_*` ou `ROADMAP_*` datados.
- Não criar post-mortems ou documentos novos para cada bug/CI; preferir actualizar a documentação viva ou o diário existente.

## Modelo do produto

ASCENDENDO é um platformer vertical de precisão, não uma engine genérica. Preferir sistemas semânticos específicos ao jogo e evitar abstrações que não resolvam um problema real.

A campanha é uma subida vertical contínua. `SCREEN`/`SEGMENT` é uma referência de authoring; vários screens formam um espaço vertical contínuo. Nunca assumir `LOGICAL_HEIGHT == levelHeight`.

### Level authoring

- `PLATFORM` é o principal conteúdo authored.
- Spawn é estado derivado: chão inicial canónico, posição horizontal canónica e nunca serializado.
- O objetivo é derivado no nível final a partir da plataforma authored mais alta e nunca é serializado como `FLAG`.
- Não reintroduzir ferramentas de authoring para `SPAWN` ou `FLAG`; APIs legadas podem permanecer apenas por compatibilidade.
- Posição de plataforma não deve ser forçada a uma grelha global. A composição visual 16×16 é uma regra de apresentação/forma, não uma razão para arredondar coordenadas de authoring.

### Editor

Prioridade:

1. undo/redo por gesto e regressões;
2. validação física e diagnóstico visível;
3. HUD/contexto e layout responsivo;
4. picker/select e manipulação directa, incluindo resize;
5. navegação vertical/minimap;
6. apresentação visual estruturada.

Um drag inteiro é uma operação de histórico, independentemente do número de frames. Uma nova edição limpa redo.

A validação estrutural assíncrona e a validação física de reachability são conceitos diferentes. O renderer não recalcula física: deve receber um `EditorRenderSnapshot` já enriquecido pelo validator.

A validação diz se uma geometria é fisicamente/estruturalmente possível; não diz se o nível é divertido ou bom para humanos.

## Gameplay e dificuldade

Não adicionar mortes, hazards ou outras mecânicas só para criar progresso.

A dificuldade mecânica/control-space e a dificuldade humana são camadas diferentes. Perfis de simulação são hipóteses até haver calibração humana. Completion rate de uma IA não deve ser tratado automaticamente como dificuldade humana.

O objetivo de uma run é uma subida contínua; não tratar os 25 níveis actuais como 25 runs independentes.

## Visual e arquitectura

Prioridade visual: gameplay readability → navigation → atmosphere → decoration.

A área lógica é 640×360 com apresentação adaptável. Fullscreen/resizing deve preservar composição e aspect ratio lógico.

Não expandir o `PlatformCompositor` por antecipação. Integrar primeiro assets reais, licenciados e verificáveis; auto-tiling é apresentação e nunca altera geometria de gameplay.

## Verificação

Para alterações críticas, usar os workflows relevantes: testes nativos, Windows, campanha, captura/determinismo e gates de warnings/UI quando aplicável.

No Windows/PowerShell, não dar comandos bash incompatíveis. Para Python, preferir `py -m pytest` quando aplicável.

Se um CI falhar, investigar a causa e corrigir a branch. Um rerun só é evidência suficiente quando o job realmente termina com sucesso e não houve alteração necessária.
