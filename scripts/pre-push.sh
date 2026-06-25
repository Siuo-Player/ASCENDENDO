#!/usr/bin/env bash
# scripts/pre-push.sh
# Valida fisicamente a campanha inteira antes do push.

echo "============================================================"
echo " ASCENDENDO: AI Pre-Push Validation"
echo "============================================================"

python3 Development/AI_Validation/ai_validator.py --campaign

if [ $? -ne 0 ]; then
    echo ""
    echo "[FALHA] A fisica provou que a campanha e impossivel de bater ou tem erros!"
    echo "Corrige os distanciamentos das plataformas nos teus .lvl"
    echo "============================================================"
    exit 1
fi

echo "[OK] Fisica de salto e limites verificados matematicamente."
echo "============================================================"
exit 0
