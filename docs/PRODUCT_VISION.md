# ASCENDENDO — Visão do produto

## 1. Propósito

ASCENDENDO é um **Vertical Precision Platformer** construído sobre um motor 2D próprio. O objectivo não é apenas entregar um jogo com alguns níveis: é criar uma experiência de plataforma vertical precisa e um sistema de autoria suficientemente fiável para que novos níveis possam continuar a ser criados, testados, validados, jogados e partilhados durante muitos anos.

O projecto tem dois lados inseparáveis:

- **jogo:** salto de precisão, leitura vertical, risco, aprendizagem e domínio da execução;
- **ecossistema de autoria:** criação, playtest, validação, diagnóstico e organização de níveis/campanhas.

O editor é infraestrutura do produto. A intenção é que a comunidade não dependa da equipa original para validar manualmente cada mapa ou para produzir todo o conteúdo futuro.

## 2. Fantasia do jogador

A experiência deve fazer cada salto parecer uma decisão deliberada. A progressão é vertical e exige leitura do espaço, compromisso com a trajectória e execução consistente.

A mecânica central actualmente definida é o **Commitment Jump**:

- salto parabólico;
- ângulo fixo de 60°;
- sem controlo aéreo;
- carga de força antes da libertação;
- feedback visual explícito sobre a força carregada.

O jogador não deve ter de inferir a carga apenas pelo tempo de pressão de uma tecla. A barra/indicador de força deve comunicar claramente o estado actual e os limites mínimo/máximo.

O jogo deve privilegiar domínio, não volume de mecânicas. Novos sistemas só devem entrar quando reforçarem o objectivo principal ou uma necessidade comprovada do produto.

## 3. Unidade de jogo: o nível

Cada `.lvl` corresponde a **uma tela lógica 640×360**. O nível é uma unidade pequena e controlável de autoria, validação e composição.

A campanha é uma sequência ordenada dessas telas. `campaign.txt` é a autoridade sobre a ordem da campanha.

O design deve privilegiar:

- leitura vertical clara;
- plataformas e saltos intencionais;
- continuidade entre telas;
- dificuldade analisável e reproduzível;
- possibilidade de testar um nível isoladamente antes de o integrar na campanha.

A localização física de um ficheiro não define se pertence à campanha. `reorganize.py` continua a tratar do routing entre `Levels/`, `Unused/` e `NaoValidados/`.

## 4. Level Editor

O Level Editor vive dentro do próprio jogo e edita apenas uma tela 640×360. O canvas completo deve permanecer perceptível; não é uma ferramenta de desenho livre nem um editor com pan ilimitado.

O fluxo pretendido inclui:

```text
editar → testar → observar → voltar sem perder a edição → corrigir → validar → guardar
```

O editor deve suportar, dentro do escopo actual:

- colocar, mover e remover plataformas;
- snap previsível;
- editar o spawn com limites de segurança derivados da plataforma inicial;
- colocar/remover a FLAG apenas quando o nível ocupar a posição final da campanha;
- guardar no formato `.lvl` existente;
- validação antes da persistência final;
- playtest não persistente.

A arquitectura deve continuar preparada para acrescentar conteúdo no futuro sem quebrar níveis antigos.

## 5. Campaign Editor

O Campaign Editor organiza os níveis como uma **playlist/timeline vertical**. Cada nível aparece como um bloco compacto com proporção 16:9 e a ordem vertical representa a ordem da campanha.

O modelo de campanha deve permitir:

- carregar a playlist canónica;
- seleccionar explicitamente um nível;
- reordenar níveis;
- preservar a identidade da selecção durante reordenações;
- validar referências;
- rejeitar duplicados, ficheiros inexistentes e path escape;
- guardar apenas `campaign.txt`;
- deixar o routing físico para `reorganize.py`.

A 9.6 deve terminar com este modelo consumido pelo fluxo visual real do editor, e não como um componente lógico isolado.

## 6. Playtest, validação e diagnóstico

Há duas camadas complementares:

1. **feedback rápido em memória**, para o autor trabalhar sem esperar por um processo externo a cada alteração;
2. **validação final do executável**, como autoridade para conteúdo que vai ser jogado/importado.

Um mapa importado ou partilhado deve ser validado novamente pelo EXE. A origem externa nunca é a autoridade final de validade.

Quando uma tentativa de percurso falhar, o diagnóstico deve ser útil para o autor, mostrando quando possível:

- trajectória tentada;
- ponto aproximado de falha;
- primeira causa útil conhecida, por exemplo alvo demasiado longe, ângulo impossível, plataforma fora da janela alcançável, colisão lateral ou sequência seguinte inacessível.

No Campaign Editor, o diagnóstico deve distinguir um problema local de uma quebra na transição entre níveis.

## 7. Determinismo e reprodutibilidade

O motor usa fixed timestep e o projecto pretende que input, simulação, replay e estados importantes sejam reprodutíveis.

