#!/usr/bin/env bash
# scripts/pre-push.sh
# Valida a consistência do repositório e a física da campanha antes do push.

set -e

echo "============================================================"
echo " ASCENDENDO: Pre-Push Validation"
echo "============================================================"

echo "-> A validar consistência do repositório..."
python3 Development/Tools/validate_repository_state.py

echo "-> A validar catálogo de campanhas..."
python3 Development/AI_Validation/validate_campaign_catalogue.py

echo "-> A validar reachability mecânica e física..."
python3 Development/AI_Validation/ai_validator.py --campaign

echo "============================================================"
echo " [OK] Consistência, catálogo e física verificados com sucesso!"
echo "============================================================"
exit 0
