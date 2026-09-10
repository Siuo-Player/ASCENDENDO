# Foundation Gate — estado verificado — 2026-09-10

## Objetivo

Registar a verificação atual do Gate #272 e substituir o baseline histórico de ~95% por evidência concreta do estado integrado em `main`.

## Resultado

A fundação requerida para o Gameplay 1.0 está suficientemente consolidada para o fecho do gate. As lacunas descritas na auditoria histórica de 2026-08-26 foram revistas contra a arquitetura atual e não constituem blockers ativos.

## Evidência por contrato

### Core / Logic / Presentation

O runtime mantém responsabilidades separadas: `GameSession` concentra política de estado/simulação; `GraphicsRuntime` e `PresentationRuntime` concentram lifecycle e apresentação; `main.cpp` atua como composição/orquestração. O gameplay não passa diretamente o modelo de domínio ao renderer: o frame de gameplay usa `buildRenderSnapshot(...)` antes da apresentação.

### Input semântico

`GameAction`/`KeyBindings` são a autoridade de input configurável. Os fluxos do editor e do Campaign Editor usam ações semânticas próprias, com bindings explícitos.

### Determinismo / fixed timestep

O núcleo já possui timestep defensivo e a infraestrutura de testes/captura determinística é executada nos gates CI. A evidência final mais recente inclui captura determinística verde nos PRs de fundação/editor.

### Lifecycle gráfico

O lifecycle gráfico está encapsulado no runtime dedicado e os fluxos Vulkan críticos foram previamente validados em CI headless. A matriz corrente executa Linux/headless, sanitizers, Windows e captura determinística.

### Validação fail-closed

Parsing e validação rejeitam dados inválidos em vez de os promover silenciosamente. A alteração de #269, por exemplo, adicionou rejeição explícita de coordenadas não finitas sem reintroduzir quantização.

### Save/load e modelo de level

O modelo canónico `screen = 640×360`, `level = N screens verticais`, largura fixa e compatibilidade de `.lvl` legado foram integrados em #265. `SCREENS N` é persistido e ficheiros antigos continuam a significar uma screen.

### Runtime / editor / tooling

#270 integrou o Campaign Editor no fluxo real sem criar uma segunda autoridade de regras: `campaign.txt` continua a autoridade da ordenação e `LevelDataIO` permanece a fronteira de carregamento do level. O Level Editor trabalha com o mesmo modelo de level vertical.

### Authoring / coordinates

#271, governado por #269, removeu o snap obrigatório de 4 px. Coordenadas fracionárias são preservadas no authoring, movimento, spawn, FLAG e restore; mínimos geométricos continuam validados separadamente.

## Reavaliação das fragilidades históricas

| Fragilidade histórica | Estado atual |
|---|---|
| `main.cpp` excessivamente responsável por subsistemas | mitigado: runtime/session/bootstrap/presentation estão extraídos; `main.cpp` fica como composição |
| gameplay diretamente acoplado ao domínio na apresentação | mitigado: `RenderSnapshotBuilder` produz dados de apresentação |
| paths de dados presos à source tree | mitigado: `RuntimeBootstrap` + `RuntimePaths` preparam user-data paths separados |
| matriz CI limitada | mitigado para os contratos atuais: Linux normal + ASan/UBSan + Windows + deterministic capture |
| editor baseado numa única screen | resolvido pela base N-screen vertical de #265 e integração de #270 |
| snap obrigatório contradizia pixel-perfect | resolvido por #269/#271 |

As fragilidades de engenharia que continuam possíveis no futuro são extensões ou trabalho de produto mensurável; não há, nesta revisão, uma segunda autoridade de regras ou um blocker estrutural conhecido que impeça o avanço para Gameplay.

## Evidência integrada

- PR #265 → `1d2d1a511314c5c7a820c268c270b63be6f8e6b9`
- PR #270 → `4b1bab04da2613c65e26c8a23c253ac7c59499ca`
- PR #271 → `c60cb057c2b49b5f027108c7f51b6cf4ff04b246`
- PR #280 → `82e08cf8ce0153c5ebcbd3c505dcdcb617657cd3`

O último PR de implementação (#271) passou Linux C++20, Linux ASan/UBSan, Windows e captura determinística no head validado antes do merge.

## Decisão

O Gate #272 pode ser fechado depois de esta auditoria e a atualização do `docs/ROADMAP.md` serem integradas em `main`. O encerramento não implica congelamento da arquitetura: mudanças futuras devem continuar a ser extensões motivadas pelos próximos gates.
