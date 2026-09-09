# Roadmap de desenvolvimento — ASCENDENDO

> Documento operacional canónico do produto. A ordem abaixo define a sequência principal até ao 1.0. O progresso é ponderado por importância de produto; não representa ficheiros, linhas de código ou número de issues fechadas.

## Modelo de progresso até 1.0

O produto é acompanhado por seis dimensões, nesta ordem:

1. **Fundação** — contratos técnicos que tornam o resto do jogo previsível e seguro.
2. **Gameplay** — experiência base de jogar uma run completa.
3. **Conteúdo** — campanhas, levels e assets suficientes para o jogo ter conteúdo real e não apenas fixtures.
4. **Validação** — garantir que levels, runs, dificuldade e replay são verificáveis e reproduzíveis.
5. **Produto** — editor e fluxos de utilização completos, UX, partilha e integração das peças numa experiência coerente.
6. **Release** — empacotamento, QA final, distribuição e operação pública.

Esta ordem não proíbe trabalho paralelo. Significa que um bloco posterior não deve ser usado para esconder um blocker anterior. Trabalho paralelo é aceitável quando não cria uma dependência incorrecta nem transforma protótipos em critérios de 1.0.

### Estado de referência

**Estimativa global actual: ~55%.**

| Dimensão | Peso conceptual | Estado estimado | Regra de avanço |
|---|---:|---:|---|
| Fundação | 10% | **~95%** | fechar todos os contratos base e deixar apenas extensões justificadas |
| Gameplay | 20% | **~85%** | run 1.0 completa, previsível e jogável |
| Conteúdo | 20% | **~35%** | conteúdo oficial suficiente, estruturado e validável |
| Validação | 15% | **~35%** | validator, dificuldade e replay passam de infraestrutura a evidência usada em conteúdo real |
| Produto | 25% | **~50%** | editor/campaign UX, fluxos principais e integração jogador↔editor completos |
| Release | 10% | **~10%** | build distribuível, QA final, import/export e presença pública |

Estas percentagens são **baseline de acompanhamento**, não critérios automáticos de fecho. Uma fase só fica `CLOSED` quando os seus critérios de saída têm implementação e evidência suficientes no `main`.

---

# 1. Fundação — FECHAR A 100%

### Meta verdadeira

A fundação deve deixar de ser um conjunto de componentes que “funcionam” e tornar-se uma base estável sobre a qual gameplay, editor, conteúdo e ferramentas possam evoluir sem reabrir constantemente contratos básicos.

O objectivo é que o núcleo determine de forma clara:

- estado do jogo e separação Core/Logic/Presentation;
- input semântico e bindings;
- determinismo;
- fixed timestep;
- lifecycle gráfico;
- validação fail-closed;
- contratos de save/load e estados relevantes;
- fronteiras entre runtime, editor e tooling.

### Estado actual

**~95%.** A fundação foi declarada fechada nas fases anteriores e já suporta o desenvolvimento activo. O trabalho restante nesta dimensão deve limitar-se a lacunas concretas encontradas por features reais, não a reabrir a arquitectura por preferência estética.

### Para atingir 100%

A fundação fica efectivamente fechada quando:

1. cada contrato base necessário ao Gameplay 1.0 está documentado;
2. os fluxos críticos têm testes executáveis;
3. alterações de editor, streaming ou replay não dependem de estados implícitos não documentados;
4. nenhuma feature posterior precisa de duplicar a autoridade do motor;
5. eventuais extensões da fundação aparecem como decisões motivadas, com contrato próprio.

**Não significa:** que nunca mais haverá alterações arquitecturais. Significa que novas alterações são extensões justificadas, não dívida estrutural conhecida.

---

# 2. Gameplay — FECHAR DEPOIS DA FUNDAÇÃO

### Meta verdadeira

O jogador deve conseguir iniciar uma run, jogar com o modelo físico pretendido, progredir pelo level e atingir a `FLAG`, sem que a experiência dependa do editor, de ferramentas internas ou de comportamento experimental.

A especificação 1.0 do gameplay é:

- salto parabólico a 60°;
- sem air control;
- carga contínua;
- carga linear em 1.0;
- força apresentada com precisão real, sem compressão artificial `0–255`;
- plataformas estáticas;
- spawn e `FLAG` como entidades de 1.0;
- `FLAG` localizada na última screen;
- sem sistema de morte/failure em 1.0;
- saída voluntária para o menu.

A arquitectura deve continuar capaz de receber perigos, checkpoints, moving platforms, triggers, colectáveis e outras entidades futuras, mas essas mecânicas **não entram automaticamente em 1.0**.

