# ASCENDENDO — Arquitectura de Segurança e Proteção de Dados

## 1. Visão Geral e Modelo de Ameaças

O ASCENDENDO integra um subsistema de segurança e integridade criptográfica com zero dependências externas (`Game/Core/Security.h`, `Game/Core/Security.cpp`, `Game/Core/Environment.h`, `Game/Core/Environment.cpp`).

### Superfícies de Risco e Mitigações

| Vetor de Risco | Risco Potencial | Mecanismo de Defesa do ASCENDENDO |
|---|---|---|
| **Adulteração de Save/Progresso** | Modificação arbitrária de recordes, tempos ou níveis completados | Envelope criptográfico autenticado com HMAC-SHA256 (`ASCENDENDO_SECURE_V1`) e verificação em tempo constante |
| **Directory Traversal / Path Escape** | Níveis ou playlists com `../` ou symlinks a aceder a ficheiros arbitrários do sistema | Verificação estrita de contenção de caminho (`core::Security::isPathContained`) |
| **Injeção de Nomes de Ficheiros** | Criação de ficheiros com caracteres ilegais ou nomes reservados do Windows (`CON`, `PRN`, `AUX`, `NUL`, `COM1-9`, `LPT1-9`) | Sanitização exaustiva de nomes de ficheiro (`core::Security::sanitizeFilename`) |
| **Injeção em CSV / Folhas de Cálculo** | Execução de fórmulas arbitrárias (`=`, `+`, `-`, `@`) ao abrir `runs.csv` no Excel/LibreOffice | Neutralização sistemática de prefixos de fórmula e escape RFC 4180 (`core::Security::sanitizeCsvField`) |
| **Ataques de Temporização (Timing Attacks)** | Dedução da assinatura criptográfica através do tempo de resposta da comparação de strings | Comparação em tempo constante com acumulador volátil (`core::Security::constantTimeEquals`) |
| **Corrupção por Crash / Queda de Energia** | Ficheiro de save parcialmente gravado e inutilizado após encerramento abrupto | Transações atómicas com gravação em ficheiro temporário e substituição atómica (`std::filesystem::rename`) |
| **Acesso a Variáveis de Ambiente Inseguro** | Vulnerabilidades e avisos de compilação associados a `std::getenv` multithreaded | Invólucro seguro multiplataforma (`_dupenv_s` em Windows, consulta segura em POSIX) |

---

## 2. Especificação Criptográfica

### 2.1 SHA-256 (NIST FIPS 180-4)
O motor implementa o padrão NIST FIPS 180-4 SHA-256 de forma nativa e autónoma. Não recorre a OpenSSL ou bibliotecas dinâmicas externas, garantindo determinismo idêntico entre arquiteturas Windows x64 e Linux x64.
- Testado contra os vectores oficiais de teste NIST (strings vazias, mensagens pequenas e mensagens multi-bloco de 512 bits).

### 2.2 HMAC-SHA256 (RFC 2104)
A autenticidade dos dados persistidos é assegurada via HMAC-SHA256. Uma chave de autenticação interna da aplicação deriva os blocos interno e externo (`ipad = 0x36`, `opad = 0x5c`), gerando uma assinatura criptográfica de 32 bytes (256 bits).
- Qualquer mutação de 1 bit nos dados ou na assinatura resulta na rejeição imediata da leitura.

### 2.3 Envelope Seguro (`ASCENDENDO_SECURE_V1`)
Os ficheiros protegidos (como o perfil do jogador `profile.secure`) utilizam o seguinte layout binário:
```text
+------------------------------------+-----------------------+--------------------+
| Magic Header (16 bytes)            | HMAC-SHA256 (32 bytes)| Carga Útil         |
| "ASCENDENDO_SECURE_V1\0\0\0\0"     | Assinatura dos dados  | Conteúdo em texto  |
+------------------------------------+-----------------------+--------------------+
```

