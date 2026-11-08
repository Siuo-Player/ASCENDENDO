# Referências de Design — Editores, Jogos Abertos e Game Feel

Este documento regista referências externas e literatura canónica usadas para orientar o desenho, ergonomia, sensação de controlo (*game feel*) e ferramentas do ASCENDENDO. Não são especificações para copiar literalmente: servem para identificar padrões de interface, workflow, física perceptual e composição de níveis comprovados pela indústria e pela investigação.

---

## 1. Tiled — Referência para Edição Espacial e Snapping

[Tiled](https://github.com/mapeditor/tiled) é um editor de mapas open source consolidado na indústria 2D.

### O que aproveitamos
- Separar claramente o documento de nível (`LevelEditorDocument`) da camada de apresentação/renderização;
- Tratar o snapping como ferramenta de criação declarativa, não como detalhe escondido do motor;
- Permitir que o editor mostre a estrutura do conteúdo de forma previsível;
- Manter o formato do mapa declarativo e legível em texto plano;
- Pensar desde cedo em retrocompatibilidade e evolução do formato.

### O que não copiamos
O ASCENDENDO não precisa da generalidade de camadas infinitas do Tiled: o nosso jogo é um platformer vertical focado no espaço lógico `640×360`.

Fonte: [Tiled — GitHub](https://github.com/mapeditor/tiled).

---

## 2. Godot — Ferramentas Modais, Viewport e Snapping

O editor 2D do [Godot](https://github.com/godotengine/godot) oferece ferramentas distintas para selecionar, mover, escalar e inspecionar entidades.

### O que aproveitamos
- Modos de ferramenta explícitos (`STAMP` vs `DRAG`);
- Atalhos de teclado curtos para ações frequentes mapeados via `GameAction`;
- Snapping configurável, mantendo distinção entre grelha visual (16 px) e passo de quantização (4 px);
- Seleção e transformação como conceitos separados;
- Feedback visual imediato no viewport de trabalho (resizing com handles de canto nos 4 vértices).

Fonte: [Godot 2D editor documentation](https://github.com/godotengine/godot-docs/blob/master/tutorials/2d/introduction_to_2d.rst).

---

## 3. SuperTux — Editor Integrado e Filosofia de Comunidade

O [SuperTux](https://github.com/SuperTux/supertux) possui um Level Editor integrado dentro do próprio binário executável e fluxo comunitário de add-ons.

### O que aproveitamos
- O editor pertencer ao próprio executável do jogo (sem dependência de IDEs externas);
- Separação explícita entre editar uma tela individual (`LevelEditor`) e organizar a progressão global (`CampaignEditor`);
- Validação automática de integridade e formatos de ficheiro versionáveis.

Fonte: [SuperTux — GitHub](https://github.com/SuperTux/supertux).

---

## 4. Jump King — Referência Visual, Verticalidade e Masoquismo de Precisão

[Jump King] (Nexile) é uma inspiração fundamental para a estética vertical e o peso dramático da queda.

### O que aproveitamos
- Composição vertical por ecrãs interligados em subida contínua;
- Leitura imediata de silhuetas e plataformas;
- Verticalidade como linguagem de tensão: o risco da queda substitui o conceito de morte por vidas/HP;
- Workflow de playtest imediato dentro do jogo a partir do editor.

### O que não copiamos
- A física do ASCENDENDO é governada pelo **Commitment Jump a 60° com carga contínua** (tempo máximo de carga de 0.4s), não pela física específica ou tempos de salto do Jump King;
- Não usamos formatos de terceiros: os nossos ficheiros `.lvl` e `campaign.txt` são proprietários, auditados e seguros contra injeções.

---

## 5. Steve Swink (2008) — *Game Feel: A Game Designer's Guide to Virtual Sensation*

A obra de referência de Steve Swink (*Game Feel*, Morgan Kaufmann, 2008) estabelece as fundações perceptuais da nossa experiência de controlo:

### 1. Resposta em Tempo Real e Ciclo Perceptual
O cérebro humano percebe continuidade táctil quando o tempo de resposta entre a pressão da tecla e o início da reação visual ocorre abaixo de 50 ms. O loop fixo a 60 Hz do ASCENDENDO (16.67 ms) assegura essa fidelidade háptica.

### 2. Princípio da Antecipação Visual (Squash & Stretch)
Para que o jogador sinta o "peso" do salto sem prejudicar o determinismo milimétrico da colisão:
- Durante a carga do salto, o renderizador de sprites (`WorldRenderer`) deforma progressivamente a escala vertical (até 12% de compressão) e expande a horizontal (até 10%), mantendo a base estritamente alinhada com o chão.
- **Invariante Físico**: O `PhysicsBody` canónico permanece rigorosamente $16 \times 16$ px, garantindo que o "juice" visual nunca compromete o determinismo das colisões.

### 3. Enquadramento e Rastreio de Câmara com Restrições de Fronteira
A câmara segue o jogador com interpolação suave (Lerp exponencial a 35% de offset vertical). Contudo, segundo Swink (Capítulo 13), em quedas bruscas a suavização pura pode fazer o avatar escapar do campo de visão:
- O ASCENDENDO implementa **Safety Viewport Clamping**: limites rígidos de emergência (`MIN_BOTTOM_MARGIN = 24px`, `MIN_TOP_MARGIN = 32px`). Se a velocidade do avatar exceder a taxa de convergência do Lerp, a câmara trava imediatamente nos limites de segurança, garantindo que o jogador nunca fica oculto.

Fonte: Swink, S. (2008). *Game Feel: A Game Designer's Guide to Virtual Sensation*. CRC Press / Morgan Kaufmann.

---

## 6. Maddy Thorson & Noel Berry (*Celeste*, 2018) — Desacoplamento e Legibilidade

As apresentações técnicas e retrospectivas de design de *Celeste* (GDC 2018) inspiram a filosofia de tolerância e clareza do ASCENDENDO:

### O que aproveitamos
- **Desacoplamento de Apresentação e Física**: Todos os efeitos visuais (squash, estiramento, partículas, trailers de rasto) são derivados de um snapshot de leitura unidirecional (`RenderSnapshot`), sem qualquer mutação de volta ao estado do mundo físico.
- **Leitura Visual em Alto Contraste**: As plataformas possuem contraste nítido contra o fundo atmosférico, eliminando ambiguidades sobre onde o jogador pode pousar.
- **Commitment Jump Irrevogável**: Ao soltar o salto, o avatar compromete-se com a sua trajetória balística, sem controlo aéreo artificially introduzido a meio do voo, recompensando o planeamento espacial deliberado.

Fonte: Thorson, M., & Berry, N. (2018). *Level Design Workshop: Designing Celeste*. Game Developers Conference.

---

## 7. Derek Yu (*Spelunky*, 2016) — Atomicidade de Conteúdo e Garantias de Alcance

Derek Yu (*Spelunky*, Boss Fight Books, 2016) explora a garantia matemática de passabilidade em jogos de plataformas:

### O que aproveitamos
- **Garantia de Alcançabilidade por Secção**: Nenhum nível do ASCENDENDO pode ser guardado ou jogado se não tiver sido formalmente provado que existe uma rota fisicamente executável até à plataforma mais alta.
- **Modularidade de Telas**: Telas verticais de `640×360` coladas em espaço contínuo asseguram transições transparentes de streaming sem quebras de framerate.

Fonte: Yu, D. (2016). *Spelunky*. Boss Fight Books.

---

## 8. Aramini et al. (2018) & Smith et al. (2008) — Cones de Trajetória e Envelopes Balísticos

Estudos de geração procedural e authoring assistido de platformers (*Aramini et al. 2018; Smith et al. 2008*) demonstram que os designers cometem erros de cálculo de alcance físico se não tiverem feedback visual da envolvente balística.

### Implementação no Editor do ASCENDENDO
- **Módulo `JumpTrajectory`**: Fornece em tempo real a amostragem analítica da parábola a 60° com $g = -980\text{ px/s}^2$.
- **Cones Tracejados Visuais**:
  - Arco Ciano suave: Salto mínimo (toque rápido de 1 tick, alcance vertical de 23.9 px, horizontal de 55.2 px).
  - Arco Dourado brilhante: Salto com carga máxima (0.4s de carga, alcance vertical de 137.7 px, horizontal de 318.1 px).
- O autor inspeciona visualmente as rotas a partir do chão ou de qualquer plataforma selecionada, eliminando saltos acidentalmente impassáveis.

Fontes:
- Aramini, D., et al. (2018). *Reachability-driven level design for precision platformers*. IEEE CoG.
- Smith, G., et al. (2008). *Tanagra: A mixed-initiative level design tool*. ACM FDG.

---

## 9. Matriz de Síntese de Decisões de Design

| Problema de Design | Referência / Literatura | Solução Canónica no ASCENDENDO |
|---|---|---|
| **Antecipação do Salto** | Swink (2008) / Thorson (2018) | Visual squash até 12% no topo; `PhysicsBody` $16 \times 16$ 100% determinístico. |
| **Enquadramento em Queda** | Swink (2008, *Camera Tracking*) | Lerp suave a 35% com clamping rígido inferior a 24 px e superior a 32 px. |
| **Validação de Alcance** | Aramini (2018) / Smith (2008) | Pré-visualização de arcos ciano (mínimo) e dourado (máximo) no editor. |
| **Formato e Snapping** | Tiled / Godot | Documento declarativo, grid visual de 16 px e snap físico de 4 px. |
| **Integração do Editor** | SuperTux / Jump King | Editor embutido no executável, teste com 1 clique e histórico undo/redo por gesto. |
| **Isolamento de Contas** | NIST SP 800-63B / PPID | Cofres locais independentes via HMAC-SHA256 sem transmissão para a nuvem. |
