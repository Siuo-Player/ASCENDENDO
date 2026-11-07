# Sessão 2026-11-02 — Automação de Release, Cofre de Contas Criptográfico e Otimização do Verificador

## Contexto & Objetivos
- Responder integralmente às diretrizes do utilizador para a versão 1.0:
  1. Preparar o lançamento (release) 100% gratuito e automatizado do jogo via GitHub (Releases & GitHub Pages);
  2. Implementar e auditar o sistema de contas de jogador (`AccountManager`) com foco em privacidade máxima (zero telemetria, zero PII, cofre criptográfico HMAC-SHA256 e PBKDF2 offline);
  3. Esclarecer com total transparência a arquitetura do catálogo (10 campanhas com progressão física, 610 níveis) vs. a meta de 100+ campanhas;
  4. Otimizar analítica e computacionalmente os verificadores de dificuldade física para execução veloz;
  5. Validar todas as suites de testes nativos e de tooling.

## Implementações Realizadas

### 1. Sistema de Contas Criptográficas Locais (`Game/Core/AccountManager`)
- **Zero PII & 100% Offline:** Sem dependência de serviços remotos ou OAuth (Google), protegendo integralmente a identidade real do jogador.
- **Derivação Forte de Chave:** PBKDF2 com HMAC-SHA256 (10.000 iterações) e salt aleatório de 16 bytes por utilizador.
- **Isolamento de Diretórios:** Cada jogador possui a sua sandbox `userdata/Accounts/<hash>/` contendo o seu perfil assinado criptograficamente (`profile.secure`).
- **Verificação em Tempo Constante:** Prevenção ativa de timing attacks em comparações de hash.
- **Testes Unitários:** `Tests/Unit/test_account_manager.cpp` cobrindo registo, login, troca de credenciais, deleção e rejeição de adulterações no cofre.

### 2. Automação de Release Gratuita (GitHub Releases & Pages)
- **`.github/workflows/release.yml`:** Pipeline automatizado no GitHub Actions para Windows x64. Compila `game.exe` em C++20 com Vulkan SDK e Clang/MSVC, empacota shaders SPIR-V e assets, e publica automaticamente `ascendendo-windows-x64.zip` acompanhado do checksum SHA-256 no GitHub Releases.
- **`Development/Tools/package_release.py`:** Utilitário Python para empacotamento local rápido e determinístico.
- **Portal Web no GitHub Pages (`site/` e `.github/workflows/pages.yml`):**
  - Website estático moderno com tipografia Orbitron e Space Grotesk;
  - Simulador interativo em Canvas HTML5 replicando a física do Commitment Jump a 60°;
  - Exposição das 10 campanhas oficiais e manifesto de segurança/privacidade;
  - Download direto das releases oficiais sem anúncios ou intermediários.

### 3. Otimização do Verificador de Dificuldade (`Development/AI_Validation/validate_campaign.py`)
- **Poda Geométrica e Balística:** Plataformas com distância vertical superior a 145 px são podadas analiticamente de imediato.
- **Filtragem de Testes de Robustez:** Execução de ensaios de ruído estocástico apenas nos candidatos mais centralizados.
- **Multiprocessamento de Granularidade Fina:** Paralelização distribuída por nível (`_validate_level_job`) via `ProcessPoolExecutor`, balanceando os 610 níveis por todos os núcleos da CPU.

## Testes & Verificações
- `make tests-fast`: 406 casos de teste e 4684 asserções aprovadas com 0 falhas.
- `python -m pytest Tests/Python/test_campaign_validation.py`: 8 testes aprovados em 7.58s.
- `python Development/Tools/validate_repository_state.py`: Consistência comprovada nos 610 níveis canónicos.
- `python Development/Tools/check_source_sizes.py`: Políticas de tamanho de ficheiro respeitadas.
- `python Development/Tools/validate_platform_asset_registry.py`: Assets e proveniência validados.
