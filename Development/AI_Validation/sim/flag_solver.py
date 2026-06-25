import random
from engine import simulate_jump_flag, PLAYER_WIDTH

def find_charge_for_flag(start_x, start_y, direction, target_center_x, platforms, flag_rect):
    best_cf, best_err = None, float('inf')
    for i in range(101):
        cf = i/100.0
        touched, fx, fy = simulate_jump_flag(start_x, start_y, direction, cf, platforms, flag_rect)
        if not touched: continue
        err = abs(fx - target_center_x)
        if err < best_err:
            best_err, best_cf = err, cf
    return best_cf, best_err

def robustness_test_flag(start_x, start_y, direction, charge_fraction, platforms, flag_rect,
                          n_trials=50, charge_pct_err=0.08, pos_err_px=10.0, seed=42):
    rng = random.Random(seed)
    successes = 0
    fails = []
    for _ in range(n_trials):
        cf_noise = max(0.0, min(1.0, charge_fraction*(1.0+rng.uniform(-charge_pct_err,charge_pct_err))))
        x_noise = start_x + rng.uniform(-pos_err_px, pos_err_px)
        touched, fx, fy = simulate_jump_flag(x_noise, start_y, direction, cf_noise, platforms, flag_rect)
        if touched:
            successes += 1
        else:
            fails.append((cf_noise, x_noise))
    return successes, n_trials, fails
