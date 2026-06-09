#!/usr/bin/env bash
# =============================================================================
#  scripts/pre-commit.sh — Hook de pre-commit
#
#  Bloqueia qualquer commit na main branch se a bateria de testes falhar.
#  Conforme Regra de Versionamento do README: "Nenhum código é aceite na
#  main branch sem passar 100% da bateria de testes."
#
#  Instalação (executar uma vez após git init):
#    cp scripts/pre-commit.sh .git/hooks/pre-commit
#    chmod +x .git/hooks/pre-commit
# =============================================================================

set -euo pipefail

echo ""
echo "  [hook] A verificar testes antes do commit..."
echo ""

# Correr a bateria de testes com compilação paralela (muito mais rápido)
if ! make tests -j8; then
    echo ""
    echo "  ╔══════════════════════════════════════════════════════════╗"
    echo "  ║  ❌ COMMIT BLOQUEADO                                    ║"
    echo "  ║                                                          ║"
    echo "  ║  A bateria de testes falhou.                             ║"
    echo "  ║  Corrige os erros acima antes de fazer commit.           ║"
    echo "  ╚══════════════════════════════════════════════════════════╝"
    echo ""
    exit 1
fi

echo ""
echo "  ✅ Todos os testes passaram. Commit permitido."
echo ""
exit 0