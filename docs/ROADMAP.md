# Roadmap de desenvolvimento — ASCENDENDO

> Documento operacional canónico. A história detalhada pertence ao Git e aos registos históricos. Este ficheiro descreve o motivo do projecto, o estado actual, a ordem de execução, o que falta e porque cada etapa deve acontecer no momento indicado.
>
> **Visão canónica do produto:** `docs/PRODUCT_VISION.md`  
> **Decisões funcionais canónicas:** `docs/PRODUCT_DECISIONS.md`  
> **Especificação UX do editor:** `docs/EDITOR_UX_SPEC.md`

## 1. Porque existe o projecto

ASCENDENDO é um **Vertical Precision Platformer** construído sobre um motor 2D próprio. O projecto não pretende ser apenas uma demonstração técnica nem apenas uma campanha fixa: pretende resultar num jogo de plataforma vertical preciso, difícil de dominar, acompanhado por um sistema de autoria que permita continuar a produzir e validar conteúdo sem depender permanentemente da equipa original.

O produto tem, portanto, dois lados inseparáveis:

```text
jogo
  + editor
  + playtest
  + validação
  + determinismo/replay
  + gestão de campanhas
  + pipeline de assets
  + distribuição
  = ASCENDENDO
```

### O motivo central

A experiência de jogo é baseada em **compromisso**: o jogador lê o espaço, carrega a força de um salto, decide quando libertar e aceita a trajectória escolhida. A simplicidade mecânica é intencional; o desafio deve nascer da precisão, do desenho do nível e da aprendizagem do movimento, não de uma quantidade arbitrária de sistemas.

O editor existe porque a vida útil do jogo não deve acabar quando os mapas iniciais acabam. Um nível criado pela comunidade precisa de poder passar pelo mesmo ciclo de qualidade que um nível oficial:

```text
criar
→ testar
→ diagnosticar
→ validar
→ guardar
→ integrar na campanha
→ jogar novamente
```

Confiabilidade, verificabilidade e determinismo são, por isso, requisitos de produto e não apenas detalhes internos de engenharia.

## 2. Princípios que governam o roadmap

Toda a evolução deve seguir:

```text
investigar
→ documentar a decisão
→ escolher a menor tranche suficiente
→ implementar
→ testar
→ validar em CI
→ actualizar a documentação activa
→ avançar
```

Uma propriedade só muda de estado quando existe evidência adequada. “O código parece correcto” não é o mesmo que “a propriedade foi validada”.

Não se adicionam mecânicas ou abstrações apenas porque parecem interessantes. A ordem privilegia primeiro o que fecha o produto já definido ou remove uma dependência real para o próximo passo.

---

# 3. Ordem global do produto

## Fase A — Fundação e confiança do motor ✅ FECHADA

### O que foi feito

A fundação consolidou os contratos críticos necessários para o crescimento do projecto: ownership de estado, fronteiras entre Core/Logic/Presentation, input semântico, validação de geometria, comportamento fail-closed, lifecycle gráfico e evidência multiplataforma.

### Porque veio primeiro

Sem uma base estável, alterações de editor ou gameplay poderiam introduzir regressões difíceis de localizar e tornar os resultados de testes pouco confiáveis. A fundação cria as condições para que o restante do roadmap possa avançar sem voltar repetidamente às mesmas questões estruturais.

### Estado

**FOUNDATION — CLOSED.**

O estado fechado só é mantido enquanto a evidência operacional continuar válida. Não deve ser reaberta apenas para encaixar features futuras.

---

# 4. Fase 9 — Editor de níveis visual 🔄 ACTIVA

A Fase 9 transforma o editor em parte real do produto. O editor vive dentro do próprio executável, usa o mesmo sistema de input/rendering e deve permitir fechar o ciclo de autoria sem criar uma ferramenta paralela.

## 9.1 — Controlos e input semântico ✅ NÚCLEO IMPLEMENTADO

