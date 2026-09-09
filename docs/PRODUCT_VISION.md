# ASCENDENDO — Visão do produto

> Documento canónico de visão. Define porque o ASCENDENDO existe, que jogo queremos construir e quais propriedades do produto são deliberadas. Decisões funcionais mais granulares continuam em `docs/PRODUCT_DECISIONS.md`; ordem de implementação em `docs/ROADMAP.md`.

## 1. O que é o ASCENDENDO

ASCENDENDO é um **precision / masochist vertical platformer** construído sobre um motor 2D próprio.

A experiência principal é deliberadamente simples: o jogador sobe, escolhe onde quer aterrar, carrega a força de um salto e compromete-se com a trajectória. A dificuldade deve nascer da precisão, do desenho das plataformas, da leitura espacial, da repetição e da aprendizagem — não de uma acumulação artificial de mecânicas.

O projecto tem dois produtos inseparáveis:

```text
JOGO
  + AUTORIA
  + PLAYTEST
  + VALIDAÇÃO
  + DETERMINISMO / REPLAY
  + CAMPANHAS
  + COMUNIDADE
  + ASSETS / LICENCIAMENTO
  + DISTRIBUIÇÃO
  = ASCENDENDO
```

O editor não é apenas uma ferramenta interna. É infraestrutura do produto. A vida do jogo deve poder continuar através de níveis e campanhas criados por outras pessoas, sem exigir que a equipa original valide ou reconstrua manualmente cada criação.

## 2. Fantasia e filosofia do jogador

O jogador deve sentir que cada salto é uma decisão.

A experiência pretendida é:

```text
ler o espaço
→ escolher alvo
→ carregar força
→ libertar
→ observar a trajectória
→ aterrar ou falhar
→ aprender
→ tentar novamente
```

O jogo é difícil de propósito. Falhar faz parte do ciclo. Não queremos que o jogo esconda a dificuldade ou a corrija automaticamente.

A filosofia é **mastery over mechanics**: poucas regras bem definidas, muita profundidade na execução.

## 3. Mecânica base — Commitment Jump

A mecânica base actualmente aprovada é:

- salto parabólico;
- ângulo base de `60°`;
- nenhum controlo aéreo no modo/base principal;
- força determinada pelo tempo de carregamento antes da libertação;
- lançamento comprometido depois da libertação;
- feedback visual contínuo da força carregada.

### Carga do salto — decisão para 1.0

Enquanto o botão de Jump estiver premido, a força **sobe continuamente até ao máximo**. A carga não volta ao início e não faz ciclo.

Para 1.0, o crescimento de carga será **linear**. A razão é de design: num jogo de precisão, uma relação simples entre tempo de pressão e força facilita aprendizagem, memória muscular e diagnóstico. A função de carga deve, no entanto, ficar parametrizada para permitir avaliar mais tarde uma curva não-linear (por exemplo logarítmica/ease-out) sem alterar o contrato do input.

A física mantém precisão interna suficiente para não ficar dependente de uma representação inteira. O valor `0–255` é uma **representação visual normalizada** da carga para UI, não a resolução da simulação.

### Barra de força

A barra deve:

- acompanhar visualmente o personagem;
- comunicar claramente a carga actual;
- usar um gradiente de cor associado à intensidade;
- mostrar `0–255` como valor auxiliar quando houver espaço suficiente;
- evitar tapar informação importante do percurso;
- mover-se para uma posição alternativa pequena quando a posição normal acima do personagem prejudicar a leitura.

Não é obrigatório manter sempre texto numérico; o gradiente + barra devem continuar a comunicar a carga sem depender exclusivamente de cor ou número.

## 4. Movimento e bindings

O jogador começa com bindings familiares de jogos de plataforma:

- `A` / `←` — esquerda;
- `D` / `→` — direita;
- `Space` — Jump.

No produto final os jogadores devem **poder escolher livremente as teclas para as acções disponíveis**. Os defaults acima servem como ponto de partida, não como contrato rígido.

Os bindings devem ser armazenados como configuração do utilizador e a UI de Controlos deve mostrar os valores actuais.

## 5. Unidade fundamental — uma tela de nível

Um `.lvl` corresponde exactamente a **uma tela lógica de `640×360`**.

Esta é uma decisão de design do jogo, não apenas uma limitação técnica:

- uma tela é pequena o suficiente para ser lida mentalmente;
- uma tela pode ser testada isoladamente;
- cada transição pode ser estudada como uma unidade de percurso;
- campanhas podem construir progressão através da sequência de telas;
- o autor não precisa de gerir um mundo 2D arbitrariamente grande dentro do Level Editor.

