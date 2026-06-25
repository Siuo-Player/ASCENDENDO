#!/usr/bin/env python3
"""
ai_validator.py — Validador de niveis do ASCENDENDO.
Verifica se cada nivel da campanha e fisicamente passivel.
Uso:
    python3 ai_validator.py nivel.lvl          # valida um nivel
    python3 ai_validator.py --campaign         # valida toda a campanha
"""
import sys, os, math

# Forcar UTF-8 no Windows (evita UnicodeEncodeError com emojis no cp1252)
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

LOGICAL_WIDTH  = 640.0
LOGICAL_HEIGHT = 360.0
G_val          = 980.0
V_MAX          = 600.0
ANGLE          = math.pi / 3.0
TOLERANCE      = 0.90

VY_eff   = V_MAX * math.sin(ANGLE) * TOLERANCE
VX_eff   = V_MAX * math.cos(ANGLE) * TOLERANCE
MAX_JUMP = (VY_eff**2) / (2 * G_val)

CAMPAIGN_FILE = os.path.join(os.path.dirname(__file__),
                             "..", "..", "Game", "Assets", "Levels", "campaign.txt")

def validate_level(filepath: str) -> tuple[bool, str]:
    """Valida um ficheiro .lvl. Retorna (valido, mensagem)."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except FileNotFoundError:
        return False, f"Ficheiro nao encontrado: {filepath}"
    except Exception as e:
        return False, f"Erro ao ler {filepath}: {e}"

    platforms = []
    goal      = {'type': 'top'}

    for i, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith('#'): continue
        parts = line.split()
        if parts[0] == "NAME":
            continue
        elif parts[0] == "PLATFORM":
            if len(parts) != 5:
                return False, f"Linha {i}: formato invalido — esperado 'PLATFORM x y w h'"
            x, y, w, h = map(float, parts[1:])
            if x < 0 or (x + w) > LOGICAL_WIDTH:
                return False, f"Linha {i}: plataforma fora dos limites X ([{x},{x+w}] vs [0,{LOGICAL_WIDTH}])"
            if (y + h) > LOGICAL_HEIGHT:
                return False, f"Linha {i}: plataforma ultrapassa a altura da tela (topo {y+h} > {LOGICAL_HEIGHT})"
            platforms.append({'type': 'platform', 'bounds': (x, y, w, h)})
        elif parts[0] == "FLAG":
            if len(parts) != 5:
                return False, f"Linha {i}: formato invalido — esperado 'FLAG x y w h'"
            x, y, w, h = map(float, parts[1:])
            if (y + h) > LOGICAL_HEIGHT:
                return False, f"Linha {i}: FLAG ultrapassa a altura da tela"
            goal = {'type': 'flag', 'bounds': (x, y, w, h)}
        else:
            return False, f"Linha {i}: directiva desconhecida '{parts[0]}'"

    if not platforms and goal['type'] == 'top':
        return True, "Nivel vazio (aceitavel)"

    # BFS com fisica real
    ground = {'type': 'ground', 'bounds': (0, 0, LOGICAL_WIDTH, 20)}
    nodes  = [ground] + platforms + [goal]
    visited = {0}
    queue   = [0]

    while queue:
        curr = queue.pop(0)
        if curr == len(nodes) - 1:
            return True, "Caminho fisicamente possivel encontrado"
        for j in range(1, len(nodes)):
            if j in visited: continue
            p1 = nodes[curr]['bounds']
            n2 = nodes[j]
            y_start = p1[1] + p1[3]
            if n2['type'] == 'top':
                y_end = LOGICAL_HEIGHT + 20; dx = 0.0
            elif n2['type'] == 'flag':
                p2 = n2['bounds']; y_end = p2[1]
                dx = max(0.0, p2[0]-(p1[0]+p1[2]), p1[0]-(p2[0]+p2[2]))
            else:
                p2 = n2['bounds']; y_end = p2[1] + p2[3]
                dx = max(0.0, p2[0]-(p1[0]+p1[2]), p1[0]-(p2[0]+p2[2]))
            dy = y_end - y_start
            if dy > MAX_JUMP: continue
            disc = VY_eff**2 - 2 * G_val * max(0.0, dy)
            if disc < 0: continue
            max_dx = VX_eff * (VY_eff + math.sqrt(disc)) / G_val
            if dx <= max_dx:
                visited.add(j); queue.append(j)

    return False, "Nenhum caminho fisicamente possivel — nivel impossivel"

def validate_campaign(campaign_path: str) -> bool:
    """Valida todos os niveis listados em campaign.txt. Retorna True se todos OK."""
    base = os.path.dirname(campaign_path)
    try:
        with open(campaign_path, 'r', encoding='utf-8') as f:
            level_files = [l.strip() for l in f if l.strip() and not l.startswith('#')]
    except FileNotFoundError:
        print(f"[ERRO] campaign.txt nao encontrado: {campaign_path}")
        return False

    all_ok = True
    for lf in level_files:
        path = os.path.join(base, lf)
        ok, msg = validate_level(path)
        status = "[OK]   " if ok else "[ERRO] "
        print(f"  {status} {lf}: {msg}")
        if not ok:
            all_ok = False

    return all_ok

def main():
    if len(sys.argv) < 2:
        print("Uso: python3 ai_validator.py <nivel.lvl>")
        print("     python3 ai_validator.py --campaign")
        sys.exit(1)

    if sys.argv[1] == "--campaign":
        # Tentar encontrar campaign.txt relativo a este script
        script_dir = os.path.dirname(os.path.abspath(__file__))
        campaign   = os.path.join(script_dir, "..", "..", "Game", "Assets", "Levels", "campaign.txt")
        campaign   = os.path.normpath(campaign)
        # Fallback: a partir da directoria de trabalho actual
        if not os.path.exists(campaign):
            campaign = os.path.normpath("Game/Assets/Levels/campaign.txt")

        print(f"\n  Campanha: {campaign}\n")
        ok = validate_campaign(campaign)
        print()
        if ok:
            print("  [OK] Campanha validada -- todos os niveis sao passiveis.")
        else:
            print("  [ERRO] Campanha INVALIDA -- ver erros acima.")
        sys.exit(0 if ok else 1)
    else:
        path = sys.argv[1]
        ok, msg = validate_level(path)
        status = "[OK]  " if ok else "[ERRO]"
        print(f"  {status} {path}: {msg}")
        sys.exit(0 if ok else 1)

if __name__ == "__main__":
    main()
