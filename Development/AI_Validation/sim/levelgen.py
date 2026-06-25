#!/usr/bin/env python3
"""Gerador de niveis v4 — dy=85 intra-nivel, dy=105 cross-chunk (correcto)."""
import sys, random, math, os
sys.path.insert(0, os.path.dirname(__file__))
from engine import (Body, do_jump, physics_step, resolve_collision,
                    simulate_jump, simulate_jump_flag,
                    FIXED_STEP, PLAYER_WIDTH, LOGICAL_WIDTH,
                    PLAYER_MIN_JUMP, PLAYER_MAX_JUMP, GRAVITY)

PLATFORM_H   = 20.0
COS60, SIN60 = 0.5, 0.866
CLEARANCE    = 40.0    # margem minima (corpo a subir vs borda proxima da plataforma)
LEVEL_HEIGHT = 360.0   # altura de cada chunk
OFFSETS      = [0.0, LEVEL_HEIGHT, LEVEL_HEIGHT * 2]   # inicio, zigzag, precipicio

def F(charge):
    return PLAYER_MIN_JUMP + (PLAYER_MAX_JUMP - PLAYER_MIN_JUMP) * charge

def ascending_x_at(start_x, start_y, direction, charge, target_top):
    """X do corpo (borda esq) quando SOBE ate target_top. None se nao alcanca."""
    vy = F(charge) * SIN60
    vx = F(charge) * COS60 * direction
    dy = target_top - start_y
    disc = vy**2 - 2 * abs(GRAVITY) * dy
    if disc < 0:
        return None
    t1 = (vy - math.sqrt(disc)) / abs(GRAVITY)
    return start_x + vx * t1

def fly_descend_to(start_x, start_y, direction, charge, target_top, existing):
    """Simula e devolve o X do corpo quando DESCE ate target_top."""
    body = Body(start_x, start_y)
    body.grounded = True
    do_jump(body, direction, charge)
    peaked = False
    for _ in range(int(5.0 / FIXED_STEP)):
        physics_step(body, FIXED_STEP)
        resolve_collision(body, existing)
        if not peaked and body.vy < 0:
            peaked = True
        if peaked and body.y <= target_top + 2:
            return body.x
        if body.grounded:
            return body.x
    return body.x

def place_platform(land_x, x_asc, direction, w):
    """Posiciona garantindo: [1] land dentro do range; [2] borda proxima fora da zona ascendente."""
    cx = land_x + PLAYER_WIDTH / 2.0
    nx = cx - w / 2.0
    if direction == -1:
        max_right = x_asc - CLEARANCE
        if nx + w > max_right:
            nx = max_right - w
    else:
        min_left = x_asc + PLAYER_WIDTH + CLEARANCE
        if nx < min_left:
            nx = min_left
    return max(2.0, min(LOGICAL_WIDTH - w - 2.0, nx))

def edge_start(plat, direction):
    w = plat[2] - plat[0]
    if w > 500:                              # chao de largura total
        return LOGICAL_WIDTH / 2.0 - PLAYER_WIDTH / 2.0
    return (plat[2] - PLAYER_WIDTH) if direction == 1 else plat[0]

def gen_one(current, direction, charge, width, dy, existing):
    target_top = current[3] + dy
    start_x    = edge_start(current, direction)
    x_asc      = ascending_x_at(start_x, current[3], direction, charge, target_top)
    if x_asc is None:
        return None, f"CARGA INSUFICIENTE (charge={charge:.2f}, dy={dy})"
    land_x = fly_descend_to(start_x, current[3], direction, charge, target_top, existing)
    nx     = place_platform(land_x, x_asc, direction, width)
    plat   = (nx, target_top - PLATFORM_H, nx + width, target_top)
    cl     = (x_asc-(nx+width)) if direction==-1 else (nx-(x_asc+PLAYER_WIDTH))
    info   = (f"{'L' if direction==-1 else 'R'} c={charge*100:.0f}% "
              f"dy={dy} land={land_x:.0f} xasc={x_asc:.0f} cl={cl:.1f} "
              f"→ [{nx:.0f},{nx+width:.0f}] top={target_top:.0f}")
    return plat, info