### Estado actual

**~85%.** O runtime e o movimento já são uma parte forte do projecto. O que falta não é “inventar um platformer”; é tornar o gameplay base consistente em níveis verticais reais, com transições, regras de run e conteúdo que cubra os casos relevantes.

### Critérios de fecho

- uma run completa é jogável sem depender do editor;
- colisão, movimento, câmara e progressão são determinísticos;
- streaming/transição vertical não altera o resultado;
- o contrato `spawn → inputs → FLAG` está estável;
- o comportamento definido de 1.0 é testado nos cenários críticos;
- não ficam mecânicas experimentais a fingir que são parte do 1.0.

**Depois desta fase, o jogo base deve ser tratado como produto jogável estável, não como protótipo técnico.**

---

# 3. Conteúdo — CONSTRUIR O JOGO QUE O JOGADOR VAI JOGAR

### Meta verdadeira

Transformar o motor em conteúdo oficial suficiente para que ASCENDENDO tenha uma experiência completa e progressiva.

O mínimo oficial previsto para 1.0 é:

- **10 campanhas / 575 levels**;
- 1×10;
- 4×25;
- 3×50;
- 1×100;
- 1×250.

A campanha deve seguir aproximadamente fácil → difícil, sem assumir antecipadamente que um número fixo de classes de dificuldade é correcto. O número e os limites das classes devem resultar da fórmula e da validação final.

### Conteúdo visual

A direcção mantém-se:

- indie simples;
- pixel art básica;
- assets-base em peças 16×16;
- colocação pixel-perfect sem impor grelha ao level design;
- foreground jogável;
- backgrounds em camadas;
- parallax;
- sprites curados;
- ficha formal de origem/licença para assets externos.

Áudio 1.0 inclui música retro livre e efeitos de jogo/editor. A pesquisa musical aprofundada aproxima-se do fecho de 1.0.

### Critérios de fecho

- todas as campanhas oficiais existem nos formatos canónicos;
- levels oficiais são válidos no mesmo motor usado pela comunidade;
- o conteúdo representa realmente progressão e não apenas quantidade;
- assets e licenças estão registados;
- não há dependência de fixtures temporárias para preencher o catálogo.

**Não conta como conteúdo final:** levels automáticos gerados só para testar o motor, exemplos de desenvolvimento ou placeholders sem revisão.

---

# 4. Validação — PROVAR QUE O CONTEÚDO FUNCIONA

### Meta verdadeira

Separar claramente quatro perguntas:

```text
validade
    ↓
progressão
    ↓
percurso
    ↓
dificuldade
    ↓
experiência humana
```

Um level poder ser concluído não prova que é bom. Um level difícil não prova que é interessante. Um replay reproduzível não prova sozinho que a dificuldade foi calibrada.

### Componentes

- validator de campanhas e levels;
- Difficulty Tester;
- runs determinísticas;
- replay por reexecução do motor;
- métricas de tempo;
- análise de rotas rápidas/difíceis e fáceis/demoradas;
- validação humana onde o critério é experiência e não apenas correção mecânica.

### Regras de replay e runs

A unidade de run é:

```text
spawn → inputs → FLAG
```

O replay é uma reexecução pelo motor. O tempo válido é o tempo autoritativo produzido pela execução válida.

Leaderboards usam **tempo**. Em 1.0 não se introduzem mortes nem uma pontuação baseada apenas em altura.

### Critérios de fecho

- validator detecta os defeitos de conteúdo previstos;
- Difficulty Tester usa uma definição estável e documentada;
- os resultados são reproduzíveis;
- replays não são apenas armazenamento de inputs: demonstram reexecução válida;
- as campanhas oficiais passam pelo pipeline real de validação;
- os limites do que é inferido automaticamente e do que exige avaliação humana estão documentados.

---

# 5. Produto — FECHAR O FLUXO COMPLETO DO UTILIZADOR

Esta é a grande camada actualmente em construção. O editor não é um fim em si mesmo: é a ferramenta que transforma o sistema de levels num produto utilizável.

## 5.1 Sistema de autoria vertical

Modelo canónico:

```text
screen = 640×360
level  = N screens verticais, largura sempre 640
campaign = sequência de levels
```

Requisitos:

- N variável;
- mapeamento screen ↔ coordenada mundial determinístico;
- `.lvl` antigo continua a significar N=1;
- metadata `SCREENS N` persistida no `.lvl`.

## 5.2 Editor vertical

- viewport capaz de navegar pelo level completo;
- câmara vertical previsível;
- navegação por teclado;
- mouse/drag como conveniência, não como requisito;
- operações essenciais acessíveis sem mouse.

