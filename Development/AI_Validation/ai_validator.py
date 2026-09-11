#!/usr/bin/env python3
"""Lightweight authoritative-adjacent level reachability validator."""

from __future__ import annotations

import math
import os
import sys

try:
    from sim.engine import LOGICAL_HEIGHT, LOGICAL_WIDTH, G, VY_EFFECTIVE  # type: ignore
except ImportError:
    LOGICAL_WIDTH = 640.0
    LOGICAL_HEIGHT = 360.0
    G = 1000.0
    VY_EFFECTIVE = 420.0

MAX_JUMP = 260.0
VX_EFFECTIVE = 260.0
GROUND_HEIGHT = 16.0
CAMPAIGN_FILE = os.path.join(
    os.path.dirname(__file__), "..", "..", "Game", "Assets", "Levels", "campaign.txt"
)


def validate_level(filepath: str) -> tuple[bool, str]:
    """Valida um ficheiro .lvl. Retorna (valido, mensagem)."""
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            lines = f.readlines()
    except FileNotFoundError:
        return False, f"Ficheiro nao encontrado: {filepath}"
    except Exception as e:
        return False, f"Erro ao ler {filepath}: {e}"

    platforms = []

    for i, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
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
            if not all(math.isfinite(value) for value in (x, y, w, h)):
                return False, f"Linha {i}: coordenadas de plataforma nao finitas"
            if w <= 0 or h <= 0:
                return False, f"Linha {i}: dimensao de plataforma invalida"
            if x < 0 or (x + w) > LOGICAL_WIDTH:
                return False, f"Linha {i}: plataforma fora dos limites X ([{x},{x+w}] vs [0,{LOGICAL_WIDTH}])"
            if y < GROUND_HEIGHT or (y + h) > LOGICAL_HEIGHT:
                return False, f"Linha {i}: plataforma ocupa o chão implícito ou ultrapassa a altura da tela"
            platforms.append({"type": "platform", "bounds": (x, y, w, h)})
        elif parts[0] == "FLAG":
            return False, f"Linha {i}: FLAG nao e permitida em .lvl — o objetivo final e derivado automaticamente da plataforma mais alta da campanha"
        else:
            return False, f"Linha {i}: directiva desconhecida '{parts[0]}'"

    if not platforms:
        return True, "Nivel vazio (aceitavel)"

    # O mundo começa com chão implícito 640x16 e spawn centrado no seu topo.
    ground = {"type": "ground", "bounds": (0.0, 0.0, LOGICAL_WIDTH, GROUND_HEIGHT)}
    nodes = [ground] + platforms
    visited = {0}
    queue = [0]

    while queue:
        curr = queue.pop(0)
        for j in range(1, len(nodes)):
            if j in visited:
                continue
            p1 = nodes[curr]["bounds"]
            p2 = nodes[j]["bounds"]
            y_start = p1[1] + p1[3]
            y_end = p2[1] + p2[3]
            dx = max(0.0, p2[0] - (p1[0] + p1[2]), p1[0] - (p2[0] + p2[2]))
            dy = y_end - y_start
            if dy > MAX_JUMP:
                continue
            disc = VY_EFFECTIVE**2 - 2 * G * max(0.0, dy)
            if disc < 0:
                continue
            max_dx = VX_EFFECTIVE * (VY_EFFECTIVE + math.sqrt(disc)) / G
            if dx <= max_dx:
                visited.add(j)
                queue.append(j)

    if len(visited) == len(nodes):
        return True, "Plataformas fisicamente alcançaveis"

    return False, "Nenhum caminho fisicamente possivel — nivel impossivel"


def validate_campaign(campaign_path: str) -> bool:
    """Valida todos os niveis listados em campaign.txt. Retorna True se todos OK."""
    base = os.path.dirname(campaign_path)
    try:
        with open(campaign_path, "r", encoding="utf-8") as f:
            level_files = [l.strip() for l in f if l.strip() and not l.startswith("#")]
    except FileNotFoundError:
        print(f"[ERRO] campaign.txt nao encontrado: {campaign_path}")
        return False

    all_ok = True
    for lf in level_files:
        path = os.path.join(base, lf)
        ok, msg = validate_level(path)
        status = "[OK]   " if ok else "[ERRO] "
        print(f"{status}{lf}: {msg}")
        if not ok:
            all_ok = False
    return all_ok


def main() -> int:
    if len(sys.argv) > 1 and sys.argv[1] == "--campaign":
        return 0 if validate_campaign(CAMPAIGN_FILE) else 1
    if len(sys.argv) > 1:
        ok, msg = validate_level(sys.argv[1])
        print(("[OK] " if ok else "[ERRO] ") + msg)
        return 0 if ok else 1
    print("Uso: ai_validator.py --campaign | <level.lvl>")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
