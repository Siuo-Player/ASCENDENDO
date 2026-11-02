# Gotcha 0002 — PowerShell no Windows, quebras de linha CRLF e ferramentas

- **Área:** Ambiente / PowerShell / Python Tooling
- **Data:** 2026-11-02

## Problemas e Soluções

1. **PowerShell vs Bash:** Comandos UNIX como `head`, `dir /s` ou aspas triplas em `python -c` falham no PowerShell do Windows. Em vez disso:
   - Usar `Select-Object -First N` em vez de `head -n N`.
   - Usar `Get-ChildItem -Recurse` em vez de `dir /s`.
   - Preferir scripts Python simples para substituição de ficheiros para evitar problemas com parsing de aspas e novas linhas no terminal.
2. **Quebras de linha CRLF (`\r\n`):** Ficheiros no Windows frequentemente contêm CRLF. Ao fazer substituição de texto com `edit_file`, o alvo pode falhar na correspondência de caracteres em branco se a string usar apenas `\n`. É mais seguro usar `write_file` ou scripts com `encoding="utf-8"`.
3. **Buffer de Saída Python:** Ao correr comandos Python em segundo plano, a saída de consola é mantida em buffer pelo SO. Comandos longos (como validação de 610 níveis) podem parecer parados se o log não tiver atualizações imediatas, mas o processo continua em execução normal.
