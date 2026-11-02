// ASCENDENDO — Web Canvas Preview & Commitment Jump Simulator
// Replicating 60° commitment jump physics (640x360 logical coordinates)

(function() {
    const canvas = document.getElementById('previewCanvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const coordsEl = document.getElementById('canvasCoords');

    // Constants matching C++ Config.h
    const LOGICAL_WIDTH = 640;
    const LOGICAL_HEIGHT = 360;
    const FIXED_STEP = 1.0 / 60.0;
    const GRAVITY = -980.0;
    const PLAYER_WIDTH = 16.0;
    const PLAYER_HEIGHT = 16.0;
    const PLAYER_MIN_JUMP = 250.0;
    const PLAYER_MAX_JUMP = 600.0;
    const PLAYER_CHARGE_TIME = 0.4;
    const COS60 = 0.5;
    const SIN60 = 0.866;

    // Platforms
    const platforms = [
        { x: 0, y: 0, w: 640, h: 20 },         // Ground
        { x: 192, y: 74, w: 176, h: 16 },      // Plat 1
        { x: 110, y: 138, w: 176, h: 16 },     // Plat 2
        { x: 324, y: 202, w: 160, h: 16 },     // Plat 3
        { x: 96, y: 266, w: 160, h: 16 },      // Plat 4
        { x: 260, y: 310, w: 120, h: 16 }      // Goal platform
    ];

    // Player state
    const player = {
        x: 320 - PLAYER_WIDTH / 2,
        y: 20,
        vx: 0,
        vy: 0,
        grounded: true,
        charging: false,
        chargeTime: 0,
        direction: 1 // 1 for right, -1 for left
    };

    let trail = [];

    // Mouse / Touch handling
    let isMouseDown = false;
    let holdStartTime = 0;

    canvas.addEventListener('mousedown', (e) => {
        isMouseDown = true;
        holdStartTime = performance.now();
        const rect = canvas.getBoundingClientRect();
        const clickX = (e.clientX - rect.left) * (LOGICAL_WIDTH / rect.width);
        // Set jump direction towards click or alternate
        player.direction = clickX > player.x ? 1 : -1;
    });

    window.addEventListener('mouseup', () => {
        if (!isMouseDown) return;
        isMouseDown = false;
        if (player.grounded) {
            const duration = Math.min((performance.now() - holdStartTime) / 1000, PLAYER_CHARGE_TIME);
            const ratio = duration / PLAYER_CHARGE_TIME;
            const speed = PLAYER_MIN_JUMP + (PLAYER_MAX_JUMP - PLAYER_MIN_JUMP) * ratio;
            player.vx = speed * COS60 * player.direction;
            player.vy = speed * SIN60;
            player.grounded = false;
            trail = [];
        }
    });

    canvas.addEventListener('mousemove', (e) => {
        const rect = canvas.getBoundingClientRect();
        const mx = Math.round((e.clientX - rect.left) * (LOGICAL_WIDTH / rect.width));
        const my = Math.round(LOGICAL_HEIGHT - (e.clientY - rect.top) * (LOGICAL_HEIGHT / rect.height));
        if (coordsEl) coordsEl.innerText = `X: ${mx} | Y: ${my}`;
    });

    // Simple AABB overlap
    function checkCollision(px, py) {
        for (const p of platforms) {
            if (px + PLAYER_WIDTH > p.x && px < p.x + p.w &&
                py + PLAYER_HEIGHT > p.y && py < p.y + p.h) {
                return p;
            }
        }
        return null;
    }

    // Main animation loop (fixed timestep physics)
    let lastTime = performance.now();
    let accumulator = 0;

    function loop(time) {
        const dt = Math.min((time - lastTime) / 1000, 0.1);
        lastTime = time;
        accumulator += dt;

        while (accumulator >= FIXED_STEP) {
            updatePhysics(FIXED_STEP);
            accumulator -= FIXED_STEP;
        }

        render();
        requestAnimationFrame(loop);
    }

    function updatePhysics(step) {
        if (!player.grounded) {
            player.vy += GRAVITY * step;
            player.x += player.vx * step;
            player.y += player.vy * step;

            // Wall bounce
            if (player.x <= 0) {
                player.x = 0;
                player.vx = -player.vx * 0.8;
                player.direction = 1;
            } else if (player.x >= LOGICAL_WIDTH - PLAYER_WIDTH) {
                player.x = LOGICAL_WIDTH - PLAYER_WIDTH;
                player.vx = -player.vx * 0.8;
                player.direction = -1;
            }

            // Top boundary
            if (player.y >= LOGICAL_HEIGHT - PLAYER_HEIGHT) {
                player.y = LOGICAL_HEIGHT - PLAYER_HEIGHT;
                player.vy = -100;
            }

            // Platform collision (landing on top)
            if (player.vy < 0) {
                for (const p of platforms) {
                    const platTop = p.y + p.h;
                    if (player.y <= platTop && player.y - player.vy * step >= platTop - 4) {
                        const centerX = player.x + PLAYER_WIDTH / 2;
                        if (centerX >= p.x && centerX <= p.x + p.w) {
                            player.y = platTop;
                            player.vy = 0;
                            player.vx = 0;
                            player.grounded = true;
                            break;
                        }
                    }
                }
            }

            // Record trajectory trail
            trail.push({ x: player.x + PLAYER_WIDTH / 2, y: player.y + PLAYER_HEIGHT / 2 });
            if (trail.length > 50) trail.shift();
        } else {
            // If charging, track charge
            if (isMouseDown) {
                player.chargeTime = Math.min((performance.now() - holdStartTime) / 1000, PLAYER_CHARGE_TIME);
            } else {
                player.chargeTime = 0;
            }
        }
    }

    function render() {
        ctx.clearRect(0, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT);

        // Background grid
        ctx.strokeStyle = "rgba(0, 240, 255, 0.04)";
        ctx.lineWidth = 1;
        for (let x = 0; x < LOGICAL_WIDTH; x += 32) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, LOGICAL_HEIGHT);
            ctx.stroke();
        }
        for (let y = 0; y < LOGICAL_HEIGHT; y += 32) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(LOGICAL_WIDTH, y);
            ctx.stroke();
        }

        // Draw platforms
        for (let i = 0; i < platforms.length; i++) {
            const p = platforms[i];
            const canvasY = LOGICAL_HEIGHT - (p.y + p.h);

            // Platform fill
            ctx.fillStyle = i === platforms.length - 1 ? "rgba(255, 42, 109, 0.4)" : "rgba(16, 26, 46, 0.9)";
            ctx.fillRect(p.x, canvasY, p.w, p.h);

            // Platform edge glow
            ctx.strokeStyle = i === platforms.length - 1 ? "#ff2a6d" : "#00f0ff";
            ctx.lineWidth = 2;
            ctx.strokeRect(p.x, canvasY, p.w, p.h);

            // Top highlight line
            ctx.fillStyle = i === platforms.length - 1 ? "#ff5f9e" : "#80f8ff";
            ctx.fillRect(p.x, canvasY, p.w, 2);
        }

        // Draw trajectory trail
        if (trail.length > 1) {
            ctx.beginPath();
            ctx.strokeStyle = "rgba(0, 240, 255, 0.4)";
            ctx.lineWidth = 2;
            for (let i = 0; i < trail.length; i++) {
                const pt = trail[i];
                const cy = LOGICAL_HEIGHT - pt.y;
                if (i === 0) ctx.moveTo(pt.x, cy);
                else ctx.lineTo(pt.x, cy);
            }
            ctx.stroke();
        }

        // Draw jump charge prediction arc when holding
        if (player.grounded && isMouseDown) {
            const ratio = player.chargeTime / PLAYER_CHARGE_TIME;
            const speed = PLAYER_MIN_JUMP + (PLAYER_MAX_JUMP - PLAYER_MIN_JUMP) * ratio;
            const vx0 = speed * COS60 * player.direction;
            const vy0 = speed * SIN60;

            ctx.beginPath();
            ctx.strokeStyle = "rgba(255, 184, 0, 0.8)";
            ctx.setLineDash([4, 4]);
            ctx.lineWidth = 2;

            let simX = player.x + PLAYER_WIDTH / 2;
            let simY = player.y + PLAYER_HEIGHT / 2;
            let simVx = vx0;
            let simVy = vy0;

            ctx.moveTo(simX, LOGICAL_HEIGHT - simY);
            for (let step = 0; step < 35; step++) {
                simVy += GRAVITY * FIXED_STEP;
                simX += simVx * FIXED_STEP;
                simY += simVy * FIXED_STEP;
                if (simX < 0 || simX > LOGICAL_WIDTH || simY < 0) break;
                ctx.lineTo(simX, LOGICAL_HEIGHT - simY);
            }
            ctx.stroke();
            ctx.setLineDash([]);
        }

        // Draw Player
        const pCanvasY = LOGICAL_HEIGHT - (player.y + PLAYER_HEIGHT);
        ctx.fillStyle = player.grounded ? "#00f0ff" : "#05ffa1";
        ctx.shadowColor = player.grounded ? "rgba(0, 240, 255, 0.8)" : "rgba(5, 255, 161, 0.8)";
        ctx.shadowBlur = 10;
        ctx.fillRect(player.x, pCanvasY, PLAYER_WIDTH, PLAYER_HEIGHT);
        ctx.shadowBlur = 0;

        // Player face / eye showing direction
        ctx.fillStyle = "#07090e";
        const eyeX = player.direction > 0 ? player.x + 10 : player.x + 2;
        ctx.fillRect(eyeX, pCanvasY + 4, 4, 4);

        // Charge bar over player
        if (player.grounded && isMouseDown) {
            const barW = 24;
            const barH = 4;
            const barX = player.x + PLAYER_WIDTH / 2 - barW / 2;
            const barY = pCanvasY - 10;
            const fillW = barW * (player.chargeTime / PLAYER_CHARGE_TIME);

            ctx.fillStyle = "rgba(0, 0, 0, 0.7)";
            ctx.fillRect(barX, barY, barW, barH);
            ctx.fillStyle = "#ffb800";
            ctx.fillRect(barX, barY, fillW, barH);
            ctx.strokeStyle = "#fff";
            ctx.lineWidth = 1;
            ctx.strokeRect(barX, barY, barW, barH);
        }
    }

    requestAnimationFrame(loop);
})();


