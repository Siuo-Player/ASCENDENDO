# ADR-0006 — Adoção do Guia Canónico de Identidade e Segurança do Estúdio (Zero Blast Radius & PPID)

## Contexto
O estúdio definiu o **Guia Canónico de Arquitetura de Segurança e Decisões de Identidade** como blueprint unificado para todos os projetos atuais e futuros.
O ASCENDENDO precisava de alinhar formalmente o seu subsistema de autenticação e proteção de dados com este padrão, garantindo que o jogador possa desfrutar da conveniência de identificação multi-plataforma (incluindo Google OIDC e Contas de Estúdio) sem qualquer risco de correlação cruzada entre jogos ou exposição de dados pessoais (PII).

## Decisão

Adotar integralmente as 5 diretrizes do Guia Canónico no ASCENDENDO:

1. **Raio de Explosão Zero (*Zero Blast Radius*) via PPID:**
   - Implementação de Pairwise Pseudonymous Subject Identifiers em conformidade com o **OpenID Connect Core 1.0 (Secção 8)** e **NIST SP 800-63B**.
   - Cada jogo deriva o seu identificador de forma determinística e unidirecional utilizando o seu `project_slug="ascendendo"` e `sector_salt`:
     $$\text{PlayerID}_{\text{Ascendendo}} = \text{as\_}\langle\text{HMAC-SHA256}(\text{SectorSalt} : \text{"ascendendo"}, \text{MasterSubjectID})[:16]\rangle$$
   - Isso garante que $\text{PlayerID}_{\text{Ascendendo}} \neq \text{PlayerID}_{\text{RedWar}} \neq \text{PlayerID}_{\text{Outro}}$, tornando matematicamente impossível cruzar perfis ou reutilizar credenciais roubadas.

2. **Modelo de Identidade em 3 Camadas:**
   - **Camada 1 (Convidado):** Sessão anónima imediata via UUIDv4 local (`localStorage` / `userdata`). Zero recolha de dados.
   - **Camada 2 (Google OIDC com Menor Privilégio):** Validação de token JWT assinado pela Google com escopos mínimos (`openid`, `profile`). Sem acesso a senhas, emails sensíveis ou permissões invasivas (Drive, Gmail). Herda 2FA/Passkeys nativos da Google.
   - **Camada 3 (Conta Dedicada de Jogo):** Cofre criptográfico com PBKDF2-HMAC-SHA256 e CSPRNG salt de 256 bits, isolado hermeticamente no sistema de ficheiros (`userdata/Accounts/<hash>/`).

3. **Defesas de Sistema e Canal Lateral:**
   - Verificação em tempo constante (`constantTimeEquals` em C++, `hmac.compare_digest` em Python) para proteção absoluta contra *timing attacks*.
   - Tokens com audiência estrita (`aud: "ascendendo-client"`) e expiração curta (1 hora).
   - Pacote `packages/studio_auth` construído com zero dependências externas no PyPI, blindado contra *supply-chain attacks*.

4. **Defesa de Rede e Payloads:**
   - Teto rígido de 64 KB para pacotes de dados.
   - Mascaramento defensivo de erros (sem stack traces ou caminhos do SO expostos ao cliente).

## Consequências
- Conformidade total e harmoniosa com o blueprint de segurança do estúdio.
- O ASCENDENDO passa a operar como um cidadão de primeira classe do ecossistema do estúdio sem comprometer a sua natureza offline-first e a soberania do jogador sobre os seus próprios dados.
