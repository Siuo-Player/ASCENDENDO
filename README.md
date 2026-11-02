# ASCENDENDO

**2D vertical precision platformer com motor Vulkan próprio.**

ASCENDENDO é construído à volta do **Commitment Jump**: o jogador escolhe a força do salto, lança-se a um ângulo fixo de 60° e não corrige a trajectória no ar. A ideia central é transformar limitação em precisão: observar, medir, decidir e assumir o salto.

**Autor:** Rafael Gomes Bernardo  
**Assistência de desenvolvimento:** Claude (Anthropic), Gemini (Google) e ChatGPT — GPT-5.6 Luna (OpenAI)

## Estado actual

O `main` contém um jogo executável com:

- simulação determinística a 60 Hz e Commitment Jump;
- progressão vertical contínua, câmara e transições suaves entre ecrãs;
- catálogo oficial com **610 níveis** em 10 campanhas (`Game/Assets/Campaigns/catalogue.txt`); `Game/Assets/Levels/campaign.txt` permanece como formato histórico/fallback com 25 níveis;
- alternância de ecrã inteiro dinâmico via tecla `F11` e janela redimensionável com preservação de aspect ratio (letterboxing/pillarboxing a 640×360);
- subsistema de segurança criptográfica com autenticação HMAC-SHA256, proteção contra injeções CSV/traversal e persistência atómica de perfil (`docs/SECURITY.md`);
- replay determinístico verificado ponta a ponta e save states;
- controlos reconfiguráveis em ficheiro seguro de utilizador;
- editor de níveis integrado e editor de campanha com validação física;
- pipeline de apresentação Vulkan com fontes TrueType e sprites em pixel-art.

O foco para o lançamento **Ascendendo 1.0** é **validação humana, calibração da curva de dificuldade e experiência de utilizador**.

## O jogo

A campanha sobe verticalmente. Os níveis usam plataformas estáticas e estado de jogo derivado para spawn e objetivo final. A dificuldade nasce da geometria, leitura espacial, posicionamento e compromisso do salto, não de controlos arbitrários ou hazards artificiais.

O conteúdo visual segue uma direcção de pixel art, com módulos de plataforma de 16×16, foreground jogável de alta leitura e fundos em camadas com parallax. Assets externos só entram no jogo quando a origem, licença e ficheiro exacto estiverem verificados.

## Editor

O editor faz parte do produto. Permite criar e alterar plataformas, navegar por níveis verticais, seleccionar e editar geometria, desfazer/refazer, guardar com validação física de reachability e organizar campanhas completas sem depender de ferramentas externas. Spawn e objetivo final são estado derivado e nunca são authored nos ficheiros `.lvl`.

## Controlos

| Tecla / Ação | Função |
|---|---|
| `A` / `←` | Mover para a esquerda |
| `D` / `→` | Mover para a direita |
| `Espaço` (premir/carregar) | Carregar Commitment Jump (ângulo fixo a 60°) |
| `Espaço` (soltar) | Lançar salto com a força acumulada |
| `F11` | Alternar ecrã inteiro (Fullscreen) |
| `Esc` / `P` | Pausa / Menu |
| `E` | Alternar para o Editor de Níveis integrado |

## Documentação

Há uma única fonte viva para o planeamento: [`docs/ROADMAP.md`](docs/ROADMAP.md).

- Especificação de Segurança e Integridade: [`docs/SECURITY.md`](docs/SECURITY.md).
- Arquitetura técnica estável: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).
- Visão e regras do produto: [`docs/PRODUCT_VISION.md`](docs/PRODUCT_VISION.md).
- Formato e regras dos níveis: [`docs/LEVEL_FORMAT.md`](docs/LEVEL_FORMAT.md).
- Estratégia de testes e validação: [`docs/TESTING.md`](docs/TESTING.md).
- Notas de licenças de terceiros: [`docs/THIRD_PARTY_NOTICES.md`](docs/THIRD_PARTY_NOTICES.md).
- Créditos de assistência por IA: [`docs/AI_CREDITS.md`](docs/AI_CREDITS.md).

## Build e testes

O projecto requer **C++20** (Clang/LLVM), **Vulkan SDK** (1.3+), **GLFW 3.4** e **Python 3.10+**.

```bash
# Compilar e executar testes rápidos TDD
make tests-fast

# Compilar e executar a suite de testes completa (430+ casos de teste)
make tests

# Compilar o executável de release do jogo
make game

# Executar suite de validação em Python (ferramentas e física)
python -m pytest Tests/

# Validar consistência de estado do repositório
python Development/Tools/validate_repository_state.py

# Validar reachability mecânica de todo o catálogo oficial (610 níveis)
python Development/AI_Validation/validate_campaign.py --catalogue
```

A validação automática demonstra correção física e reprodutibilidade; **não substitui o playtesting humano**.

```text
código seguro → nível jogável → campanha validada → playtest humano → release 1.0
```

## Licença

MIT License ([`LICENSE`](LICENSE)). As dependências e ativos externos mantêm as suas respetivas licenças e proveniências descritas em [`docs/THIRD_PARTY_NOTICES.md`](docs/THIRD_PARTY_NOTICES.md).
