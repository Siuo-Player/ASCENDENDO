# ASCENDENDO — Visão do produto

> Documento canónico de visão: define o jogo, a experiência e as propriedades deliberadas do produto. A ordem de execução fica em `docs/ROADMAP.md`. Decisões históricas ficam em `docs/DECISIONS/`.

## 1. O que é

ASCENDENDO é um **precision / masochist vertical platformer** construído sobre um motor 2D próprio.

A experiência deve ser simples de compreender e difícil de dominar: ler o espaço, escolher onde aterrar, carregar o salto, libertá-lo e comprometer-se com a trajectória. A profundidade nasce da precisão, desenho de plataformas, memória muscular, repetição e aprendizagem.

O produto é simultaneamente:

```text
JOGO + AUTORIA + PLAYTEST + VALIDAÇÃO + DETERMINISMO/REPLAY
+ CAMPANHAS + COMUNIDADE + ASSETS/LICENCIAMENTO + DISTRIBUIÇÃO
```

## 2. Regras de jogo 1.0

### Commitment Jump

- salto parabólico;
- ângulo base `60°`;
- sem controlo aéreo;
- força dependente do tempo de carga;
- carga contínua enquanto Jump está premido;
- sem reset/ciclo durante a mesma carga;
- crescimento linear em 1.0;
- função parametrizável para experiências futuras.

A força apresentada ao jogador representa directamente o estado real da simulação; não existe uma escala artificial `0–255`.

### Movimento e controlos

Defaults: `A`/`←` esquerda, `D`/`→` direita, `Space` Jump. O jogador pode remapear livremente as acções disponíveis.

## 3. Estrutura espacial

```text
SCREEN = 640 × 360
LEVEL  = 640 × (N × 360)
```

Um `.lvl` representa um level vertical composto por `N` screens de `640×360`. `N` é variável. Não há largura configurável.

A progressão é exclusivamente ascendente. O jogador pode cair para screens inferiores e regressar fisicamente, mas nunca progride deliberadamente para baixo.

## 4. Conteúdo 1.0

Só existem três entidades de gameplay:

- plataformas estáticas;
- jogador/spawn;
- FLAG.

A FLAG fica no fim do level, na última screen. Não há morte em 1.0 nem estado de failure/death.

A arquitectura continua aberta a perigos, moving platforms, triggers, colectáveis, checkpoints e outros objectivos futuros, mas essas mecânicas ficam fora de 1.0.

## 5. Transição e streaming

A passagem entre screens é contínua. O jogo mantém as zonas necessárias para transições suaves e não introduz pausas visíveis nem altera a simulação determinística.

## 6. Level Editor

O Level Editor edita o level completo e preserva `640×360` como unidade de leitura.

```text
teclado apenas
teclado + rato
```

As operações essenciais continuam disponíveis por bindings semânticos. Rato/drag é acelerador, não requisito.

O playtest não grava automaticamente alterações.

## 7. Estado de edição

O documento em memória é separado do estado persistido. `Esc` pode tratar alterações pendentes como carrinho: guardar, descartar ou rever. Undo/redo continua a ser o histórico base.

## 8. Campaign Editor

O Campaign Editor gere a sequência de levels. O fluxo 1.0 começa por seleccionar, reordenar, abrir, voltar, validar e guardar `campaign.txt`.

## 9. Campanhas 1.0

Mínimo oficial: **10 campanhas / 575 levels**.

Cada campanha progride aproximadamente do fácil para o difícil/extremo. A escala nominal de dificuldade deriva do modelo e da validação reais, não de um número arbitrário congelado.

## 10. Validação, dificuldade e runs

Conteúdo publicável passa pelo validador e pelo difficulty tester.

```text
spawn → inputs/movimentos gravados → FLAG
```

O replay é reexecutado pelo motor e o tempo dessa execução válida é a medida autoritativa.

## 11. Visual

Direcção 1.0: **indie, simples, pixel-art básica**, com legibilidade do gameplay acima da decoração.

Foreground contém jogador e plataformas; background cria profundidade; parallax usa velocidades relativas diferentes.

### Plataforma e auto-tiling

O autor trabalha com **uma plataforma contínua de um material**, não com pintura manual de tiles.

- a posição no mundo é pixel-perfect e não está bloqueada a uma grelha global de `16×16`;
- largura **e altura** da plataforma são múltiplas positivas de `16 px`;
- a composição visual usa uma malha `16×16` local à região;
- o renderer escolhe automaticamente sprites curados segundo adjacência, topologia, material, superfícies expostas, interiores, bordas, cantos e joins;
- plataformas/regiões de materiais diferentes podem compor-se visualmente quando existe contacto geométrico real;
- relva e outros tratamentos de borda só aparecem onde o contexto visual os torna coerentes;
- variantes podem ser escolhidas com aleatoriedade controlada e determinística para reduzir repetição;
- uma variante ausente ou inelegível produz fallback determinístico;
- auto-tiling é **presentation-only**: nunca altera posição, dimensão, colisão ou outra geometria de gameplay.

`16×16` é unidade de composição visual, não unidade obrigatória de autoria do mundo.

## 12. Sprites e licenciamento

Sprites são opções curadas. Todo asset externo tem ficha formal de origem/licença no repositório, inclusive CC0.

## 13. Áudio

1.0 inclui música retro simples livre para redistribuição e efeitos de jogo/editor. Efeitos não-musicais podem ter takes múltiplos com selecção controlada; a música permanece estável.

## 14. Comunidade e publicação

A unidade principal de publicação é a campanha. `.lvl` continua a ser unidade técnica de armazenamento/import/export do level.

1.0 requer import/export de campanhas e um site público para partilha e rankings.

## 15. Distribuição e horizonte

Obrigatório para 1.0:

- Windows x64 standalone;
- catálogo validado;
- difficulty tester;
- site público de partilha/ranking.

A versão web é desejável, não obrigatória.

A arquitectura permanece aberta a mods, expansões e jogos derivados sem introduzir em 1.0 mecânicas sem razão de design.