### O que fazemos

As acções são representadas semanticamente através de `GameAction`/`KeyBindings`, em vez de a lógica do editor depender directamente de teclas físicas.

### Porque foi necessário nesta posição

O editor precisa de crescer para várias ferramentas e modos. Uma dependência directa em teclas tornaria a UX difícil de remapear, testar e manter. A fronteira semântica foi portanto estabelecida antes da expansão das ferramentas.

### O que ainda falta

Um ecrã visual de **Controlos / Key Bindings**, descobrível dentro do jogo, onde o utilizador possa consultar as acções e remapeá-las sem documentação externa.

---

## 9.2 — Rato e viewport ✅ IMPLEMENTADO

### O que fazemos

Existe estado de cursor e botões, conversão determinística de coordenadas janela → espaço lógico com letterbox e hit-testing de UI.

### Porque veio antes da edição visual

Toda a edição com rato depende de o cursor físico mapear exactamente para o espaço lógico do nível. Se esta fronteira estiver errada, snapping, selecção e arrasto ficam errados simultaneamente.

### Contrato

O canvas do editor continua a ser `640×360`. Áreas de letterbox não são áreas editáveis.

---

## 9.3 — Estado EDITOR e infraestrutura ✅ IMPLEMENTADO / REVISÃO VISUAL PENDENTE

### O que fazemos

Existe `GameState::EDITOR`, grid visual, acesso pelo menu e separação entre documento de edição e snapshot consumido pela presentation.

### Porque veio antes das ferramentas

O editor precisa de um espaço de execução isolado do gameplay. Assim, adicionar uma ferramenta nova não exige misturar física/render/input do jogo principal com a lógica de autoria.

### Pendente

Confirmação visual humana do layout/menu/editor. Não deve ser fechada apenas alterando constantes sem observar o resultado final.

---

## 9.4 — Manipulação de entidades ✅ IMPLEMENTADO

### O que fazemos

O editor já suporta a base actualmente aprovada:

- criar, mover e apagar plataformas;
- snap determinístico;
- spawn com Y fixo e X limitado à plataforma inicial segura;
- FLAG apenas no nível final da campanha;
- cancelamento transaccional de movimentos;
- snapshot separado para rendering.

### Porque veio antes de persistência

Primeiro é necessário demonstrar que o conteúdo pode ser alterado correctamente em memória. Persistir antes de estabilizar essa representação apenas tornaria os erros mais permanentes.

### Limite importante

O Level Editor é de **uma tela 640×360**. Não deve ganhar pan ilimitado apenas para acomodar conteúdo que deveria ser composto em múltiplos níveis.

---

## 9.5 — Guardar e validar ✅ VERTICAL SLICE FECHADA

### O que fazemos

- serialização no `.lvl` já existente;
- validação antes de escrever;
- substituição através de ficheiro temporário;
- validação assíncrona sobre snapshot imutável;
- geração do documento para detectar resultados obsoletos;
- integração de guardar/validar através de acções semânticas.

### Porque veio antes da gestão de campanha

Um editor que só cria estado efémero ainda não constitui um pipeline de autoria. O nível precisa de sair do editor de forma segura e verificável antes de a campanha poder tratá-lo como conteúdo persistente.

### O que fica por exercitar

O fluxo de produto completo ainda precisa de ser combinado com a gestão de campanha e com o playtest sem confundir estado persistido com alterações temporárias.

---

## 9.6 — Gestão de campanha 🔄 ACTIVA / INTEGRAÇÃO VISUAL POR CONCLUIR

### O que já existe

`CampaignEditorDocument` já implementa a base lógica de:

- carregar `campaign.txt`;
- representar níveis como blocos ordenados;
- seleccionar um nível;
- reordenar níveis;
- preservar a identidade da selecção durante reorder;
- rejeitar referências duplicadas;
- rejeitar níveis inexistentes;
- rejeitar path escape;
- guardar apenas a ordem canónica em `campaign.txt`;
- preservar a responsabilidade do routing físico em `reorganize.py`.

