#!/usr/bin/env python3
import sys, os, random
sys.path.insert(0, os.path.dirname(__file__))
from engine import simulate_jump, PLAYER_WIDTH, PLAYER_HEIGHT

def find_charge_for_target(start_x, start_y, direction, target_center_x, platforms,
                            target_top_y=None, lo=0.0, hi=1.0, iters=40):
    """Procura charge_fraction tal que a aterragem fique o mais perto possivel
       de target_center_x, EXIGINDO que a aterragem aconteca exactamente na
       altura do alvo (target_top_y) — caso contrario podia "validar" um
       pouso no CHAO ou noutra plataforma so porque o X calhou perto."""
    def land(cf):
        landed, fx, fy, vx, vy = simulate_jump(start_x, start_y, direction, cf, platforms)
        return landed, fx, fy

    best_cf, best_err = None, float('inf')
    for i in range(101):
        cf = i/100.0
        landed, lx, ly = land(cf)
        if not landed: continue
        if target_top_y is not None and abs(ly - target_top_y) > 1.5:
            continue  # pousou noutro sitio (chao, outra plataforma) — nao conta
        err = abs(lx - target_center_x)
        if err < best_err:
            best_err, best_cf = err, cf
    return best_cf, best_err

def robustness_test(start_x, start_y, direction, charge_fraction, platforms,
                     target_rect, n_trials=40, charge_pct_err=0.08, pos_err_px=8.0,
                     seed=42):
    """Repete o MESMO salto N vezes com erro aleatorio de +-charge_pct_err na carga
       e +-pos_err_px na posicao de descolagem. Devolve taxa de sucesso e detalhes."""
    rng = random.Random(seed)
    successes = 0
    failures = []
    for _ in range(n_trials):
        cf_noise = charge_fraction * (1.0 + rng.uniform(-charge_pct_err, charge_pct_err))
        cf_noise = max(0.0, min(1.0, cf_noise))
        x_noise = start_x + rng.uniform(-pos_err_px, pos_err_px)
        landed, fx, fy, vx, vy = simulate_jump(x_noise, start_y, direction, cf_noise, platforms)
        # Sucesso: aterrou, o CENTRO do corpo cai dentro do target em X,
        # e a altura final bate com o TOPO do alvo (index 3, nao o index 1/base!)
        body_center = fx + PLAYER_WIDTH/2.0
        ok = landed and target_rect[0] <= body_center <= target_rect[2] and abs(fy - target_rect[3]) < 1.5
        if ok:
            successes += 1
        else:
            failures.append((cf_noise, x_noise, landed, fx, fy))
    return successes, n_trials, failures