Isto permite:

- reproduzir falhas;
- comparar versões;
- testar níveis de forma consistente;
- suportar save states e replay;
- produzir evidência de engenharia em vez de depender de observações ocasionais.

A validação deve continuar a combinar testes Linux, sanitizers, Windows, campanha activa e capturas determinísticas quando aplicável.

## 8. Direcção visual

A regra principal é **gameplay-first readability**: personagem, plataformas, perigos e objectivos devem ser distinguíveis imediatamente.

A composição desejada separa:

- **foreground:** personagem e plataformas jogáveis, com maior contraste e clareza;
- **background:** atmosfera e profundidade;
- **parallax:** camadas de fundo com movimento relativo para reforçar a sensação de subida sem competir com a jogabilidade.

O visual deve apoiar a leitura dos saltos e não transformar cada tela numa imagem saturada de informação.

## 9. Arte, sprites e licenciamento

A autoria visual deve usar assets controlados e reutilizáveis, não um editor de desenho livre.

Preferências actuais:

- assets próprios sempre que possível;
- recursos CC0 ou com licença claramente conhecida e compatível quando forem externos;
- registo da origem/licença mesmo quando um recurso é CC0;
- evitar assets pagos, de origem desconhecida ou com termos incompatíveis com distribuição futura.

Existe uma direcção de pequena escala, incluindo **16×16** para assets base quando apropriado, mas a colocação dos elementos no nível não deve ficar presa a uma grelha rígida apenas por causa disso.

A arquitectura suporta sprites e o plano futuro inclui escolhas controladas de sprites para personagem, chão/plataformas e decoração. Alterar um sprite deve ser uma decisão visual e não alterar silenciosamente a física ou a colisão.

Criação/edição livre de sprites dentro do jogo não é prioridade enquanto a base de assets e direitos não estiver consolidada.

## 10. Campanhas e comunidade

A campanha oficial é apenas o primeiro corpus. O produto deve poder crescer para:

- múltiplas campanhas;
- campanhas longas;
- campanhas especiais e desafios extremos;
- níveis comunitários;
- conteúdo importado/partilhado validado localmente.

A selecção de campanha deve existir como conceito mesmo quando há apenas uma. Deve permitir uma escolha consciente e comunicar, quando disponível:

- nome;
- número de níveis;
- estado de validade;
- preview/miniatura;
- iniciar;
- voltar.

Partilha entre máquinas e funcionalidades online são posteriores ao núcleo local e determinístico.

## 11. Dificuldade e análise

O projecto deve separar explicitamente:

```text
validade física
    ↓
dificuldade de execução/conteúdo
    ↓
desempenho observado
    ↓
dificuldade percebida/experiência
```

Um nível válido não é necessariamente fácil; um nível difícil não é necessariamente defeituoso.

Um futuro analisador de dificuldade deve primeiro ajudar o autor. Não deve alterar silenciosamente a geometria, física ou conteúdo authored.

Adaptive difficulty não é uma regra de produto já aprovada. Antes de ser considerada, o projecto precisa de métricas reproduzíveis e evidência de playtesting.

## 12. Portabilidade e distribuição

O produto final pretendido é um **Windows x64 portable/standalone**.

O jogador deve poder abrir a distribuição directamente, sem instalar o ambiente de desenvolvimento, sem terminal visível e sem depender de ferramentas como Python, Make, Vulkan SDK ou bibliotecas de desenvolvimento externas.

Os recursos de runtime devem acompanhar a distribuição final na própria pasta do produto.

## 13. Princípios de engenharia que protegem o produto

O código deve evoluir segundo:

```text
investigar
→ documentar a decisão
→ implementar a menor alteração suficiente
→ testar
→ validar em CI
→ actualizar o estado canónico
```

A história detalhada pertence ao Git e às notas apropriadas; documentação activa deve descrever decisões e estado actuais.

Não criar abstrações ou mecânicas apenas para “parecer mais completo”. Toda a alteração deve justificar-se por uma necessidade do produto, uma falha demonstrável ou uma melhoria de verificabilidade.

## 14. Definição de sucesso

A visão é concretizada quando um utilizador consegue fazer, no próprio jogo:

```text
abrir
→ escolher uma campanha
→ jogar uma sequência vertical exigente e legível
→ compreender a força dos saltos
→ dominar a execução
→ abrir o editor
→ construir/ajustar um nível
→ testar sem persistir automaticamente
→ validar
→ guardar
→ integrar numa campanha
→ voltar a jogar
```

E quando a equipa consegue:

```text
falha reproduzível
→ isolar
→ diagnosticar
→ corrigir
→ proteger com teste/evidência
→ continuar a evoluir sem quebrar o conteúdo existente
```

Este ciclo é o motivo pelo qual gameplay, editor, determinismo, validação, arte e distribuição pertencem ao mesmo projecto.
