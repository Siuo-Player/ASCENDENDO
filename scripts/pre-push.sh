#!/usr/bin/env bash
# scripts/pre-push.sh
# Valida fisicamente a campanha inteira antes do push.

echo "============================================================"
echo " ASCENDENDO: AI Pre-Push Validation"
echo "============================================================"

# Invoca o modelo matemático de saltos da IA
python3 Development/AI_Validation/ai_validator.py --campaign

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ FALHA: A física provou que a campanha é impossível de bater ou tem erros!"
    echo "Corrige os distanciamentos das plataformas nos teus .lvl"
    echo "============================================================"
    exit 1
fi

echo "✅ Física de salto e limites verificados matematicamente."
echo "============================================================"
exit 0