# ASCENDENDO

**2D vertical precision platformer com motor Vulkan próprio.**

ASCENDENDO é construído à volta do **Commitment Jump**: o jogador escolhe a força do salto, lança-se a um ângulo fixo de 60° e não corrige a trajectória no ar. A ideia central é transformar limitação em precisão: observar, medir, decidir e assumir o salto.

**Autor:** Rafael Gomes Bernardo  
**Assistência de desenvolvimento:** Claude (Anthropic) e Gemini (Google)

## Estado actual

O `main` contém um jogo executável com:

- simulação determinística a 60 Hz e Commitment Jump;
- progressão vertical por níveis, câmara e transições entre ecrãs;
- replay/save states e menus/pausa;
- controlos reconfiguráveis;
- editor de níveis integrado e editor de campanha;
- validação física automática e validação no editor;
- pipeline de apresentação de plataformas e captura determinística;
- campanha actualmente com **15 níveis** em `Game/Assets/Levels/campaign.txt`.

O principal trabalho em falta é **conteúdo jogável, qualidade da experiência e validação humana**. A infraestrutura existente só deve crescer quando um problema real do jogo o exigir.

## O jogo

A campanha sobe verticalmente. Os níveis usam plataformas estáticas, spawn e `FLAG`. A dificuldade deve nascer sobretudo da geometria, leitura espacial, posicionamento e compromisso do salto, não de controlos cada vez mais complexos.

O conteúdo visual segue uma direcção simples de pixel art, com módulos de plataforma de 16×16, foreground jogável e fundos em camadas com parallax. Assets externos só entram no jogo quando a origem, licença e ficheiro exacto estiverem verificados.

## Editor

O editor faz parte do produto. Permite criar e alterar níveis, navegar por níveis verticais, posicionar spawn/flag e plataformas, desfazer/refazer, guardar com validação e trabalhar sem depender de ferramentas externas. O editor de campanha organiza os níveis da campanha.

## Documentação

Há uma única fonte viva para o planeamento: [`docs/ROADMAP.md`](docs/ROADMAP.md).

Documentação técnica estável: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).  
Visão e regras de produto: [`docs/PRODUCT_VISION.md`](docs/PRODUCT_VISION.md).  
Formato dos níveis: [`docs/LEVEL_FORMAT.md`](docs/LEVEL_FORMAT.md).  
Testes: [`docs/TESTING.md`](docs/TESTING.md).

O Git preserva o histórico. Não são mantidos snapshots datados, estados correntes duplicados ou work packages concluídos apenas para registar o trabalho passado.

## Build e testes

Dependências e comandos completos estão no `Makefile` e nos workflows de CI. O projecto usa C++17+, Vulkan SDK, GLFW e Python 3 para tooling/validação.

A validação automática demonstra correção e reprodutibilidade; **não substitui playtesting humano**.

```text
código correcto → nível jogável → campanha interessante → playtest → release
```

## Licença

MIT. Dependências externas mantêm os termos das respectivas licenças/proveniências.
