#!/usr/bin/env python3
import sys
import math

# Constantes da Física do teu C++
G = 980.0
V_MAX = 800.0
ANGLE = math.radians(60)

# MARGEM DE ERRO: O validador assume que o jogador só consegue usar 85% do salto
TOLERANCE = 0.85 
VY = V_MAX * math.sin(ANGLE) * TOLERANCE
VX = V_MAX * math.cos(ANGLE) * TOLERANCE

MAX_JUMP_Y = (VY**2) / (2 * G)
LOGICAL_WIDTH = 360.0
LOGICAL_HEIGHT = 640.0

def validate_jump(n1, n2):
    """Calcula matematicamente se um salto entre dois nós é possível."""
    p1 = n1['bounds']
    y_start = p1[1] + p1[3] # Salta sempre do topo da origem
    
    t2 = n2['type']
    if t2 == 'top':
        y_end = LOGICAL_HEIGHT
        dx = 0.0 # Pode cruzar a linha do topo em qualquer lado
    elif t2 == 'flag':
        p2 = n2['bounds']
        y_end = p2[1] # Basta tocar na base da bandeira
        if p1[0] + p1[2] < p2[0]: dx = p2[0] - (p1[0] + p1[2])
        elif p1[0] > p2[0] + p2[2]: dx = p1[0] - (p2[0] + p2[2])
        else: dx = 0.0
    else: # platform
        p2 = n2['bounds']
        y_end = p2[1] + p2[3]
        if p1[0] + p1[2] < p2[0]: dx = p2[0] - (p1[0] + p1[2])
        elif p1[0] > p2[0] + p2[2]: dx = p1[0] - (p2[0] + p2[2])
        else: dx = 0.0
        
    dy = y_end - y_start
    
    if dy > MAX_JUMP_Y: return False
    
    if dy > 0:
        discriminant = VY**2 - 2 * G * dy
        if discriminant < 0: return False
        t_max = (VY + math.sqrt(discriminant)) / G
    else:
        t_max = (VY + math.sqrt(VY**2 - 2 * G * dy)) / G
        
    max_dx = VX * t_max
    return dx <= max_dx

def validate_level(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except: return False

    platforms = []
    goal_node = {'type': 'top'} # Padrão: chegar aos 640px

    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'): continue
        parts = line.split()
        if parts[0] == "PLATFORM":
            x, y, w, h = map(float, parts[1:])
            platforms.append({'type': 'platform', 'bounds': (x, y, w, h)})
        elif parts[0] == "FLAG":
            x, y, w, h = map(float, parts[1:])
            goal_node = {'type': 'flag', 'bounds': (x, y, w, h)}

    # Nós do Grafo: [Chão] + [Plataformas...] + [Objetivo]
    nodes = [{'type': 'ground', 'bounds': (0, 0, LOGICAL_WIDTH, 0)}] + platforms + [goal_node]
    
    # Pesquisa em Grafo (BFS) para encontrar se há um caminho desde o chão até ao fim
    visited = set([0])
    queue = [0]
    
    while queue:
        curr = queue.pop(0)
        
        # Se chegou ao objetivo final, o nível tem solução!
        if curr == len(nodes) - 1:
            print(f"  ✅ [VALIDADO] Há caminho possível em: {filepath}")
            return True
            
        for i in range(1, len(nodes)):
            if i not in visited:
                if validate_jump(nodes[curr], nodes[i]):
                    visited.add(i)
                    queue.append(i)
                    
    print(f"  ❌ [ERRO GRAVE] Nível impossível: {filepath} (Mesmo usando saltos perfeitos)")
    return False

def validate_campaign(campaign_file):
    print("\n==================================================")
    print(" ASCENDENDO: AI Validation (Margem de Erro: 15%)")
    print("==================================================")
    try:
        with open(campaign_file, 'r', encoding='utf-8') as f:
            levels = [line.strip() for line in f if line.strip() and not line.startswith('#')]
    except FileNotFoundError:
        print(f"❌ ERRO: Playlist não encontrada.")
        return False

    all_valid = True
    for lvl in levels:
        full_path = f"Game/Assets/Levels/{lvl}"
        if not validate_level(full_path): all_valid = False

    return all_valid

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python ai_validator.py <ficheiro.lvl> OU --campaign")
        sys.exit(1)
    
    if sys.argv[1] == "--campaign":
        sys.exit(0 if validate_campaign("Game/Assets/Levels/campaign.txt") else 1)
    else:
        sys.exit(0 if validate_level(sys.argv[1]) else 1)