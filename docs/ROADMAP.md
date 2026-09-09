# Roadmap de desenvolvimento — ASCENDENDO

> Documento operacional canónico. O estado só avança quando existe evidência de implementação e testes adequados.

## 1. Ordem de trabalho

```text
investigar
→ documentar
→ implementar
→ testar
→ validar em CI
→ actualizar docs
→ avançar
```

Não trabalhamos directamente em `main`.

# 2. Fundação ✅ FECHADA

A fundação actual cobre separação Core/Logic/Presentation, contratos de input semântico, determinismo/fixed timestep, validação fail-closed e lifecycle gráfico.

A fundação fechada não significa que features futuras não possam exigir novas abstrações; significa que os contratos existentes têm evidência suficiente para servir de base.

# 3. Fase 9 — Sistema de autoria 🔄 ACTIVA

A Fase 9 transforma o editor num produto utilizável. A nova unidade é:

```text
screen = 640×360
level  = N screens verticais, largura sempre 640
campaign = sequência de levels
```

## 9.1 Input semântico e teclado-only ✅ núcleo

Já existe `GameAction`/`KeyBindings` e bindings para acções do editor.

### Requisito final

Toda operação essencial do editor deve funcionar com **teclado apenas**. Mouse/drag é atalho de conveniência.

## 9.2 Modelo espacial vertical 🔄 EM IMPLEMENTAÇÃO

Objetivos:

- largura exactamente 640;
- altura `N × 360`;
- N variável;
- mapeamento screen ↔ coordenada mundial determinístico;
- `.lvl` antigo continua a significar N=1;
- nova metadata `SCREENS N` persistida no `.lvl`.

### Porque agora

O modelo anterior de um level = uma única tela limita a variedade do conteúdo. A screen mantém a legibilidade original; o level ganha profundidade vertical sem ampliar a largura.

## 9.3 Editor vertical 🟡 PRÓXIMO

Adaptar viewport, câmara e ferramentas do Level Editor para navegar verticalmente pelo level completo.

A navegação deve funcionar só com teclado e com teclado+rato.

## 9.4 Undo/redo e carrinho 🟡

Consolidar um histórico geral de alterações. `Esc` apresenta mudanças pendentes como carrinho e permite guardar tudo, descartar tudo ou rever/desfazer selectivamente.

## 9.5 Playtest não persistente 🟡

Executar o level actualmente editado, regressar ao editor e preservar o documento em memória sem save automático.

## 9.6 Campaign Editor 🔄

Integrar `CampaignEditorDocument` + snapshot + UI:

```text
seleccionar
→ reordenar
→ abrir level
→ voltar
→ validar
→ guardar campaign.txt
→ reabrir
```

Criar/remover entries directamente da UI permanece em investigação UX.

## 9.7 Selecção de campanha 🟡

Seleccionar explicitamente campanha mesmo com uma única campanha. A UI usa nome + username + metadata + stable ID; thumbnails não são requisito 1.0.

## 9.8 Controlos/rebind 🟡

Finalizar e tornar descobrível o ecrã de bindings, permitindo remapeamento livre das acções suportadas.

## 9.9 Revisão visual 🟡

Rever visualmente menu, editor, viewport e responsividade apenas depois de o fluxo funcional estar consolidado.

# 4. Gameplay 1.0 🟡

Base:

- salto parabólico a 60°;
- sem air control;
- carga contínua;
- carga linear em 1.0;
- força visível com precisão real, sem `0–255` artificial;
- plataformas estáticas + spawn + FLAG;
- FLAG na última screen;
- sem morte/failure em 1.0;
- saída voluntária para menu.

A arquitectura deve aceitar futuras entidades letais, colectáveis obrigatórios, checkpoints, moving platforms e triggers sem as activar em 1.0.

# 5. Transição e streaming 🟡

Implementar transição vertical contínua entre screens/levels, mantendo a zona actual e a necessária para a próxima transição, pré-carregando a próxima e mantendo zonas anteriores recuperáveis durante quedas.

O streaming não pode alterar o resultado determinístico.

# 6. Determinismo, replay e runs 🟡

A run é `spawn → inputs → FLAG`.

O replay é reexecutado pelo motor. A execução válida determina o tempo autoritativo.

Leaderboards usam tempo; não há mortes nem métrica de altura em 1.0.

Agregados planeados: melhor tempo global, média, melhor jogador e estimativas de rotas rápidas/difíceis e fáceis/demoradas.

# 7. Difficulty Tester e catálogo 🟡

Separar:

```text
validade → progressão → percurso → dificuldade → experiência humana
```

Derivar o número de classes de dificuldade da fórmula final, em vez de escolher previamente um número arbitrário.

Cada campanha deve ser aproximadamente fácil → difícil. O catálogo global deve ser centrado no normal.

Mínimo 1.0: **10 campanhas / 575 levels oficiais**:

- 1×10;
- 4×25;
- 3×50;
- 1×100;
- 1×250.

# 8. Visual e áudio 🟡

Direcção: indie, simples, pixel art básica.

Assets base em peças 16×16; colocação pixel-perfect, sem grelha obrigatória.

Foreground jogável + backgrounds em camadas + parallax.

Sprites curados. Todos os assets externos têm ficha formal de origem/licença.

Áudio 1.0: música retro livre e efeitos de jogo/editor; efeitos não-musicais podem usar várias variações com randomização controlada.

Pesquisa musical aprofundada fica próxima do fecho de 1.0.

# 9. Comunidade e distribuição 🟡

Unidade principal de publicação: **campanha**.

`.lvl` continua unidade técnica de storage/import/export.

Obrigatório 1.0:

- importar/exportar campanhas;
- Windows x64 standalone;
- site público de partilha/rankings;
- validação de campanhas;
- difficulty tester.

Versão web é desejável.

# 10. Qualidade de conteúdo 🟡

A campanha oficial e a campanha comunitária devem ser validáveis pelo mesmo motor. A publicação não deve depender de validação manual da equipa.

O routing físico dos ficheiros continua a cargo de `reorganize.py`; `campaign.txt` é autoridade da ordem.

# 11. 1.1+ e derivados 🔭

Depois de 1.0 fechado, a arquitectura pode receber novas direcções: perigos, moving platforms, triggers, colectáveis/checkpoints, novos objectivos, e variantes horizontais em jogos derivados.

O princípio é generalizar interfaces e formatos sem antecipar mecânicas de produto antes de existir razão de design.

# 12. Critério de 1.0

```text
jogo base estável
→ editor keyboard-first
→ levels verticais N×360 @ 640 px
→ playtest não persistente
→ undo/redo + carrinho
→ campaign editor
→ campanhas oficiais completas
→ validator + difficulty tester
→ replay/runs determinísticos
→ import/export
→ Windows x64
→ site público
```

Só declarar 1.0 concluído quando este percurso for efectivamente exercitável e validado.
