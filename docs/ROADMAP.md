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

**Estimativa global actual: ~56%.**

| Dimensão | Peso conceptual | Estado estimado | Regra de avanço |
|---|---:|---:|---|
| Fundação | 10% | **100%** | contratos base verificados e sem dívida estrutural conhecida que bloqueie gameplay |
| Gameplay | 20% | **~85%** | run 1.0 completa, previsível e jogável |
| Conteúdo | 20% | **~35%** | conteúdo oficial suficiente, estruturado e validável |
| Validação | 15% | **~35%** | validator, dificuldade e replay passam de infraestrutura a evidência usada em conteúdo real |
| Produto | 25% | **~50%** | editor/campaign UX, fluxos principais e integração jogador↔editor completos |
| Release | 10% | **~10%** | build distribuível, QA final, import/export e presença pública |

As percentagens são baseline de acompanhamento e não critérios automáticos de fecho. Uma fase só fica `CLOSED` quando os seus critérios de saída têm implementação e evidência suficientes no `main`.

---

# 1. Fundação — ✅ FECHADA

### Meta verdadeira

A fundação deve determinar de forma clara o estado do jogo e a separação Core/Logic/Presentation, input semântico e bindings, determinismo e fixed timestep, lifecycle gráfico, validação fail-closed, contratos de save/load e fronteiras runtime/editor/tooling.

### Estado actual

**100%.** A revisão de 2026-09-10 concluiu que as lacunas do baseline histórico foram resolvidas ou absorvidas por extensões posteriores justificadas. A evidência detalhada está em `docs/FOUNDATION_GATE_STATUS_2026-09-10.md`.

### Evidência integrada

- PR #265 — fundação de níveis verticais, input semântico e editor keyboard-first — `1d2d1a511314c5c7a820c268c270b63be6f8e6b9`.
- PR #270 — Campaign Editor no fluxo real, com `campaign.txt` como autoridade — `4b1bab04da2613c65e26c8a23c253ac7c59499ca`.
- PR #271 — remoção do snap obrigatório de 4 px e preservação de coordenadas — `c60cb057c2b49b5f027108c7f51b6cf4ff04b246`.
- PR #280 — ordem operacional canónica dos seis gates — `82e08cf8ce0153c5ebcbd3c505dcdcb617657cd3`.

A matriz do último PR de implementação relevante passou Linux C++20, Linux ASan/UBSan, Windows e captura determinística.

### Regra após o fecho

Fundação fechada não significa arquitectura congelada. Alterações posteriores entram como extensões motivadas pelos gates seguintes, não como reabertura indiscriminada da base.

---

# 2. Gameplay — FECHAR DEPOIS DA FUNDAÇÃO

### Meta verdadeira

O jogador deve conseguir iniciar uma run, jogar com o modelo físico pretendido, progredir pelo level e atingir a `FLAG`, sem depender do editor, de ferramentas internas ou de comportamento experimental.

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

A arquitectura deve continuar capaz de receber perigos, checkpoints, moving platforms, triggers, colectáveis e outras entidades futuras, mas essas mecânicas não entram automaticamente em 1.0.

### Estado actual

**~85%.** O runtime e o movimento já são uma parte forte do projecto. O que falta é tornar o gameplay base consistente em níveis verticais reais, com transições, regras de run e conteúdo que cubra os casos relevantes.

### Critérios de fecho

- uma run completa é jogável sem depender do editor;
- colisão, movimento, câmara e progressão são determinísticos;
- streaming/transição vertical não altera o resultado;
- o contrato `spawn → inputs → FLAG` está estável;
- o comportamento definido de 1.0 é testado nos cenários críticos;
- não ficam mecânicas experimentais a fingir que são parte do 1.0.

---

# 3. Conteúdo — CONSTRUIR O JOGO QUE O JOGADOR VAI JOGAR

### Meta verdadeira

Transformar o motor em conteúdo oficial suficiente para que ASCENDENDO tenha uma experiência completa e progressiva.

