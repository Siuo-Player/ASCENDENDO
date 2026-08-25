# Process incident — 2026-08-26

Durante o início da decomposição de `test_keybindings.cpp`, um novo ficheiro foi criado acidentalmente em `main` antes da criação da branch de trabalho.

## Correção

- o commit acidental é preservado como ponto de partida da branch de trabalho;
- o ficheiro será removido de `main` imediatamente;
- nenhuma alteração funcional adicional será feita em `main` nesta sequência;
- o work package continuará numa branch dedicada com CI antes de qualquer integração.

## Regra reforçada

```text
criar branch
→ investigar/documentar
→ implementar
→ validar
→ PR
→ integrar
```
