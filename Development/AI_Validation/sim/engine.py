#!/usr/bin/env python3
"""
Simulador fiel ao motor C++ (Physics.cpp v7.2, Player.cpp v7.2, Level.cpp v7.2).
Replica passo-a-passo (FIXED_STEP) em vez de formulas continuas, para apanhar
qualquer efeito de discretizacao/colisao que a analise analitica esconde.
"""
import math, random

# ---- Config.h ----
LOGICAL_WIDTH  = 640.0
LOGICAL_HEIGHT = 360.0
FIXED_STEP     = 1.0/60.0
GRAVITY        = -980.0
PLAYER_WIDTH   = 16.0
PLAYER_HEIGHT  = 16.0
PLAYER_MOVE_SPEED = 140.0
PLAYER_MIN_JUMP   = 250.0
PLAYER_MAX_JUMP   = 600.0
PLAYER_CHARGE_TIME = 0.4
SKIN_WIDTH = 1.0
COS60, SIN60 = 0.5, 0.866

class Body:
    def __init__(self, x, y):
        self.x, self.y = x, y
        self.vx, self.vy = 0.0, 0.0
        self.grounded = True
    def bounds(self):
        return (self.x, self.y, self.x+PLAYER_WIDTH, self.y+PLAYER_HEIGHT)

def overlaps(a, b):
    return a[2] > b[0] and a[0] < b[2] and a[3] > b[1] and a[1] < b[3]

def physics_step(body, dt):
    """Physics.cpp v7.2 :: PhysicsWorld::step"""
    if not body.grounded:
        body.vy += GRAVITY*dt
    body.x += body.vx*dt
    body.y += body.vy*dt
    body.grounded = False
    if body.x < 0.0:
        body.x = 0.0; body.vx = 0.0
    elif body.x > LOGICAL_WIDTH - PLAYER_WIDTH:
        body.x = LOGICAL_WIDTH - PLAYER_WIDTH; body.vx = 0.0
    if body.y <= 0.0:
        body.y = 0.0
        if body.vy < 0.0: body.vy = 0.0
        body.grounded = True

def resolve_collision(body, platforms):
    """Level.cpp v7.2 :: resolveCollision (replica exacta, incl. MTV lateral/teto)."""
    collided = False
    bx0, by0, bx1, by1 = body.bounds()
    for (px0, py0, px1, py1) in platforms:
        bx0, by0, bx1, by1 = body.bounds()
        probe = (bx0, by0-SKIN_WIDTH, bx1, by1)
        if not overlaps(probe, (px0,py0,px1,py1)):
            continue
        bAABB = (bx0,by0,bx1,by1)
        if not overlaps(bAABB, (px0,py0,px1,py1)):
            if body.vy <= 0.0:
                body.y = py1
                body.vy = 0.0
                body.grounded = True
                collided = True
            continue
        exitLeft  = bx1 - px0
        exitRight = px1 - bx0
        exitUp    = py1 - by0
        exitDown  = by1 - py0
        minExitX = min(exitLeft, exitRight)
        minExitY = min(exitUp, exitDown)
        lateral = abs(body.vx) > abs(body.vy) and minExitX <= minExitY
        if lateral:
            if exitLeft < exitRight:
                body.x = px0 - PLAYER_WIDTH
            else:
                body.x = px1
            body.vx = -body.vx*0.3
            collided = True
        else:
            if exitUp <= exitDown:
                if body.vy <= 0.0:
                    body.y = py1
                    body.vy = 0.0
                    body.grounded = True
                    collided = True
            else:
                if body.vy > 0.0:
                    body.y = py0 - PLAYER_HEIGHT
                    body.vy = -body.vy*0.3
                    body.vx = body.vx*0.9
                    collided = True
    return collided

def do_jump(body, direction, charge_fraction):
    """Player.cpp v7.2 :: aplica a velocidade de salto (release com carga acumulada)."""
    charge_fraction = max(0.0, min(1.0, charge_fraction))
    force = PLAYER_MIN_JUMP + (PLAYER_MAX_JUMP-PLAYER_MIN_JUMP)*charge_fraction
    body.vy = force*SIN60
    body.vx = force*COS60*direction
    body.grounded = False

def simulate_jump(start_x, start_y, direction, charge_fraction, platforms,
                   max_seconds=3.0):
    """Simula um salto a partir de uma posicao parada (grounded), devolve
       (pousou:bool, x_final, y_final, frames, bateu_algo_no_caminho:bool)."""
    body = Body(start_x, start_y)
    body.grounded = True
    do_jump(body, direction, charge_fraction)
    steps = int(max_seconds / FIXED_STEP)
    landed = False
    hit_something = False
    for _ in range(steps):
        physics_step(body, FIXED_STEP)
        c = resolve_collision(body, platforms)
        if c:
            hit_something = True
        if body.grounded and body.vy == 0.0:
            landed = True
            break
    return landed, body.x, body.y, body.vx, body.vy

def simulate_jump_flag(start_x, start_y, direction, charge_fraction, platforms, flag_rect,
                        max_seconds=3.0):
    """Como simulate_jump, mas a FLAG nao e uma plataforma solida (Level.cpp guarda-a
       separada, so' verificada por overlap de AABB em main.cpp). Devolve True assim
       que o corpo tocar a FLAG em QUALQUER momento do voo, mesmo sem pousar."""
    body = Body(start_x, start_y)
    body.grounded = True
    do_jump(body, direction, charge_fraction)
    steps = int(max_seconds / FIXED_STEP)
    for _ in range(steps):
        physics_step(body, FIXED_STEP)
        resolve_collision(body, platforms)
        bx0, by0, bx1, by1 = body.bounds()
        if overlaps((bx0,by0,bx1,by1), flag_rect):
            return True, body.x, body.y
        if body.grounded and body.vy == 0.0:
            break  # pousou nalgum lado sem tocar a flag
    return False, body.x, body.y