### 2.4 Verificação em Tempo Constante
A função `core::Security::constantTimeEquals` processa todos os bytes da string independentemente de discrepâncias prévias, com recurso a um acumulador com qualificador `volatile`, prevenindo ataques de canal lateral (timing attacks).

---

## 3. Contenção de Caminhos e Sanitização

### 3.1 Contenção de Diretórios (`isPathContained`)
Garante que nenhum caminho acedido pelo runtime ou editor ultrapassa o diretório-raiz designado:
- Resolve caminhos através de `std::filesystem::weakly_canonical`.
- Rejeita null-bytes (`\0`) e sequências relativas `..`.
- Compara componente a componente a partir da raiz, impedindo escapes por links simbólicos ou diretórios aninhados.

### 3.2 Sanitização de Ficheiros (`sanitizeFilename`)
Elimina caracteres proibidos em sistemas POSIX e NTFS (`<`, `>`, `:`, `"`, `/`, `\`, `|`, `?`, `*`), suprime caracteres de controlo ASCII (0–31), e anula dispositivos de hardware reservados do MS-DOS/Windows (`CON`, `PRN`, `AUX`, `NUL`, `COM1-9`, `LPT1-9`).

---

## 4. Mitigação de Injeção em Folhas de Cálculo (CSV Injection)

O histórico de runs (`runs.csv`) é frequentemente inspecionado em ferramentas analíticas como Excel, Calc ou Google Sheets. 

Para prevenir a execução automática de comandos maliciosos via fórmulas:
1. Se qualquer campo (`campaignName`, `campaignID`) iniciar com `=`, `+`, `-` ou `@`, é prefixado com um apóstrofo `'`, convertendo-o estritamente em texto plano.
2. Campos que contenham vírgulas, quebras de linha (`\n`, `\r`) ou aspas são encapsulados em aspas duplas com escape (`""`), em conformidade estrita com o RFC 4180.

---

## 5. Transações Atómicas e Resiliência a Falhas

A função `core::Security::writeSecureFile`:
1. Gera um ficheiro temporário único no mesmo diretório de destino: `<caminho>.tmp.<pid>.<timestamp>`.
2. Escreve o cabeçalho `ASCENDENDO_SECURE_V1`, o HMAC-SHA256 calculado e a carga útil.
3. Força a escrita e fecha os descritores de ficheiro.
4. Executa a substituição atómica via `std::filesystem::rename` (sobrepondo o ficheiro anterior de forma atómica ao nível do sistema de ficheiros).

Se o processo for interrompido a meio da escrita por falha elétrica ou fecho forçado da aplicação, o ficheiro de save original mantém-se 100% íntegro e não corrompido.

---

## 6. Validação e Testes Automatizados

O subsistema de segurança é coberto por testes unitários exaustivos em `Tests/Unit/test_security.cpp` e `Tests/Unit/test_profile_storage.cpp`:
- Vetores NIST SHA-256 oficiais.
- Vetores RFC 2104 HMAC-SHA256 oficiais.
- Verificação de resistência a adulteração (tampering bit-flip test).
- Testes de contenção de caminho com tentativas deliberadas de directory traversal.
- Testes de sanitização de dispositivos reservados Windows.
- Testes de neutralização de fórmulas maliciosas em CSV.

---

## 7. Arquitetura de Identidade Federada & Raio de Explosão Zero (StudioAuth)

O ASCENDENDO adota formalmente o **Guia Canónico de Arquitetura de Segurança e Decisões de Identidade** do estúdio, garantindo que a comodidade do utilizador nunca compromete o isolamento entre projetos (*Zero Blast Radius*).

### 7.1 Filosofia do Raio de Explosão Zero (*Zero Blast Radius*)

O utilizador pode utilizar a mesma identidade global (ex.: Conta Google ou Conta Studio), mas o subsistema deriva identificadores criptográficos matematicamente estanques para cada projeto:

```mermaid
flowchart TD
    subgraph Utilizador ["👤 Jogador"]
        Master["Conta Google ou Studio ID"]
    end

    subgraph Nucleo ["🔐 studio_auth (Derivação Criptográfica Unidirecional)"]
        KDF1["HMAC-SHA256(Master, Salt_Ascendendo)"]
        KDF2["HMAC-SHA256(Master, Salt_OutroJogo)"]
    end

    subgraph Isolamento ["🛡️ Ecossistema Estanque (Blast Radius = 0)"]
        JA["▲ ASCENDENDO<br/>PlayerID: as_71037d7c...<br/>Token aud: 'ascendendo-client'"]
        JB["🎮 Outro Projeto (ex.: RedWar)<br/>PlayerID: re_f8a90e80...<br/>Token aud: 'outro-client'"]
    end

    Master --> KDF1 --> JA
    Master --> KDF2 --> JB
    JA -.x|"IMPOSSÍVEL CORRELACIONAR<br/>OU REUTILIZAR CREDENCIAIS"| JB
```

- **Padrão Normativo**: **Pairwise Pseudonymous Subject Identifiers (PPID)**, em conformidade com o **OpenID Connect Core 1.0 (Secção 8)** e **NIST SP 800-63B**.
- **Derivação Unidirecional**: O runtime e o portal web nunca armazenam a identidade mestre ou email do jogador:
  $$\text{PlayerID}_{\text{Ascendendo}} = \text{as\_}\langle\text{HMAC-SHA256}(\text{SectorSalt} : \text{"ascendendo"}, \text{MasterSubjectID})[:16]\rangle$$
- **Imunidade a Vazamentos**: Mesmo em caso de extração completa de ficheiros locais de um jogo, é matematicamente impossível reconstruir o identificador de outro projeto ou aceder à conta Google do jogador.

### 7.2 Os 3 Níveis de Identidade

| Nível | Mecanismo | Aplicação no ASCENDENDO | Risco de Privacidade |
|---|---|---|---|
| **1. Convidado (Guest ID)** | Gera um identificador aleatório guardado apenas no dispositivo (`localStorage` / `userdata`). | Jogo imediato sem qualquer formulário ou registo. | **Zero absoluto** (sem dados pessoais). |
| **2. Conectar com Google (OIDC)** | Validação de token JWT assinado criptograficamente pela Google, convertendo o `sub` em Pairwise ID (`as_...`). | Sincronização segura de recordes e campanhas sem expor credenciais reais. | **Mínimo e Auditado** (zero senhas, 2FA herdado da Google). |
| **3. Conta Dedicada Studio / Local** | Cofre criptográfico HMAC-SHA256 com PBKDF2 e salt aleatório de 256 bits gerado por CSPRNG. | Utilizadores que preferem manter o jogo 100% offline e independente. | **Isolado hermeticamente** no subdiretório do utilizador. |

### 7.3 Decisões sobre a Conta Google

1. **Zero Acesso a Palavras-passe:** O jogo nunca vê, recebe ou manipula credenciais da Google.
2. **Menor Privilégio (*Least Privilege*):** Pedem-se apenas os escopos estritamente necessários (`openid` e `profile`). Nenhuma permissão de Gmail, Drive ou contactos é alguma vez solicitada.
3. **Segurança Herdada:** O login Google herda autenticação multi-fator (2FA), Passkeys e biometria do telemóvel sem qualquer custo adicional.

### 7.4 Defesas Ativas e Integridade de Rede

* **Proteção contra Timing Attacks:** Toda a validação de palavras-passe e assinaturas opera em tempo constante (`core::Security::constantTimeEquals` em C++, `hmac.compare_digest` em Python).
* **Zero Dependências Externas na Cadeia de Suprimentos:** O pacote `packages/studio_auth` foi concebido sem dependências externas no PyPI, anulando vetores de *Supply Chain Attacks*.
* **Teto Rígido de Payload (64 KB):** Quaisquer mensagens ou uploads de pacotes de dados acima de 64 KB são descartados na camada de transporte.
* **Tokens com Expiração Curta (1h):** Tokens de sessão expiram em 3600 segundos e contêm verificação estrita de audiência (`aud: "ascendendo-client"`).

