# Roadmap — ASCENDENDO

Este é o **único documento vivo de planeamento do produto**. O Git é o histórico; o código e os assets no `main` são a fonte da verdade do estado implementado.

## Estado actual

O jogo já tem uma base jogável e um catálogo oficial com **610 níveis** em 10 campanhas integrado no runtime. O runtime inclui física determinística a 60 Hz, Commitment Jump a 60°, câmara vertical contínua, carregamento e transições de níveis, replay determinístico verificado ponta a ponta até ao objetivo derivado, menus de navegação, controlos reconfiguráveis, alternância dinâmica de ecrã inteiro (F11), preservação do rácio de aspecto 16:9, editor de níveis e editor de campanha, pipeline de apresentação com fontes TrueType e sprites, e um subsistema seguro de integridade criptográfica com HMAC-SHA256 para persistência e perfis de utilizador.

A aceitação do produto para 1.0 entra agora na fase de validação humana, calibração de dificuldade e fecho da experiência de utilizador.

---

## Marcos até ao ASCENDENDO 1.0

### 1. Conteúdo e Catálogo Canónico 1.0
O catálogo oficial é composto por 10 campanhas com progressão vertical contínua:
`01-iniciacao` (10), `02-ritmo` (25), `03-deslocamento` (25), `04-alternancia` (25), `05-pressao` (25), `06-persistencia` (50), `07-margem` (50), `08-precisao` (50), `09-resistencia` (100) e `10-cume` (250) — totalizando **610 níveis**.

Critérios de conclusão de cada nível no catálogo:
1. Formato oficial canónico `.lvl` sem `SPAWN` nem `FLAG` authored;
2. Validação automática de reachability física e ausência de bloqueios (`validate_campaign.py`);
3. Execução e streaming contínuos sem quedas de framerate no runtime Vulkan;
4. Playtesting manual com verificação de leitura espacial;
5. Registo e afinação de plataformas enganadoras ou de ruído visual.

### 2. Validação, Replays e Determinismo Físico
- Suite de evidência determinística (`test_replay_run_evidence.cpp`): gravação de sequência de inputs e playback determinístico até à colisão com a flag derivada.
- Manter o contrato de simulação fixa a 60 Hz invariante entre plataformas (Windows x64 e Linux x64).
- Difficulty Tester: validação automática como cálculo de limites físicos, mantendo estrita separação entre viabilidade geométrica e dificuldade humana perceptível.

### 3. Apresentação Visual e Janela Responsiva
- Área lógica canónica: `640 × 360` com escala adaptativa com letterboxing/pillarboxing.
- Alternância instantânea de fullscreen via `F11` e detecção de modos do monitor primário via GLFW.
- Apresentação em pixel-art com módulos 16×16, distinguibilidade clara entre solo/plataformas jogáveis e elementos decorativos de fundo.
- Assets integrados respeitando o registo de proveniência e licenciamento (`Game/Assets/Sprites/PLATFORM_ASSET_REGISTRY.md` e `docs/THIRD_PARTY_NOTICES.md`).

### 4. Mecanismo Seguro de Informação e Dados (Criptografia e Resiliência)
- Subsistema nativo NIST FIPS 180-4 SHA-256 e RFC 2104 HMAC-SHA256 para proteção e autenticação de saves e perfis (`profile.secure`).
- Verificação de autenticidade em tempo constante (`constantTimeEquals`) imune a timing attacks.
- Contenção rigorosa de diretórios (`isPathContained`) prevenindo directory climbing (`..`) e manipulações de symlink.
- Sanitização de nomes de ficheiro contra injeções de caminhos e nomes reservados de sistema no Windows (`CON`, `PRN`, `AUX`, `NUL`, etc.).
- Proteção ativa contra injeção de fórmulas CSV (`=`, `+`, `-`, `@`) no histórico analítico de runs (`runs.csv`).
- Transações atómicas de ficheiros com escrita temporária e substituição atómica resiliente a crashes e falhas de energia.

### 5. Editor Integrado como Ferramenta de Produção
- Operações de histórico atómicas: um gesto inteiro de drag & drop corresponde a uma única operação de Undo/Redo.
- Diagnóstico visual de reachability em tempo real através de `EditorRenderSnapshot` enriquecido sem recalcular física na thread de apresentação.
- Manipulação direta fluida: selecção, redimensionamento de plataformas e navegação vertical intuitiva.
- Editor de campanha para reordenação, inserção e validação de playlists `.txt`.

### 6. Calibração Humana e Qualidade de Experiência
- Sessões de playtest externo com jogadores humanos sem conhecimento prévio do motor.
- Avaliação da curva de frustração vs. aprendizagem: o Commitment Jump deve recompensar paciência e leitura.
- Ergonomia dos controlos: clareza na barra de força e resposta tátil imediata ao soltar a tecla de salto.
- Menus responsivos, ecrã de créditos acessível e opções claras de remapeamento de teclas.

### 7. Release 1.0 e Distribuição
Só declarar **ASCENDENDO 1.0** quando o fluxo completo de utilização externa for comprovado:
```text
instalar → abrir → escolher campanha → jogar e ascender → criar/editar níveis → validar → guardar com segurança → voltar a abrir
```
- Pacote zip autónomo e portátil para Windows x64 sem necessidade de dependências instaladas externamente.
- CI verde em todas as suites: C++ unit/integration/acceptance tests, Python validation, Vulkan headless e verificação estrita do repositório.

---

## O que não fazer

- Não criar novos snapshots datados `STATUS_*`, `CURRENT_STATUS_*` ou `ROADMAP_*`.
- Não transformar cada bug ou execução de CI num documento permanente ou post-mortem duplicado.
- Não usar linhas de código ou contagem de commits como métrica de conclusão do jogo.
- Não desvirtuar o foco: o ASCENDENDO é um precision platformer vertical de autor, não uma engine genérica.
- Nunca reintroduzir `SPAWN` ou `FLAG` como entidades authored nos ficheiros de nível.

## Critério de progresso

O progresso real mede-se por:
**código seguro e determinístico → conteúdo jogável no catálogo → minutos de jogo comprovados → validação com jogadores humanos → experiência 1.0 pronta para lançamento.**
