# ASCENDENDO

**2D vertical precision platformer com motor Vulkan próprio.**

ASCENDENDO é um platformer de progressão vertical baseado no **Commitment Jump**: o jogador escolhe a força do salto, lança-se a um ângulo fixo e não corrige a trajectória no ar. A mecânica transforma limitação em precisão: observar, medir, decidir e assumir o salto.

**Autor:** Rafael Gomes Bernardo  
**Assistência de desenvolvimento:** Claude (Anthropic) e Gemini (Google)  
**Repositório:** https://github.com/Siuo-Player/ASCENDENDO

## Estado atual

O `main` já ultrapassou a antiga descrição de “Fase 9.3”. O código actual inclui motor Vulkan/GLFW, física determinística a 60 Hz, Commitment Jump a 60°, câmara vertical, replay/save states, menu/pausa, controlos reconfiguráveis, **editor de níveis integrado**, **editor de campanha**, validação física automática, validação assíncrona no editor, compositor 8-neighbour com adjacência cross-region e swapchain fail-closed.

A referência operacional viva é [`docs/00-meta/ROADMAP.md`](docs/00-meta/ROADMAP.md), e o estado actual está em [`docs/00-meta/CURRENT_STATUS.md`](docs/00-meta/CURRENT_STATUS.md). Snapshots datados são histórico, não a fonte de verdade.

## Estrutura principal

```text
ASCENDENDO/
├── Game/
│   ├── Core/              # estado, acções, bindings, viewport, bootstrap
│   ├── Graphics/          # Vulkan, renderer, camera, apresentação
│   ├── Logic/             # gameplay, física, campanha, editor, replay
│   └── Assets/Levels/     # campanha e níveis
├── Development/
│   ├── AI_Validation/     # simulador, solver e level generator/validator
│   ├── LevelEditor/       # validação standalone
│   └── Runs/              # dados gerados de execução local
├── Tests/                 # unit + integration
├── docs/00-meta/          # roadmap/estado operacionais
├── main.cpp
└── Makefile
```

## Jogabilidade

A simulação usa fixed timestep de `1/60 s` e gravidade de `-980 px/s²`. A força do salto é carregada durante um período limitado e convertida num lançamento a 60°.

O jogador não recebe controlo aéreo. A decisão acontece antes do salto; a execução acontece durante o voo. A dificuldade deve vir de geometria, posicionamento, leitura visual e compromisso, não de controlos arbitrariamente complexos.

## Editor integrado

O editor é parte do jogo, não uma ferramenta externa. O `GameState::EDITOR` suporta edição de plataformas, spawn e flag, selecção por teclado/rato, cursor de autoria, navegação vertical em níveis maiores que um ecrã, tamanhos predefinidos, undo/redo, gravação com validação e validação assíncrona com controlo de `generation`. O `GameState::CAMPAIGN_EDITOR` ordena e abre os níveis da campanha.

## Níveis e campanha

`Game/Assets/Levels/campaign.txt` define a ordem da campanha. O foco actual deixou de ser expandir infraestrutura de apresentação sem conteúdo: o próximo progresso material é **mais minutos de jogo**.

Formato:

```text
NAME Nome
PLATFORM x y largura altura
FLAG x y largura altura
```

O `Development/AI_Validation/ai_validator.py` executa uma verificação determinística de alcançabilidade baseada na física do jogo.

## Build

É necessário C++17+, CMake, Vulkan SDK com `glslc`, GLFW 3.4 e Python 3. Dependências header-only usadas pelo projecto estão em `external/`.

No Windows, `external/glfw/lib-vc2022/` não deve ser presumido como uma biblioteca versionada no Git. O caminho reprodutível é obter o commit fixado do GLFW e construí-lo localmente com CMake/MSVC, como faz o workflow Windows.

## Testes e validação

Testes unitários, integração e validação de campanha protegem o comportamento. Captura determinística verifica múltiplos aspect ratios e níveis, mas **não substitui playtesting humano**.

```text
código correto → nível jogável → campanha interessante → validação humana → release
```

Não confundimos `CI ≠ diversão`, `captura ≠ playtest`, `metadata ≠ ficheiro binário` e `validação semântica ≠ validação física completa`.

## Desenvolvimento orientado ao produto

O Git é o histórico. `ROADMAP.md` e `CURRENT_STATUS.md` são documentos vivos. Não são criados snapshots datados por rotina quando já existe um documento canónico. Um incidente só recebe post-mortem formal quando houve impacto relevante, perda de dados/credenciais, bloqueio real ou uma mudança de política que justifique registo.

A métrica operacional passa a ser: **níveis jogáveis e validados, minutos de gameplay únicos e jogadores externos que conseguem completar a campanha**.

## Licença

MIT. Dependências externas mantêm os termos das respectivas licenças/proveniências.