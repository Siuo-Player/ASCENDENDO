# Roadmap de desenvolvimento

## Estado no início desta documentação

`main` encontra-se no fim da infraestrutura da Fase 9.3.

Concluído:

- Fases 1–8: motor, física, campanha, UI, texto TTF, sprites, replay/save e validação.
- 9.1: `GameAction` + `KeyBindings`.
- 9.2: rato + conversão window/logical + menus clicáveis.
- 9.3: `GameState::EDITOR`, acesso por menu/tecla e câmera/grelha.
- correção de bootstrap GLFW/Vulkan e cleanup de falhas parciais, integrada em `main` por PR #1.

## Fase 9 — Editor de níveis

### 9.1 ✅ — Controlos

Concluída. O sistema de intenção lógica existe e permite rebind/persistência.

Pendente histórico: UI visual de CONTROLS e ligação das ações `MoveLeft/MoveRight/Jump` ao gameplay normal. Não bloqueará 9.4 se os bindings do editor forem independentes e corretamente testados.

### 9.2 ✅ — Input de rato e viewport

Concluída. Cursor, botões, `windowToLogical()` e hit-test existem.

### 9.3 ✅ — Estado EDITOR

Concluída. O editor abre dentro do jogo, tem câmera livre e grelha.

### 9.4 ▶ — Manipulação de entidades

Objetivo atual:

- documento editável em memória;
- plataformas STAMP/DRAG;
- preset pequeno/médio/grande;
- seleção/movimento;
- apagar por rato ou teclado;
- snap obrigatório;
- spawn restrito;
- FLAG única da campanha no último nível;
- testes unitários da camada editável.

Default de interação: STAMP + tamanho médio.

### 9.5 — Guardar + validar

Depois de 9.4:

- serializar o documento para o formato `.lvl` existente;
- colocar níveis em construção/incorretos em `NaoValidados/`;
- executar validação em background;
- mostrar progresso/estado;
- permitir sair do editor sem bloquear o jogo enquanto a validação termina;
- publicar o resultado como notificação quando disponível.

### 9.6 — Editor de campanha

Depois de 9.5:

- lista de níveis em estilo playlist;
- reordenar a campanha;
- abrir um nível diretamente no `GameState::EDITOR` através de uma transição de estado;
- persistir apenas a ordem em `campaign.txt`;
- não mover ficheiros de `Levels`, `Unused` ou `NaoValidados`.

### Pós-9.6 — sprites do editor

A gestão de sprites entra antes do Release Build se ainda for necessária para o conteúdo final. A política é partilha de recursos GPU e referências compactas por entidade, com atlas quando for benéfico.

### Fase 10 — Release Build

Só depois da experiência de jogo/editor estar funcional:

- executable portable;
- sem consola para a versão final;
- assets e DLLs na pasta local;
- paths relativos;
- mensagens de erro amigáveis;
- documentação de jogador separada da documentação de desenvolvimento.

## Ideias não aprovadas

Partilha de campanhas/níveis/runs entre máquinas continua fora do roadmap ativo.

## Regra de progressão

Nenhuma fase seguinte começa enquanto a anterior não tiver:

- implementação coerente;
- testes relevantes;
- documentação atualizada;
- branch própria;
- PR aberta;
- PR integrada em `main`.

Antes de cada novo passo: integrar o PR anterior, abandonar a branch anterior e criar uma branch nova a partir do `main` atualizado.