O Level Editor não ganha pan/scroll ilimitado para compensar uma tela demasiado grande. Se mais espaço for necessário para o desafio, acrescenta-se outra tela à campanha.

## 6. Conteúdo de gameplay de 1.0

Para 1.0 o conjunto de entidades jogáveis é deliberadamente mínimo:

- plataformas estáticas;
- personagem/spawn;
- FLAG/objetivo final.

Não entram em 1.0 como mecânicas obrigatórias perigos, plataformas móveis, triggers, inimigos, interruptores, física especial ou outros sistemas equivalentes.

O modelo e o formato devem, contudo, permanecer **extensíveis** para permitir conteúdo desse tipo em 1.1+ sem invalidar níveis antigos ou tornar a autoria actual descartável.

A regra é:

```text
1.0 = pequeno conjunto de regras + alta profundidade
1.1+ = extensão aditiva quando houver uma razão de design real
```

## 7. Level Editor

O Level Editor vive no próprio jogo e edita exactamente uma tela `640×360`.

O fluxo principal é:

```text
editar
→ testar imediatamente
→ observar a execução
→ regressar ao editor
→ corrigir
→ validar
→ guardar
```

O editor deve permitir em 1.0:

- criar/mover/remover plataformas;
- snap previsível;
- editar o spawn dentro das regras seguras da plataforma inicial;
- colocar/remover a FLAG apenas quando o nível for o nível final da campanha;
- testar sem persistir automaticamente alterações experimentais;
- validar antes de tornar o estado persistido apto para publicação;
- guardar o formato de nível suportado pelo jogo.

O editor conserva UI de desenvolvedor suficiente para diagnóstico, estado de validação e informação técnica. Não deve esconder informação necessária para criar e depurar níveis.

## 8. Carrinho de alterações / estado não guardado

O estado editado e o estado persistido são conceitos separados.

Ao sair do editor com `Esc`, o jogador deve conseguir decidir o destino das alterações através de uma UI semelhante a um **carrinho de compras**:

```text
ALTERAÇÕES PENDENTES
- mover plataforma A
- criar plataforma B
- apagar plataforma C
- alterar spawn

[Guardar todas] [Descartar todas] [Rever alterações]
```

A revisão deve permitir desfazer algumas alterações e manter outras quando isso for tecnicamente suportado pelo modelo de edição. A intenção é evitar o falso binário “guardar tudo / perder tudo”.

## 9. Campaign Editor

O Campaign Editor é diferente do Level Editor.

Ele apresenta os níveis como uma **playlist / timeline vertical** de blocos compactos com proporção `16:9`. A posição vertical representa a ordem da campanha.

A campanha deve suportar:

- seleccionar nível;
- reordenar níveis;
- abrir o nível seleccionado no Level Editor;
- voltar ao Campaign Editor preservando o contexto;
- validar todas as referências;
- guardar a ordem em `campaign.txt`;
- manter o routing físico de ficheiros sob responsabilidade de `reorganize.py`.

### Criação e remoção de entradas

A direcção 1.0 pode começar com `select / reorder / open`, mas a decisão de permitir criar/remover níveis directamente a partir do Campaign Editor fica deliberadamente em **pesquisa UX**.

A razão é que esta é uma questão de workflow, não de arquitectura: referências a platformer makers mostram que criar, testar, organizar e publicar são workflows recorrentes, mas isso não prova que adicionar/remover uma entrada nesta UI seja melhor para o ASCENDENDO. A decisão será tomada a partir de referências e, quando possível, playtesting com utilizadores. citeturn967301search0turn967301search1turn800299search17

## 10. Identidade de campanhas e níveis

O produto deve preparar desde cedo uma identidade persistente para conteúdo partilhado.

Uma campanha deve ter pelo menos:

- `account/creator`;
- `campaign name` legível;
- identificador único estável;
- versão;
- número de níveis;
- informação de dificuldade;
- tempos/runs agregados quando existirem;
- estado de validação;
- metadata necessária para publicação.

Duas campanhas do mesmo criador **não devem poder ter o mesmo nome público**. O identificador interno continua a existir para garantir unicidade mesmo quando os nomes mudarem ou forem importados.

Não dependemos de thumbnails para distinguir campanhas em 1.0. A UI deve distinguir claramente por **nome + conta + metadados**. Thumbnails ficam como capacidade futura, não requisito para o lançamento.

## 11. Campanhas oficiais de 1.0