// StudioAuth Front-End Integration
(function() {
    if (typeof StudioAuthClient === 'undefined') return;

    const auth = new StudioAuthClient({
        projectSlug: "ascendendo",
        onAuthChange: updateUI
    });

    const statusBtn = document.getElementById('authStatusBtn');
    const authDot = document.getElementById('authDot');
    const authUsername = document.getElementById('authUsername');
    const modal = document.getElementById('authModal');
    const closeModal = document.getElementById('closeAuthModal');
    const modalPlayerId = document.getElementById('modalPlayerId');
    const modalUsername = document.getElementById('modalUsername');
    const modalAuthMode = document.getElementById('modalAuthMode');

    const btnGoogle = document.getElementById('btnGoogleAuth');
    const dedicatedForm = document.getElementById('dedicatedAuthForm');
    const dedicatedUser = document.getElementById('dedicatedUser');
    const dedicatedPass = document.getElementById('dedicatedPass');
    const btnLogout = document.getElementById('btnLogoutAuth');

    function updateUI(session) {
        if (!session) return;
        if (authUsername) authUsername.innerText = session.username;
        if (authDot) {
            authDot.className = session.authMode === 'guest' ? 'auth-dot guest' : 'auth-dot';
        }
        if (modalPlayerId) modalPlayerId.innerText = session.playerId;
        if (modalUsername) modalUsername.innerText = session.username;
        if (modalAuthMode) {
            modalAuthMode.innerText = session.authMode;
            modalAuthMode.className = `badge-mode ${session.authMode}`;
        }
    }

    // Initial render
    updateUI(auth.getSession());

    if (statusBtn && modal) {
        statusBtn.addEventListener('click', () => {
            modal.style.display = 'flex';
        });
    }

    if (closeModal && modal) {
        closeModal.addEventListener('click', () => {
            modal.style.display = 'none';
        });
    }

    if (modal) {
        modal.addEventListener('click', (e) => {
            if (e.target === modal) modal.style.display = 'none';
        });
    }

    if (btnGoogle) {
        btnGoogle.addEventListener('click', () => {
            const mockName = prompt("Simulação Google OIDC:\nIndica o teu nome de jogador para gerar o PPID estanque:", "Jogador");
            if (mockName) {
                auth.loginWithGoogle({ name: mockName.trim() });
                modal.style.display = 'none';
            }
        });
    }

    if (dedicatedForm) {
        dedicatedForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const u = dedicatedUser.value.trim();
            const p = dedicatedPass.value;
            if (u && p) {
                auth.loginWithDedicated(u, p);
                dedicatedUser.value = '';
                dedicatedPass.value = '';
                modal.style.display = 'none';
            }
        });
    }

    if (btnLogout) {
        btnLogout.addEventListener('click', () => {
            auth.logout();
            modal.style.display = 'none';
        });
    }
})();
