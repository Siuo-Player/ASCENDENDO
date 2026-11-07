# ADR-0011: Zero Blast-Radius Accounts & GitHub Release Distribution

## Contexto
O utilizador solicitou o fecho de release do ASCENDENDO 1.0 (gratuito e open-source via GitHub Releases e GitHub Pages), a integração e confirmação do sistema de contas de jogador com proteção de privacidade máxima (Zero Blast Radius / PPID de acordo com NIST SP 800-63B e OpenID Connect Section 8), a verificação de segurança, a aferição do número de campanhas/níveis (610 níveis distribuídos com dificuldade progressiva) e a otimização dos verificadores de reachability.

## Decisão
1. **Isolamento de Contas e Perfis no Runtime Nativo (`AccountManager`)**:
   - Cada conta possui uma diretoria e cofre estanque em `Accounts/<accountId>/` (onde `accountId = SHA-256(username)`).
   - O cofre `accounts.secure` utiliza PBKDF2 com multi-round HMAC-SHA256 (10.000 iterações), salteamento criptográfico CSPRNG e comparações em tempo constante (`constantTimeEquals`).
   - `GameSession` e `main.cpp` integram `AccountManager` para alternar o `profile.secure` ativo por utilizador e refletir o utilizador no `MenuRenderSnapshot`.
   - Na apresentação, `UiRenderer` exibe `PERFIL: <NOME> (COFRE LOCAL SEGURO)` ou `CONVIDADO`.
   - 100% offline-first, zero dependências externas de rede, sem risco de fuga de PII.

2. **Web Companion & Simulador OIDC PPID (`site/`)**:
   - SDK `studio_auth_client.js` no portal web gera IDs estanques (`as_<hash>`) com isolamento por projeto, sem guardar credenciais externas nem partilhar tokens mestres.

3. **Distribuição e Release Gratuita**:
   - Correção dos endpoints canónicos de `https://github.com/hdsf7/Ascendendo` para `https://github.com/Siuo-Player/ASCENDENDO`.
   - Workflow de release (`.github/workflows/release.yml`) configurado para gerar arquivo ZIP portátil do Windows com hash SHA-256.
   - Workflow de GitHub Pages (`.github/workflows/pages.yml`) para hospedar a página promocional e simulador web.

## Consequências
- Os dados do jogador ficam completamente protegidos mesmo em máquinas partilhadas.
- O jogo permanece determinístico e sem dependência de servidores em nuvem.
- Releases automatizadas e auditáveis via GitHub Actions.
