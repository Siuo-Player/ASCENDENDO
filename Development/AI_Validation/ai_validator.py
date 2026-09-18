#!/usr/bin/env python3
"""
ai_validator.py — Validador de niveis do ASCENDENDO.
Verifica se cada nivel da campanha e fisicamente passivel.
Uso:
    python3 ai_validator.py nivel.lvl          # valida um nivel
    python3 ai_validator.py --campaign         # valida a campanha histórica
    python3 ai_validator.py --catalogue        # valida todas as campanhas catalogadas
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
GROUND_HEIGHT  = 16.0

VY_eff   = V_MAX * math.sin(ANGLE) * TOLERANCE
VX_eff   = V_MAX * math.cos(ANGLE) * TOLERANCE
MAX_JUMP = (VY_eff**2) / (2 * G_val)

ASSETS_ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__),
                                             "..", "..", "Game", "Assets"))
CAMPAIGN_FILE = os.path.join(ASSETS_ROOT, "Levels", "campaign.txt")
CAMPAIGN_CATALOGUE = os.path.join(ASSETS_ROOT, "Campaigns", "catalogue.txt")

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

    for i, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith('#'): continue
        parts = line.split()
        if parts[0] == "NAME":
            continue
        elif parts[0] == "SPAWN":
            return False, f"Linha {i}: SPAWN nao e permitida em .lvl — o spawn e derivado automaticamente"
        elif parts[0] == "PLATFORM":
            if len(parts) != 5:
                return False, f"Linha {i}: formato invalido — esperado 'PLATFORM x y w h'"
            try:
                x, y, w, h = map(float, parts[1:])
            except ValueError:
                return False, f"Linha {i}: coordenadas de plataforma invalidas"
            if not all(math.isfinite(v) for v in (x, y, w, h)):
                return False, f"Linha {i}: coordenadas de plataforma nao finitas"
            if w <= 0 or h <= 0:
                return False, f"Linha {i}: dimensoes de plataforma invalidas"
            if x < 0 or (x + w) > LOGICAL_WIDTH:
                return False, f"Linha {i}: plataforma fora dos limites X ([{x},{x+w}] vs [0,{LOGICAL_WIDTH}])"
            if y < GROUND_HEIGHT:
                return False, f"Linha {i}: plataforma ocupa o chão implícito (Y={y} < {GROUND_HEIGHT})"
            if (y + h) > LOGICAL_HEIGHT:
                return False, f"Linha {i}: plataforma ultrapassa a altura da tela (topo {y+h} > {LOGICAL_HEIGHT})"
            platforms.append({'type': 'platform', 'bounds': (x, y, w, h)})
        elif parts[0] == "FLAG":
            return False, f"Linha {i}: FLAG nao e permitida em .lvl — o objetivo final e derivado automaticamente da plataforma mais alta da campanha"
        else:
            return False, f"Linha {i}: directiva desconhecida '{parts[0]}'"

    if not platforms:
        return True, "Nivel vazio (aceitavel)"

    # BFS com física real. O chão inicial é implícito e ocupa Y=0..16.
    ground = {'type': 'ground', 'bounds': (0, 0, LOGICAL_WIDTH, GROUND_HEIGHT)}
    nodes  = [ground] + platforms
    visited = {0}
    queue   = [0]

    while queue:
        curr = queue.pop(0)
        for j in range(1, len(nodes)):
            if j in visited: continue
            p1 = nodes[curr]['bounds']
            p2 = nodes[j]['bounds']
            y_start = p1[1] + p1[3]
            y_end = p2[1] + p2[3]
            dx = max(0.0, p2[0]-(p1[0]+p1[2]), p1[0]-(p2[0]+p2[2]))
            dy = y_end - y_start
            if dy > MAX_JUMP: continue
            disc = VY_eff**2 - 2 * G_val * max(0.0, dy)
            if disc < 0: continue
            max_dx = VX_eff * (VY_eff + math.sqrt(disc)) / G_val
            if dx <= max_dx:
                visited.add(j); queue.append(j)

    if len(visited) == len(nodes):
        return True, "Plataformas fisicamente alcançaveis"

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

def validate_catalogue(catalogue_path: str) -> bool:
    """Valida mecanicamente todas as playlists referenciadas pelo catálogo."""
    try:
        with open(catalogue_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"[ERRO] catálogo não encontrado: {catalogue_path}")
        return False

    base = os.path.dirname(catalogue_path)
    all_ok = True
    campaigns = 0

    for line_number, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith('#'):
            continue

        fields = [field.strip() for field in line.split('|')]
        if len(fields) != 3:
            print(f"  [ERRO] linha {line_number}: esperado id|nome|playlist")
            all_ok = False
            continue

        campaign_id, name, playlist = fields
        playlist_path = os.path.normpath(os.path.join(base, playlist))
        campaigns += 1

        print(f"\n  Campanha {campaign_id}: {name}")
        print(f"  Playlist: {playlist_path}")
        if not os.path.isfile(playlist_path):
            print("  [ERRO] playlist não encontrada")
            all_ok = False
            continue

        if not validate_campaign(playlist_path):
            all_ok = False

    if campaigns == 0:
        print("  [ERRO] catálogo sem campanhas")
        return False

    return all_ok


def main():
    if len(sys.argv) < 2:
        print("Uso: python3 ai_validator.py <nivel.lvl>")
        print("     python3 ai_validator.py --campaign")
        print("     python3 ai_validator.py --catalogue")
        sys.exit(1)

    if sys.argv[1] == "--campaign":
        print(f"\n  Campanha: {CAMPAIGN_FILE}\n")
        ok = validate_campaign(CAMPAIGN_FILE)
        print()
        if ok:
            print("  [OK] Campanha validada -- todos os niveis sao passiveis.")
        else:
            print("  [ERRO] Campanha INVALIDA -- ver erros acima.")
        sys.exit(0 if ok else 1)
    elif sys.argv[1] == "--catalogue":
        print(f"\n  Catalogo: {CAMPAIGN_CATALOGUE}\n")
        ok = validate_catalogue(CAMPAIGN_CATALOGUE)
        print()
        if ok:
            print("  [OK] Catalogo validado -- todas as campanhas sao fisicamente passiveis.")
        else:
            print("  [ERRO] Catalogo INVALIDO -- ver erros acima.")
        sys.exit(0 if ok else 1)
    else:
        path = sys.argv[1]
        ok, msg = validate_level(path)
        status = "[OK]  " if ok else "[ERRO]"
        print(f"  {status} {path}: {msg}")
        sys.exit(0 if ok else 1)

if __name__ == "__main__":
    main()