O mínimo de 1.0 é **10 campanhas**, podendo existir mais:

| Quantidade mínima | Níveis por campanha |
|---:|---:|
| 1 | 10 |
| 4 | 25 |
| 3 | 50 |
| 1 | 100 |
| 1 | 250 |

Total mínimo: **575 níveis oficiais**.

A cardinalidade é deliberadamente ampla para permitir progressão longa e campanhas de escalas diferentes. Os números são mínimos, não limites.

### Curva de dificuldade

Cada campanha deve ter uma progressão aproximadamente linear de dificuldade:

```text
início → fácil / introdução
        ↓
meio → domínio / aprendizagem
        ↓
fim → difícil / extremo
```

Campanhas grandes podem ser organizadas em blocos internos; por exemplo, os últimos cinco níveis podem formar um bloco extremo.

A selecção global de campanhas deve aproximar-se de uma distribuição centrada no **normal**, com menos campanhas muito fáceis e muito extremas. Isto é uma orientação de catálogo, não uma exigência estatística rígida.

O jogo **não** utiliza adaptive difficulty nem assistência automática. A alternativa à dificuldade excessiva é oferecer campanhas de outras dificuldades.

## 12. Validação de campanhas e testador de dificuldade

Para 1.0 a validação de campanha é obrigatória.

Além de validade sintáctica e geométrica, cada campanha deve passar por um **testador de dificuldade / percurso** antes de ser considerada pronta para distribuição/publicação.

As camadas devem permanecer separadas:

```text
formato válido
→ geometria válida
→ regra física válida
→ campanha conectada
→ percurso/análise de dificuldade
→ experiência humana
```

O testador de dificuldade não substitui o playtest humano e não altera automaticamente o nível para “facilitar” a experiência.

Um nível válido pode ser extremamente difícil; isso é uma propriedade possível do produto e não um erro.

## 13. Diagnóstico de percurso

Quando um percurso de validação falhar, o sistema deve mostrar, quando possível:

- tentativa/trajectória;
- ponto aproximado da falha;
- nível onde ocorreu;
- transição entre níveis quando aplicável;
- primeira causa útil conhecida.

Exemplos:

- alvo demasiado longe;
- ângulo impossível;
- plataforma fora da janela alcançável;
- colisão lateral;
- sequência seguinte inacessível.

No Campaign Editor, isto deve permitir perceber se o defeito é do nível A, da transição A→B ou do nível B.

## 14. Determinismo, replay e runs

O motor usa fixed timestep e o projecto deve preservar reprodutibilidade suficiente para:

- reproduzir falhas;
- comparar versões;
- validar níveis;
- reproduzir runs;
- produzir capturas determinísticas;
- analisar evolução de desempenho.

**Leaderboards e histórico completo de runs são desejados**, mas podem ficar depois de 1.0 se o custo ameaçar atrasar o núcleo jogável/editorial.

O replay, quando disponibilizado, deve ser reproduzível pelo próprio motor e não por uma gravação de vídeo.

## 15. Som e música

O áudio faz parte do produto 1.0, mas de forma simples.

Necessidades mínimas:

- música de fundo simples com estética retro, utilizando material devidamente livre para distribuição;
- efeitos de salto/carga/libertação;
- aterragem;
- falha/morte;
- conclusão de nível;
- UI/editor quando relevante;
- ambiente simples quando ajudar a atmosfera.

Para reduzir repetição, os efeitos não-musicais devem poder existir em **vários takes/variações e uma escolha aleatória controlada**. A música é a excepção e deve permanecer estável enquanto estiver a tocar.

## 16. Direcção visual

A estética é **indie, simples, pixel-art básica**, usando o repositório actual como ponto de partida, mas sem procurar uma produção visual excessivamente complexa.

A prioridade é:

```text
gameplay readability
    > decoration
```

### Composição

- **foreground:** personagem e plataformas jogáveis;
- **background:** atmosfera e profundidade;
- **parallax:** várias camadas de fundo com velocidades relativas diferentes.

O parallax deve reforçar a sensação de subida sem dificultar a leitura da trajetória.

## 17. Tiles, objectos e escala visual

Os elementos visuais base são construídos a partir de peças **16×16**.

Isto não significa que o jogador ou o editor tenham de trabalhar numa grelha de colocação de 16×16.

A distinção é:

```text
asset/tile → 16×16
objecto → uma ou várias peças 16×16
colocação no nível → pixel-perfect / sem grelha rígida
```

Assim, uma plataforma pode ser composta por várias peças 16×16 e posicionada na coordenada exacta necessária ao design do salto.