A selecção foi endurecida contra reordenações e contra falhas de load. Existe também uma API null-safe preparada para o consumo de UI.

### Porque esta é a prioridade actual

O Level Editor pode produzir uma tela isolada, mas o produto precisa de saber como essa tela entra na progressão. A campanha é a ponte entre **conteúdo individual** e **experiência jogável completa**.

### Próxima sequência de implementação

```text
CampaignEditorDocument
        ↓
CampaignEditorSnapshot
        ↓
UI visual de playlist/timeline
        ↓
selecção explícita
        ↓
drag/reorder
        ↓
abrir nível seleccionado
        ↓
salvar campaign.txt
        ↓
reabrir + validar
```

### Regras que não podem ser quebradas

`campaign.txt` é a autoridade da ordem.  
O editor organiza a campanha, não move ficheiros entre `Levels/`, `Unused/` e `NaoValidados/`.  
`reorganize.py` mantém o routing físico.  
O executável/validator continua a ser a autoridade final sobre validade jogável.

---

# 5. Fecho da Fase 9

A Fase 9 só deve mudar para `CLOSED` quando o fluxo completo de autoria estiver realmente exercitável e validado.

## 5.1 — Abrir níveis existentes no editor

O utilizador deve conseguir seleccionar um nível da campanha e passar desse nível para o Level Editor sem reconstruir manualmente o conteúdo.

**Porque depois de 9.6:** a campanha já é responsável pela selecção; reutiliza-se essa fonte de verdade em vez de criar uma segunda lista de níveis.

## 5.2 — Playtest não persistente

O autor deve poder testar o nível actualmente editado e regressar ao editor sem perder alterações não guardadas e sem transformar automaticamente o teste num save.

**Porque neste momento:** esta é a ligação natural entre 9.4 e 9.5. Só deve ser fechada quando o estado temporário e o estado persistido estiverem claramente separados.

## 5.3 — Selecção de campanha

Mesmo existindo uma única campanha, o modelo deve permitir seleccionar uma campanha explicitamente. Isso prepara o produto para múltiplas campanhas sem uma nova arquitectura.

**Porque depois de 9.6:** a noção de campanha e a sua ordenação já estarão estabilizadas.

## 5.4 — Controlos descobríveis

Adicionar a UI de consulta/remapeamento de bindings.

**Porque depois da estabilização do editor:** a lista de acções deve representar as acções finais realmente usadas pelo produto e não uma interface transitória.

## 5.5 — Revisão visual humana

Confirmar os layouts finais do menu/editor, legibilidade, enquadramento e ausência de clipping.

**Porque por último:** só vale a pena fechar a revisão visual quando o fluxo funcional que será observado já estiver completo.

### Critério final de saída da Fase 9

```text
seleccionar campanha
→ seleccionar nível
→ editar
→ testar sem persistir automaticamente
→ regressar
→ validar
→ guardar
→ reordenar campanha
→ reabrir
→ jogar
```

E todas as transições importantes precisam de testes/evidência apropriados.

---

# 6. Visual, arte e assets 🟡 PRÓXIMA CAMADA

Esta camada transforma o editor estrutural num sistema de produção visual. Deve vir depois do núcleo de autoria porque a arte precisa de um caminho estável para entrar no conteúdo.

## 6.1 — Identidade visual do jogo

### Regra principal

**Gameplay-first readability.** Personagem, plataformas, perigos e objectivos têm prioridade visual.

A composição desejada é:

```text
foreground
→ personagem + plataformas jogáveis

background
→ atmosfera + profundidade

parallax
→ movimento relativo das camadas de fundo
```

O fundo deve reforçar a sensação de subida e profundidade sem esconder informação necessária para executar saltos.

## 6.2 — Sprites controlados