def verify_all(platforms, directions, charges, noise=0.08, pos=10.0, n=50, seed=42):
    det_ok, rob_ok = True, True
    rng = random.Random(seed)
    for i,(src,tgt,d,cf) in enumerate(zip(platforms,platforms[1:],directions,charges)):
        sx = edge_start(src, d)
        ld,fx,fy,_,__ = simulate_jump(sx, src[3], d, cf, platforms)
        ok = ld and abs(fy-tgt[3])<1.5 and tgt[0]<=fx+PLAYER_WIDTH/2<=tgt[2]
        if not ok: det_ok=False
        succ=0
        for _ in range(n):
            cf_n=max(0,min(1,cf*(1+rng.uniform(-noise,noise))))
            sx_n=sx+rng.uniform(-pos,pos)
            ld,fx,fy,_,__=simulate_jump(sx_n,src[3],d,cf_n,platforms)
            if ld and abs(fy-tgt[3])<1.5 and tgt[0]<=fx+PLAYER_WIDTH/2<=tgt[2]: succ+=1
        pct=100*succ/n
        if pct<85: rob_ok=False
        print(f"    Salto {i+1}: det={'OK' if ok else 'FALHA'}  "
              f"rob={succ}/{n}({pct:.0f}%)  {'✅' if ok and pct>=85 else '❌'}")
    return det_ok and rob_ok

