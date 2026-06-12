#!/usr/bin/env python3
import sys
from pathlib import Path

LOGICAL_WIDTH = 360.0

def validate_level(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"❌ ERRO: Ficheiro não encontrado -> {filepath}")
        return False

    has_name = False
    valid = True

    print(f"A validar: {filepath}...")

    for i, line in enumerate(lines):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        
        parts = line.split()
        if parts[0] == "NAME":
            has_name = True
        elif parts[0] == "PLATFORM":
            if len(parts) != 5:
                print(f"  ❌ Linha {i+1}: Formato inválido. Uso: PLATFORM x y w h")
                valid = False
                continue
            
            try:
                x, y, w, h = map(float, parts[1:])
                if x < 0.0 or (x + w) > LOGICAL_WIDTH:
                    print(f"  ❌ Linha {i+1}: Plataforma fora dos limites! (X: {x}, W: {w}) -> Extremo: {x+w}. Limite máximo é {LOGICAL_WIDTH}.")
                    valid = False
            except ValueError:
                print(f"  ❌ Linha {i+1}: Valores não numéricos nas coordenadas.")
                valid = False
        elif parts[0] == "FLAG":
            pass # Ignora a flag nesta validação simples

    if not has_name:
        print(f"  ⚠️  Aviso: O nível devia ter um NAME definido.")
    
    if valid:
        print(f"  ✅ Nível OK.")
    
    return valid

def validate_campaign(campaign_file):
    print(f"\n--- A INICIAR VALIDAÇÃO DA CAMPANHA ---")
    try:
        with open(campaign_file, 'r', encoding='utf-8') as f:
            levels = [line.strip() for line in f if line.strip() and not line.startswith('#')]
    except FileNotFoundError:
        print(f"❌ ERRO: Ficheiro de campanha não encontrado -> {campaign_file}")
        return False

    all_valid = True
    for lvl in levels:
        # A CORREÇÃO ESTÁ AQUI: Adiciona o caminho completo
        full_path = f"Game/Assets/Levels/{lvl}"
        if not validate_level(full_path):
            all_valid = False

    return all_valid

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python level_validator.py <ficheiro.lvl> OU --campaign")
        sys.exit(1)
    
    arg = sys.argv[1]
    
    if arg == "--campaign":
        success = validate_campaign("Game/Assets/Levels/campaign.txt")
        sys.exit(0 if success else 1)
    else:
        success = validate_level(arg)
        sys.exit(0 if success else 1)