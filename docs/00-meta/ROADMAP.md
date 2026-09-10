# ASCENDENDO — Roadmap operacional vivo

**Fonte canónica:** este ficheiro. Snapshots `ROADMAP_YYYY-MM-DD.md` e `CURRENT_STATUS_YYYY-MM-DD.md` são apenas histórico.

## Regra de prioridade

```text
correcção que bloqueia o jogador
→ conteúdo jogável
→ validação física
→ playtest humano
→ apresentação/arte
→ performance
→ release
```

Não adicionar nova infraestrutura quando uma melhoria equivalente pode ser feita usando o que já existe. O objectivo de cada tranche é aumentar a qualidade ou a quantidade de jogo jogável.

## 1. Produto jogável — AGORA

### 1.1 Campanha

Aumentar a campanha progressivamente usando o formato `.lvl` existente e o editor já integrado. Cada nível novo deve ser carregável pelo runtime e aprovado pelo validador físico.

**Definition of done:** o nível está em `campaign.txt`, passa `ai_validator.py --campaign`, inicia no runtime e foi verificado manualmente.

### 1.2 Level design

Criar sequências com dificuldade crescente, evitando apenas repetir o mesmo salto. Variar distância horizontal, largura das plataformas, ritmo e informação visual. O Commitment Jump continua a ser a regra central.

### 1.3 Playtesting

Depois de uma tranche de níveis, obter runs humanas externas. Registar apenas evidência útil: onde o jogador falha, o que não compreende, duração, frustração e se a mecânica incentiva nova tentativa.

## 2. Editor — CONSOLIDAR

O editor `EDITOR` e o `CAMPAIGN_EDITOR` já existem em `main`. Não reabrir uma fase conceptual de editor. Corrigir apenas lacunas observadas durante autoria real: ergonomia, preview, undo/redo, save e validação.

## 3. Apresentação — USAR, NÃO EXPANDIR

O compositor de plataformas e a cadeia `RenderSnapshot` já suportam apresentação semântica e adjacência cross-region. Congelar novas extensões até haver conteúdo suficiente para revelar uma necessidade concreta.

Assets externos seguem a regra mínima:

```text
ficheiro exacto → licença/proveniência → dimensões → hash → teste visual → runtime
```

Um candidato sem binário concreto não é tratado como asset integrado.

## 4. Engenharia — CORRIGIR SÓ COM EVIDÊNCIA

A fundação Vulkan, física, replay e CI já é suficientemente madura para suportar desenvolvimento de conteúdo.

Prioridades técnicas reais:

- camera bounds/subpixel quando existir uma falha reproduzível;
- sanitizer coverage no caminho de integração de `main`;
- profiling antes de qualquer cache/batching/solver pruning;
- seams de fault injection Vulkan apenas se trouxerem valor de teste mensurável.

## 5. Release

Só depois de existir uma pequena campanha coerente e jogada por pessoas externas:

```text
build reproduzível
→ assets concretos
→ instalação simples
→ execução limpa
→ release Windows
→ decidir posteriormente distribuição Linux/macOS
```

## Métricas

Os números que importam agora são:

- níveis activos jogáveis;
- minutos de gameplay únicos;
- percentagem da campanha completável;
- jogadores externos que completam a campanha;
- bugs de gameplay reproduzíveis.

Commits, número de auditorias e número de documentos não são métricas de produto.

## Anti-fragmentação

Não criar novos roadmaps, status snapshots ou post-mortems por rotina. Alterar este ficheiro quando a prioridade muda. O `git log` preserva a evolução histórica.
