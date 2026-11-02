# ADR 0001 — Subsistema Criptográfico Nativo e Integridade de Dados

- **Estado:** Aprovado e Implementado
- **Data:** 2026-11-02
- **Autor:** Antigravity (com Rafael Gomes Bernardo)

## Contexto

O ASCENDENDO requer persistência segura de perfis de utilizador, progresso de campanha, tempos e histórico de runs (`runs.csv`), bem como manipulação segura de ficheiros de níveis e ambiente. O projeto privilegia uma compilação Windows/Linux sem dependências dinâmicas externas pesadas (como OpenSSL).

## Decisão

1. Implementar um subsistema C++20 nativo e autónomo (`Game/Core/Security.h`, `Game/Core/Security.cpp`) com os padrões oficiais:
   - NIST FIPS 180-4 SHA-256.
   - RFC 2104 HMAC-SHA256.
2. Usar o envelope binário autenticado `ASCENDENDO_SECURE_V1` para proteger saves e perfis de utilizador (`profile.secure`).
3. Adotar verificação em tempo constante (`constantTimeEquals`) com acumulador volátil contra ataques de canal lateral (timing attacks).
4. Proteger contra directory traversal e symlinks arbitrários através de `isPathContained`.
5. Sanitizar nomes de ficheiro proibindo caracteres ilegais e dispositivos de hardware reservados do Windows (`CON`, `PRN`, `AUX`, `NUL`, etc.).
6. Prevenir injeção de fórmulas CSV (`=`, `+`, `-`, `@`) neutralizando prefixos e escapando aspas no registo de histórico (`runs.csv`).
7. Implementar transações atómicas de ficheiros (gravação em `.tmp` com rename atómico) para imunidade a corrupção por crash ou perda de energia.
8. Encapsular a leitura de variáveis de ambiente em `core::getEnvironmentVariable` usando `_dupenv_s` no Windows para evitar alertas de thread-safety com `std::getenv`.

## Consequências

- Segurança absoluta e determinismo entre plataformas sem dependências externas.
- Testes unitários com vetores de teste NIST/RFC oficiais passam com 100% de sucesso.
- Ficheiros adulterados são rejeitados de forma silenciosa e segura, registando alerta e retornando estado default limpo.