## 5.3 Undo/redo e carrinho

Consolidar um histórico geral das alterações.

`Esc` deve permitir tratar mudanças pendentes como carrinho:

```text
guardar tudo
ou
fechar/descartar
ou
rever e desfazer selectivamente
```

O objectivo é evitar perda acidental de trabalho e tornar autoria iterativa segura.

## 5.4 Playtest não persistente

O autor deve conseguir testar o level actual e voltar ao editor sem save automático escondido.

O documento em memória deve permanecer recuperável até a decisão explícita do autor.

## 5.5 Campaign Editor

Fluxo alvo:

```text
seleccionar campanha
→ seleccionar level
→ reordenar
→ abrir level
→ voltar
→ validar
→ guardar campaign.txt
→ reabrir
```

A integração do Campaign Editor no fluxo real já começou a ser incorporada; o fecho exige tratar o fluxo completo como produto, não apenas a existência de `CampaignEditorDocument` e widgets isolados.

## 5.6 Selecção e controlo

- selecção explícita de campanha mesmo quando existe uma única campanha;
- nome + username + metadata + stable ID;
- thumbnails não são requisito 1.0;
- ecrã de bindings descobrível;
- remapeamento livre das acções suportadas;
- teclado como método completo de operação.

## 5.7 Revisão visual e UX

Só depois do fluxo funcional estar consolidado:

- menu;
- editor;
- viewport;
- responsividade;
- feedback visual;
- acessibilidade e discoverability.

### Critério de fecho de Produto

Um utilizador novo deve conseguir, sem conhecer a implementação:

```text
abrir
→ escolher campanha
→ jogar
→ editar/criar conteúdo
→ testar
→ validar
→ guardar/exportar
→ voltar a abrir
```

sem depender de scripts de desenvolvimento ou conhecimento interno do projecto.

---

# 6. Release — TRANSFORMAR O PROJECTO EM 1.0 DISTRIBUÍVEL

### Meta verdadeira

Não é apenas “compilar”. É entregar uma versão que um utilizador externo consegue instalar, abrir, compreender, usar e manter num percurso suportado.

Obrigatório para 1.0:

- Windows x64 standalone;
- import/export de campanhas;
- validação de campanhas;
- Difficulty Tester;
- catálogo oficial completo;
- site público de partilha/rankings;
- processo reprodutível de build;
- QA de release;
- revisão de assets/licenças;
- documentação mínima para utilizadores e criadores.

A versão web é desejável, mas não deve invalidar o 1.0 desktop se não existir uma razão de produto que a torne obrigatória.

### Critérios de fecho

- instalação limpa funciona;
- build de release é reproduzível;
- saves/imports oficiais são compatíveis com a versão suportada;
- campanhas publicadas passam pelo mesmo validator do jogo;
- regressões de runtime e editor estão cobertas por QA;
- licenciamento dos conteúdos está auditado;
- documentação de utilização está coerente com o comportamento real.

**Só aqui ASCENDENDO é 1.0.**

---

# Ordem operacional definitiva

```text
1. Fundação       [~95%] → FECHAR 100%
        ↓
2. Gameplay       [~85%] → FECHAR
        ↓
3. Conteúdo       [~35%] → CONSTRUIR E VALIDAR
        ↓
4. Validação      [~35%] → TORNAR EVIDÊNCIA OPERACIONAL
        ↓
5. Produto        [~50%] → FECHAR FLUXOS COMPLETOS
        ↓
6. Release        [~10%] → EMPACOTAR, TESTAR, PUBLICAR
```

### Paralelismo permitido

O editor e ferramentas de autoria podem continuar enquanto Gameplay fecha, desde que não introduzam regras paralelas ao motor. Validação pode preparar tooling antes de todo o conteúdo existir, mas a aceitação final depende de dados reais. Arte/áudio podem avançar incrementalmente, mas só contam como conteúdo final quando integram o catálogo oficial e passam pelas regras de licença.

### Regra de percentagem

Uma percentagem só sobe materialmente quando uma **meta de produto ponderada** muda de estado. PRs pequenos, refactors internos e documentação isolada podem melhorar qualidade sem justificar vários pontos percentuais globais.

### Critério absoluto de 1.0

```text
foundation stable
→ gameplay 1.0 stable
→ official content complete
→ validation demonstrably used
→ product/editor complete
→ release QA + distribution
→ public 1.0
```

Até esse percurso ser efectivamente exercitável, ASCENDENDO continua em desenvolvimento.