def generate_and_export(charge_intra=0.78, charge_cross=0.85, noise_trials=50):
    """Gera os 3 niveis, verifica e escreve os .lvl em coordenadas locais."""
    DY_INTRA = 85.0    # diferenca de altura dentro de um chunk
    DY_CROSS = 105.0   # diferenca de altura cross-chunk (fixo pela geometria dos chunks)

    all_ok = True
    results = {}       # nivel -> lista de (local_x, local_y, w, h, tipo)

    # ── INICIO ──────────────────────────────────────────────────────────────
    print("=" * 62)
    print("INICIO  (w=180, c_intra=%.0f%%)" % (charge_intra*100))
    print("=" * 62)
    OFF = OFFSETS[0]
    GROUND = (0, OFF, 640, OFF + PLATFORM_H)     # chao full-width em coords mundo
    plats = [GROUND]; dirs = []; charges = []

    for d in [+1, -1, +1]:
        cf = charge_intra
        p, info = gen_one(plats[-1], d, cf, 180, DY_INTRA, plats)
        print("  " + info)
        if p is None: print("  FALHA!"); break
        plats.append(p); dirs.append(d); charges.append(cf)

    print("  -- verify --")
    ok = verify_all(plats, dirs, charges, n=noise_trials)
    all_ok = all_ok and ok
    results['inicio'] = (plats, OFF)

    # ── ZIGZAG ──────────────────────────────────────────────────────────────
    print("\n" + "=" * 62)
    print("ZIGZAG  (w=140, c_cross=%.0f%%, c_intra=%.0f%%)" % (charge_cross*100, charge_intra*100))
    print("=" * 62)
    OFF = OFFSETS[1]
    plats2 = [plats[-1]]; dirs2 = []; charges2 = []

    # Salto 1 e cross-chunk (dy=105) e SEMPRE vai para a ESQUERDA
    # (porque P3 do inicio esta mais a direita)
    p, info = gen_one(plats2[-1], -1, charge_cross, 140, DY_CROSS, plats2)
    print("  CROSS: " + info)
    if p is None: print("  FALHA!"); return
    plats2.append(p); dirs2.append(-1); charges2.append(charge_cross)

    # Saltos internos do zigzag
    for d in [+1, -1, +1]:
        p, info = gen_one(plats2[-1], d, charge_intra, 140, DY_INTRA, plats2)
        print("  " + info)
        if p is None: print("  FALHA!"); break
        plats2.append(p); dirs2.append(d); charges2.append(charge_intra)

    print("  -- verify --")
    ok = verify_all(plats2, dirs2, charges2, n=noise_trials)
    all_ok = all_ok and ok
    results['zigzag'] = (plats2, OFF)

    # ── PRECIPICIO ──────────────────────────────────────────────────────────
    print("\n" + "=" * 62)
    print("PRECIPICIO  (w=100, c_cross=%.0f%%, c_intra=%.0f%%)" % (charge_cross*100, charge_intra*100))
    print("=" * 62)
    OFF = OFFSETS[2]
    plats3 = [plats2[-1]]; dirs3 = []; charges3 = []

    # Cross-chunk: de P4 do zigzag (que fica a direita), va para a esquerda
    p, info = gen_one(plats3[-1], -1, charge_cross, 100, DY_CROSS, plats3)
    print("  CROSS: " + info)
    if p is None: print("  FALHA!"); return
    plats3.append(p); dirs3.append(-1); charges3.append(charge_cross)

    for d in [+1, -1]:
        p, info = gen_one(plats3[-1], d, charge_intra, 100, DY_INTRA, plats3)
        print("  " + info)
        if p is None: print("  FALHA!"); break
        plats3.append(p); dirs3.append(d); charges3.append(charge_intra)

    # FLAG: do ultimo platform do precipicio, 1 salto mais (dir oposta ao ultimo)
    last = plats3[-1]
    last_dir = dirs3[-1]
    fl_d = -last_dir
    fl_sx = edge_start(last, fl_d)
    fl_xasc = ascending_x_at(fl_sx, last[3], fl_d, charge_intra, last[3]+DY_INTRA)
    fl_lx   = fly_descend_to(fl_sx, last[3], fl_d, charge_intra, last[3]+DY_INTRA, plats3)
    fl_top  = last[3] + DY_INTRA
    fl_nx   = place_platform(fl_lx, fl_xasc, fl_d, 120)
    FLAG    = (fl_nx, fl_top - 40, fl_nx + 120, fl_top)  # h=40
    print(f"  FLAG: [{FLAG[0]:.0f},{FLAG[1]:.0f},{FLAG[2]:.0f},{FLAG[3]:.0f}]")

    print("  -- verify --")
    ok = verify_all(plats3, dirs3, charges3, n=noise_trials)
    # verificar FLAG (overlap durante o voo)
    plats_no_flag = list(plats3)
    sx_f = edge_start(last, fl_d)
    succ_f = 0
    rng = random.Random(42)
    for _ in range(noise_trials):
        cf_n = max(0,min(1,charge_intra*(1+rng.uniform(-0.08,0.08))))
        sx_n = sx_f + rng.uniform(-10,10)
        touched,_,__ = simulate_jump_flag(sx_n,last[3],fl_d,cf_n,plats_no_flag,FLAG)
        if touched: succ_f+=1
    pct_f=100*succ_f/noise_trials
    ok_f = pct_f>=85
    print(f"    FLAG: {succ_f}/{noise_trials}({pct_f:.0f}%) {'✅' if ok_f else '❌'}")
    all_ok = all_ok and ok and ok_f
    results['precipicio'] = (plats3, OFF)
    results['FLAG'] = FLAG

    # ── OUTPUT .lvl em coords LOCAIS ────────────────────────────────────────
    print("\n" + "=" * 62)
    print("STATUS: %s" % ("TUDO OK ✅" if all_ok else "REVISAR ❌"))
    print("=" * 62)

    out_dir = os.path.join(os.path.dirname(__file__), "..")

    # INICIO
    off = OFFSETS[0]
    lines = ["NAME Inicio\n",
             "# Chao de largura total (obrigatorio no primeiro nivel)\n",
             f"PLATFORM 0 0 640 20\n"]
    for p in results['inicio'][0][1:]:   # skip ground
        lx,ly = p[0], p[1]-off
        w,h   = p[2]-p[0], p[3]-p[1]
        lines.append(f"PLATFORM {lx:.0f} {ly:.0f} {w:.0f} {h:.0f}\n")
    with open(os.path.join(out_dir, "inicio.lvl"), "w") as f:
        f.writelines(lines)
    print("Escrito: inicio.lvl")

    # ZIGZAG — skip a plataforma de saida do inicio (plats2[0])
    off = OFFSETS[1]
    lines = ["NAME ZigZag\n"]
    for p in results['zigzag'][0][1:]:
        lx,ly = p[0], p[1]-off
        w,h   = p[2]-p[0], p[3]-p[1]
        lines.append(f"PLATFORM {lx:.0f} {ly:.0f} {w:.0f} {h:.0f}\n")
    with open(os.path.join(out_dir, "zigzag.lvl"), "w") as f:
        f.writelines(lines)
    print("Escrito: zigzag.lvl")

    # PRECIPICIO
    off = OFFSETS[2]
    lines = ["NAME O Precipicio\n"]
    for p in results['precipicio'][0][1:]:
        lx,ly = p[0], p[1]-off
        w,h   = p[2]-p[0], p[3]-p[1]
        lines.append(f"PLATFORM {lx:.0f} {ly:.0f} {w:.0f} {h:.0f}\n")
    fl_lx, fl_ly = FLAG[0], FLAG[1]-off
    fl_w, fl_h   = FLAG[2]-FLAG[0], FLAG[3]-FLAG[1]
    lines.append(f"FLAG {fl_lx:.0f} {fl_ly:.0f} {fl_w:.0f} {fl_h:.0f}\n")
    with open(os.path.join(out_dir, "precipicio.lvl"), "w") as f:
        f.writelines(lines)
    print("Escrito: precipicio.lvl")

    return all_ok

if __name__ == "__main__":
    generate_and_export(charge_intra=0.78, charge_cross=0.92, noise_trials=50)