O projecto suporta o sprite do protagonista e prevê escolhas controladas de sprites para personagem, plataformas/chão e decoração.

Trocar um sprite é uma decisão visual. Não deve alterar silenciosamente a colisão ou a física.

## 6.3 — Escala 16×16

Existe uma direcção de assets base de pequena escala, incluindo `16×16` quando apropriado.

Isto **não** significa que o posicionamento do nível tenha de ser preso a uma grelha rígida. O grid de autoria é uma ferramenta de alinhamento; não é uma mecânica visual obrigatória.

## 6.4 — Licenciamento

Preferência por:

- arte própria;
- CC0 ou licença claramente compatível;
- origem/licença registada mesmo em CC0;
- evitar assets pagos, de origem desconhecida ou com termos difíceis de distribuir.

Não transformar o jogo num editor de arte livre. A complexidade de direitos e manutenção não serve o objectivo actual.

## 6.5 — Background/parallax

Deve ser introduzido quando houver corpus de níveis suficiente para testar a regra visual em situações variadas, em vez de optimizar uma única tela.

---

# 7. Determinismo, replay, telemetria e PES 🟡 CONTÍNUO

O determinismo não é uma feature decorativa: é o mecanismo que permite reproduzir exactamente o que aconteceu.

## Objectivos

- replay determinístico;
- save states;
- reprodução de falhas;
- capturas determinísticas;
- comparação entre versões;
- telemetria de runs quando houver uma pergunta concreta a responder.

## Quando aprofundar

Depois da integração de campanha, porque então haverá cenários de jogo/editor concretos que podem ser medidos de forma útil.

## Próximo investimento

O **Movement Feel Benchmark** já existe como ferramenta e deve ser usado/expandido para cenários pequenos de movimento/câmara/VFX. Ele permanece distinto de benchmarks futuros de PCG e de geração condicionada ao jogador.

Não criar métricas apenas por acumulação: cada nova métrica deve responder a uma pergunta de engenharia ou design.

---

# 8. Qualidade de níveis e campanhas 🟡

A qualidade deve ser separada em níveis de evidência:

```text
sintaxe válida
→ geometria válida
→ comportamento físico válido
→ percurso/análise de execução
→ transição de campanha
→ experiência percebida
```

Ser válido não significa ser fácil. Ser difícil não significa ser defeituoso.

## Corpus real

`Game/Assets/Levels/NaoValidados/` pode legitimamente estar vazio. Não criar mapas artificiais só para produzir uma aparência de cobertura.

Quando houver conteúdo comunitário ou mais mapas oficiais, a validação deve ser aplicada sobre esse corpus real.

---

# 9. Dificuldade, análise e geração 🟡 POSTERIOR

A dificuldade deve ser tratada como propriedade distinta da validade:

```text
validade física
→ dificuldade de execução
→ desempenho observado
→ dificuldade percebida
```

## Porque é posterior

Antes de fazer inferências sobre dificuldade é necessário existir um pipeline estável de níveis, runs, replay e observação.

## Regras

Um analisador deve começar por ajudar o autor. Não deve alterar automaticamente geometria ou física authored.

Adaptive difficulty não é uma decisão de produto aprovada por defeito. Só deve entrar depois de existir evidência de que resolve um problema real e de que o efeito pode ser medido.

---

# 10. Comunidade e partilha 🟡 POSTERIOR

O objectivo futuro é permitir níveis e campanhas comunitárias, com validação local obrigatória antes de executar conteúdo externo.

A sequência deve ser:

```text
conteúdo criado
→ serialização
→ validação local
→ importação
→ organização em campanha
→ playtest
```

Partilha entre máquinas, distribuição online e funcionalidades multiplayer em tempo real não devem atrasar o núcleo local determinístico.

---

# 11. Distribuição final 🟡 ÚLTIMA FASE

O produto final pretendido é um **Windows x64 portable/standalone**.

