# Campaign Editor — modelo de interação

## Objetivo

O Campaign Editor não é uma versão ampliada do Level Editor. É uma **timeline vertical de níveis** usada para organizar, inspecionar e testar a fluidez da campanha.

A distinção é deliberada:

```text
Level Editor
    = precisão dentro de uma tela

Campaign Editor
    = composição e fluxo entre telas
```

A separação segue um padrão comum em ferramentas abertas: uma unidade jogável é editada diretamente e uma estrutura superior organiza várias unidades.

## Level Editor

- Canvas fixo: `640x360`.
- Limites sempre visíveis.
- Sem pan da própria tela.
- Snap lógico: 4 px.
- Grelha visual principal: 16 px.
- Grelha major: 64 px.
- Ferramentas com modo explícito.
- Preview/seleção visíveis.
- Playtest antes de guardar.
- Validação em memória com feedback contextual.

A inspiração de interação vem sobretudo do Godot e do SuperTux: modos distintos, snapping previsível, feedback visual e integração do editor no produto. Ver `docs/DESIGN_REFERENCES.md`.

## Campaign Editor

Cada nível é representado por uma miniatura 16:9. O bloco pode ser compacto sem perder a relação de aspeto da tela real.

A campanha é apresentada verticalmente:

```text
┌─────────────────────────┐
│ #1  inicio.lvl          │
│   ┌───────────────────┐ │
│   │     thumbnail     │ │
│   └───────────────────┘ │
└─────────────────────────┘
          ↓
┌─────────────────────────┐
│ #2  zigzag.lvl          │
│   ┌───────────────────┐ │
│   │     thumbnail     │ │
│   └───────────────────┘ │
└─────────────────────────┘
          ↓
...
```

A ordem visual representa a ordem real da campanha.

### Interações previstas

- scroll vertical da timeline;
- seleção de um nível;
- arrastar um bloco;
- snap para posições de reordenação;
- indicação clara da posição onde o bloco será inserido;
- abrir o Level Editor do nível selecionado;
- pré-visualizar níveis em sequência;
- mostrar estado de validação por nível.

Não haverá movimento livre arbitrário de níveis que possa deixar a ordem lógica ambígua: o resultado final deve ser uma ordem discreta de blocos.

## Runs de validação

O Campaign Editor deverá funcionar como uma ferramenta de observação de fluxo e não apenas como uma lista de thumbnails.

Devem existir agentes/runs visuais em background:

- pelo menos um agente ativo por nível quando possível;
- agentes em movimento para mostrar onde cada tentativa está;
- pelo menos um agente capaz de atravessar a fronteira entre níveis;
- indicador de sucesso/falha e razão útil quando uma tentativa termina.

Um run entre níveis deve tornar visível o “handoff” entre o fim de uma tela e o spawn da seguinte. O objetivo é perceber cedo se a campanha é fluida, não substituir a simulação final de produção.

## Playtest e persistência

Um nível aberto pelo Campaign Editor deve poder entrar no Level Editor sem alterar a ordem da campanha.

Um playtest deve trabalhar sobre uma cópia/snapshot do conteúdo editado, permitindo:

```text
editar
  ↓
playtest
  ↓
voltar
  ↓
continuar a editar
```

Guardar é uma ação explícita e não deve acontecer implicitamente quando o utilizador apenas testa.

## Layout adaptável

O Campaign Editor pode usar todo o espaço vertical disponível, mas os elementos individuais devem ser autoajustáveis ao viewport.

A regra é:

> uma miniatura ou painel pode ficar pequeno; nunca pode ficar parcialmente fora do viewport por causa de uma largura/posição fixa.

Texto de nomes de campanha/nível deve ter uma estratégia definida para nomes longos: redução de escala, truncamento com indicação visual ou quebra controlada. O texto nunca deve simplesmente desaparecer pela extremidade do ecrã.

## Arquitetura

```text
LevelEditorDocument
    ↓
EditorSession
    ↓
EditorRenderSnapshot
    ↓
EditorRenderer

CampaignEditorDocument
    ↓
CampaignEditorRenderSnapshot
    ↓
CampaignEditorRenderer

CampaignEditor
    └── vários ValidatorRun / SimulationRun
```

Os dois editores não partilham a mesma câmara:

- Level Editor: viewport fixo.
- Campaign Editor: timeline com deslocação vertical.

## Regras de simplicidade

O Campaign Editor deve parecer uma ferramenta de composição simples, não um editor de mapas genérico. Só introduzimos camadas, zoom ou ferramentas adicionais quando resolverem um problema real.