## 18. Sprites e conteúdo visual controlado

As opções de sprite devem ser **curadas**, não um editor de arte livre.

Isto aplica-se ao protagonista, plataformas/chão e futura decoração.

A escolha de sprite é visual; não deve alterar silenciosamente a colisão ou as regras físicas.

A criação/edição de arte dentro do jogo fica fora do núcleo 1.0.

## 19. Licenciamento

Todo asset externo que entre no produto deve possuir uma **ficha formal de origem/licença no repositório**.

Preferências:

- arte própria;
- CC0 quando adequado;
- licenças claramente compatíveis com a distribuição pretendida;
- origem registada mesmo em recursos CC0.

Evitar:

- assets pagos sem direitos de redistribuição adequados;
- assets de origem desconhecida;
- “free” sem licença verificável;
- música ou efeitos cuja licença impeça a distribuição final.

## 20. Partilha e comunidade

A comunidade é parte do produto, não um extra tardio.

A direcção 1.0 inclui **importar e exportar campanhas**.

A partilha de `.lvl` isolados é tecnicamente possível, mas não é ainda a principal unidade de publicação porque um nível tem pouca diversidade de blocos e, isoladamente, oferece menos contexto do que uma campanha. Esta questão fica como decisão de design explícita e deve ser debatida à medida que o catálogo visual crescer.

A hipótese actual para publicação é:

```text
Campanha
  + identidade do criador
  + nome/ID
  + níveis
  + dificuldade
  + validação
  + metadados
```

Um `.lvl` individual pode existir como unidade técnica de transporte/importação mesmo que a experiência de publicação privilegie campanhas.

O site futuro deve poder:

- mostrar campanhas;
- mostrar criador;
- mostrar dificuldade;
- mostrar tempos/runs quando disponíveis;
- mostrar rankings;
- permitir download/importação;
- manter histórico/versionamento onde for útil.

## 21. Site, ranking e versão web

A distribuição final desejada tem três camadas:

1. **aplicação/executável principal:** obrigatório;
2. **site público para partilha/rankings:** objectivo de 1.0;
3. **versão web jogável:** desejável, mas **não obrigatória** para 1.0.

A versão Windows x64 standalone é a referência principal do produto.

## 22. Segurança e validade de conteúdo importado

Todo conteúdo importado ou descarregado deve ser validado novamente pelo próprio executável antes de poder ser jogado.

A autoridade é:

```text
ficheiro externo
→ importação
→ validação local
→ conteúdo jogável
```

Não confiamos no servidor para declarar que um mapa é válido só porque foi publicado.

## 23. Dificuldade, desempenho e experiência

O projecto deve distinguir:

```text
validade física
        ↓
dificuldade do conteúdo
        ↓
desempenho observado
        ↓
dificuldade percebida / experiência
```

O catálogo resolve dificuldade de produto através da oferta de campanhas diferentes, e não através de alteração automática do nível em resposta ao jogador.

O futuro sistema de dificuldade deve ajudar sobretudo o autor e o processo de curadoria.

## 24. Pesquisa ainda aberta

Algumas decisões ficam abertas por serem questões de produto/UX e não contratos técnicos:

- criar/remover entradas directamente no Campaign Editor;
- se a campanha deve ser sempre a unidade pública principal ou se um futuro level-pack deve substituí-la;
- formato exacto da metadata pública e dos rankings;
- quantidade e estrutura dos dados apresentados sobre tempos/dificuldade na selecção de campanha;
- eventual curva de carga não-linear depois de experiência real com a versão linear;
- mecânicas adicionais de 1.1+;
- dimensão do corpus comunitário necessário para justificar funcionalidades de descoberta mais sofisticadas.

## 25. Definição de sucesso de 1.0

1.0 é o produto em que um utilizador consegue:

```text
abrir a aplicação
→ escolher uma das campanhas
→ compreender/remapear os controlos
→ carregar e executar saltos precisos
→ progredir por uma campanha vertical
→ perceber claramente a força do salto
→ terminar a campanha
→ abrir o Level Editor
→ editar uma tela
→ testar sem persistir automaticamente
→ rever alterações
→ validar
→ guardar
→ abrir Campaign Editor
→ ordenar a campanha
→ importar/exportar
→ voltar a jogar
```

E em que a equipa consegue:

```text
criação/importação
→ validação de campanha
→ testador de dificuldade
→ diagnóstico
→ publicação/partilha
→ run/replay quando suportado
→ evolução futura
```

sem quebrar o conteúdo existente.
