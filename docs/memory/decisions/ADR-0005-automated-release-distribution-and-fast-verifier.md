# ADR-0005 — Distribuição Automatizada Gratuita (GitHub Releases & Pages) e Otimização do Verificador

## Contexto
O ASCENDENDO 1.0 requer um modelo de distribuição 100% gratuito, sem infraestrutura paga de nuvem, mantendo as garantias de privacidade (zero telemetria, offline-first) e transparência open source.
Adicionalmente, com 610 níveis canónicos distribuídos em 10 campanhas com progressão física, o tempo de validação física exaustiva (`validate_campaign.py`) tornava os ciclos de CI e pré-lançamento lentos quando executado sequencialmente.

## Decisões

1. **Distribuição Automatizada 100% Gratuita via GitHub:**
   - **GitHub Releases (`.github/workflows/release.yml`):** Compilação nativa em C++20 com Clang/MSVC e Vulkan SDK no runner Windows x64, empacotando os binários (`game.exe`), shaders compilados SPIR-V (`Game/Shaders/`), assets (`Game/Assets/`), licenças e manual rápido em ficheiros portáteis ZIP (`ascendendo-windows-x64.zip`) com verificação criptográfica SHA-256 (`.sha256`).
   - **Empacotador Local (`Development/Tools/package_release.py`):** Permite a qualquer desenvolvedor ou utilizador gerar o mesmo pacote e checksum localmente com um único comando.
   - **Portal Web no GitHub Pages (`site/` e `.github/workflows/pages.yml`):** Um site moderno, estático e responsivo (HTML5/CSS3/JS) com tema dark neon/cyberpunk, paleta Orbitron/Space Grotesk, demonstrador interativo com Canvas HTML5 que simula em tempo real o Commitment Jump a 60°, detalhamento dos 610 níveis em 10 campanhas, manifesto de privacidade e botões de download direto para as releases oficiais.

2. **Otimização Analítico-Espacial e Paralelização do Verificador de Dificuldade:**
   - **Poda Analítica de Altura Máxima (`MAX_JUMP_DY = 145.0 px`):** Com base na fórmula balística do motor (\(V_{y0} = 600 \times \sin(60^\circ) \approx 519.6\text{ px/s}\)), o alcance vertical máximo absoluto é de 137.75 px. Qualquer plataforma alvo com `target.top > source.top + 145.0` é descartada imediatamente sem simular milhares de trajetórias impossíveis.
   - **Seleção Gulosa de Centramento para Testes de Robustez:** Apenas as melhores aterragens mais centralizadas na plataforma passam pelos testes de ruído estocástico (80 ensaios de perturbação), reduzindo simulações repetitivas com o mesmo resultado.
   - **Paralelização de Granularidade Fina:** Distribuição dos 610 níveis individuais por `ProcessPoolExecutor` em vez de campanhas inteiras, eliminando o afunilamento em campanhas grandes (como o Cume de 250 níveis) e garantindo 100% de ocupação em todos os núcleos da CPU.

## Consequências
- Tempo de validação física da totalidade do catálogo reduzido drasticamente com fidelidade matemática e determinismo estritos.
- Pipeline de publicação totalmente sem custos operacionais, auditável e imune a dependências comerciais ou proprietárias.
