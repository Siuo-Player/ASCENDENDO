# Editor de níveis — Fase 9

O editor é parte do jogo e corre no mesmo processo, através de `GameState::EDITOR`. Não é uma aplicação externa.

## Princípios

1. **Grid-first**: todas as operações espaciais relevantes passam pelo mesmo snap.
2. **Poucas operações, atalhos fortes**: a grelha reduz combinações possíveis; os bindings permitem reutilizar as mesmas ações sem hardcode.
3. **Dados compactos**: entidades guardam geometria e referências, não cópias de texturas.
4. **Sem estados inválidos apresentados**: a UI bloqueia posições fora da área editável sempre que isso puder ser determinado localmente.
5. **Undo/redo pode vir depois**: 9.4 não deve introduzi-lo antes de o modelo básico de edição estar estável.

## Fluxo de interação 9.4

### Plataforma

Existem dois modos.

**STAMP** — modo default. Um clique esquerdo num espaço vazio cria uma plataforma de tamanho médio, alinhada ao grid.

**DRAG** — clique e arrasto define diretamente a área da plataforma; o retângulo inicial e final são quantizados ao grid.

`G` alterna entre os modos através de `GameAction`/`KeyBindings`.

Presets de tamanho pequeno/médio/grande são acessíveis através de ações de binding próprias. O tamanho médio é o default para minimizar atrito ao começar a construir.

Clicar/arrastar uma plataforma existente deve selecionar/mover a entidade em vez de criar automaticamente uma duplicada. O comportamento exato é determinado pelo alvo sob o cursor: entidade encontrada → edição; vazio → criação.

### Apagar

A mesma ação lógica `DeleteSelection` terá duas entradas:

- botão direito do rato;
- `Delete`/`Backspace`.

O editor não deve depender apenas de uma delas. Isto permite que a interação funcione bem tanto com rato como com teclado.

### Spawn

O spawn é uma entidade especial, não uma plataforma normal.

- Y não é escolhido livremente;
- Y deriva do topo do chão do primeiro nível;
- apenas X é editável;
- X é quantizado ao grid;
- X é limitado à região segura definida pelo chão inicial e pelo corpo do jogador.

Não existe opção de selecionar posições fora dessa faixa.

### FLAG

A FLAG só pode ser criada/editada quando o documento corresponde ao último nível da campanha.

Deve existir no máximo uma FLAG na campanha. A organização do documento não deve permitir criar uma FLAG de final em cada tela.

## Key bindings

Todas as operações que representem intenção do utilizador devem ser `GameAction`, não `Key::X` diretamente.

A tabela abaixo é o plano funcional; a tecla é apenas o default:

| Ação | Default | Função |
|---|---|---|
| `MoveLeft` | A / LEFT | pan horizontal esquerdo |
| `MoveRight` | D / RIGHT | pan horizontal direito |
| `EditorPanUp` | W | pan vertical para cima |
| `EditorPanDown` | S | pan vertical para baixo |
| `EditorToggleMode` | G | STAMP ↔ DRAG |
| `EditorSizeDown` | [ | preset menor |
| `EditorSizeUp` | ] | preset maior |
| `DeleteSelection` | DELETE / BACKSPACE | apagar |
| `UIConfirm` | SPACE | confirmar ação/contexto |
| `Pause` | ESCAPE | sair do editor para MENU |

Os nomes persistidos em `controls.cfg` são estáveis; alterar um nome serializado exige migração.

## Grid

`EDITOR_GRID_SNAP` em `Config.h` é a fonte única do snap.

O valor atual de teste é `4.0f`. A grelha visual pode ser mais espaçada (`EDITOR_GRID_VISUAL_SPACING`) para não desenhar centenas de linhas por ecrã.

A diferença é intencional:

- **visual spacing** = informação para o olho;
- **snap** = restrição matemática da edição.

## Modelo de dados e GPU

O editor não deve criar uma textura para cada plataforma. A forma preferida é:

```text
LevelEntity
  geometry
  type
  spriteId/reference (quando aplicável)
```

No renderer, vários `spriteId`s iguais devem partilhar o mesmo recurso GPU. Para arte pixelada, um atlas pequeno é preferível a múltiplas texturas independentes quando a quantidade de sprites crescer.

## Estados e transições

O editor pode ser aberto:

- pelo item `EDITOR` no MENU;
- pela ação dedicada `OpenEditor`.

No futuro, o Editor de Campanha também pode abrir diretamente um nível e mudar para `GameState::EDITOR`, passando o documento selecionado ao editor.

ESC regressa ao `MENU`; não inicia gameplay nem altera a campanha sozinho.

## Critérios de conclusão de 9.4

A fase só está concluída quando:

- STAMP funciona;
- DRAG funciona;
- mover entidade existente funciona;
- apagar funciona por rato e teclado;
- snap é aplicado em todas as operações espaciais;
- posições inválidas não aparecem como opção;
- spawn respeita as restrições;
- FLAG respeita a regra de último nível;
- o estado do editor consegue representar estas entidades sem hacks visuais;
- existe cobertura de testes para a lógica editável, sem depender de uma GPU real.