O mínimo oficial previsto para 1.0 é **10 campanhas / 575 levels**:

- 1×10;
- 4×25;
- 3×50;
- 1×100;
- 1×250.

A campanha deve seguir aproximadamente fácil → difícil, sem assumir antecipadamente um número fixo de classes. Classes e limites devem resultar da fórmula e da validação final.

### Conteúdo visual

- indie simples;
- pixel art básica;
- assets-base em peças 16×16;
- colocação pixel-perfect sem impor grelha ao level design;
- foreground jogável;
- backgrounds em camadas;
- parallax;
- sprites curados;
- ficha formal de origem/licença para assets externos.

Áudio 1.0 inclui música retro livre e efeitos de jogo/editor.

### Critérios de fecho

- todas as campanhas oficiais existem nos formatos canónicos;
- levels oficiais são válidos no mesmo motor usado pela comunidade;
- o conteúdo representa progressão e não apenas quantidade;
- assets e licenças estão registados;
- não há dependência de fixtures temporárias para preencher o catálogo.

---

# 4. Validação — PROVAR QUE O CONTEÚDO FUNCIONA

### Meta verdadeira

Separar claramente:

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

Um level poder ser concluído não prova que é bom; um level difícil não prova que é interessante; um replay reproduzível não prova sozinho que a dificuldade foi calibrada.

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

Leaderboards usam tempo. Em 1.0 não se introduzem mortes nem uma pontuação baseada apenas em altura.

### Critérios de fecho

- validator detecta os defeitos de conteúdo previstos;
- Difficulty Tester usa uma definição estável e documentada;
- os resultados são reproduzíveis;
- replays demonstram reexecução válida;
- as campanhas oficiais passam pelo pipeline real de validação;
- os limites do que é inferido automaticamente e do que exige avaliação humana estão documentados.

---

# 5. Produto — FECHAR O FLUXO COMPLETO DO UTILIZADOR

Esta é a grande camada actualmente em construção. O editor é a ferramenta que transforma o sistema de levels num produto utilizável.

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
- mouse/drag como conveniência;
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

## 5.4 Playtest não persistente

O autor deve conseguir testar o level actual e voltar ao editor sem save automático escondido. O documento em memória deve permanecer recuperável até decisão explícita.

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

A integração funcional já foi incorporada em #270; o fecho de Produto exige tratar o fluxo completo como produto, não apenas a existência dos componentes.

Criar/remover entries directamente da UI permanece em investigação UX.

## 5.6 Selecção e controlo

- selecção explícita de campanha mesmo quando existe uma única campanha;
- nome + username + metadata + stable ID;
- thumbnails não são requisito 1.0;
- ecrã de bindings descobrível;
- remapeamento livre das acções suportadas;
- teclado como método completo de operação.

## 5.7 Revisão visual e UX

Depois do fluxo funcional consolidado:

- menu;
- editor;
- viewport;
- responsividade;
- feedback visual;
- acessibilidade e discoverability.

### Critério de fecho de Produto

Um utilizador novo deve conseguir:

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

Entregar uma versão que um utilizador externo consegue instalar, abrir, compreender, usar e manter num percurso suportado.

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

A versão web é desejável, mas não invalida automaticamente o 1.0 desktop.

### Critérios de fecho

- instalação limpa funciona;
- build de release é reproduzível;
- saves/imports oficiais são compatíveis com a versão suportada;
- campanhas publicadas passam pelo mesmo validator do jogo;
- regressões de runtime e editor estão cobertas por QA;
- licenciamento está auditado;
- documentação de utilização está coerente com o comportamento real.

**Só aqui ASCENDENDO é 1.0.**

---

# Ordem operacional definitiva

```text
1. Fundação       [100%] → FECHADA
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

Uma percentagem só sobe materialmente quando uma meta de produto ponderada muda de estado. PRs pequenos, refactors internos e documentação isolada podem melhorar qualidade sem justificar vários pontos percentuais globais.

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