## Requisitos

- sem terminal visível;
- sem instalador obrigatório;
- recursos de runtime junto do executável;
- funcionamento independente do ambiente de desenvolvimento;
- sem dependência operacional de Python, Make, Vulkan SDK ou ferramentas de build.

## Porque é último

A embalagem só deve ser congelada quando gameplay, editor, campanha, assets e validação estiverem suficientemente estáveis. Caso contrário, cada alteração estrutural gera trabalho de empacotamento repetido.

---

# 12. Ordem prática actual

```text
1. Manter main saudável e integrar cada tranche com evidência
   ↓
2. Integrar CampaignEditorDocument/CampaignEditorSnapshot na UI real
   ↓
3. Seleccionar e abrir o nível escolhido no Level Editor
   ↓
4. Fechar o fluxo editar → playtest → regressar → validar → guardar
   ↓
5. Implementar selecção explícita de campanha
   ↓
6. Implementar ecrã visual de Controlos
   ↓
7. Fazer revisão visual humana final do editor/menu
   ↓
8. Fechar Fase 9 apenas após evidência completa
   ↓
9. Consolidar sprites/assets/licenciamento
   ↓
10. Integrar background/parallax gameplay-first
   ↓
11. Expandir replay/telemetria/Movement Feel Benchmark por casos concretos
   ↓
12. Qualificar corpus real e transições de campanha
   ↓
13. Analisar dificuldade e futuras formas de geração
   ↓
14. Preparar partilha/importação comunitária
   ↓
15. Fechar release portable/standalone
```

A ordem pode mudar apenas perante uma falha, dependência ou decisão de produto que justifique objectivamente a alteração.

---

# 13. Estado actual

| Área | Estado | Porque está neste estado |
|---|---|---|
| Foundation | ✅ CLOSED | Contratos críticos e gates finais validados |
| 9.1 Controlos | ✅ núcleo | Semântica pronta; UI de configuração ainda falta |
| 9.2 Rato/viewport | ✅ | Boundary lógica/janela implementada |
| 9.3 Editor state | ✅ | Infraestrutura pronta; revisão visual humana pendente |
| 9.4 Entity authoring | ✅ | Plataformas, spawn, FLAG e cancelamento protegidos |
| 9.5 Save/validate | ✅ | Vertical slice persistente/assíncrona fechada |
| 9.6 Campaign | 🔄 | Modelo robusto; integração visual ainda falta |
| Playtest integrado | 🟡 | Falta fechar separação entre edição temporária e persistida no fluxo completo |
| Visual/assets | 🟡 | Direcção definida; produção visual estruturada ainda por consolidar |
| Replay/telemetria/PES | 🟡 | Infraestrutura existente; expansão orientada por casos de uso |
| Corpus/qualidade | 🟡 | Depende de conteúdo real e do ciclo de autoria completo |
| Dificuldade/adaptive | 🟡 POSTERIOR | Requer métricas e evidência |
| Partilha/comunidade | 🟡 POSTERIOR | Primeiro estabilizar conteúdo e validação local |
| Release portable | 🟡 ÚLTIMA | Consolidação final do produto |

---

# 14. Critério de sucesso do roadmap

O projecto deve aproximar-se de 1.0 quando um utilizador consegue, sem ferramentas externas de desenvolvimento:

```text
abrir o jogo
→ escolher campanha
→ jogar a progressão vertical
→ compreender a força dos saltos
→ abrir o editor
→ criar/alterar um nível
→ testar
→ regressar sem perder trabalho não guardado
→ validar
→ guardar
→ colocar na campanha
→ voltar a jogar
```

E a equipa consegue transformar uma falha importante em:

```text
falha reproduzível
→ evidência
→ isolamento
→ correcção
→ teste
→ validação CI
→ documentação activa
```

Esse é o critério que une gameplay, editor, campanha, determinismo, assets e distribuição.
