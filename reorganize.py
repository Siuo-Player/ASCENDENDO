#!/usr/bin/env python3
"""
reorganize.py — Motor de Reorganizacao do ASCENDENDO
Idempotente: seguro correr multiplas vezes.

Routing de niveis .lvl:
  Valido + listado em campaign.txt  →  Game/Assets/Levels/
  Valido + nao listado              →  Game/Assets/Levels/Unused/
  Invalido / incompleto             →  Game/Assets/Levels/NaoValidados/
"""
import math, shutil, sys, re, base64, subprocess
from pathlib import Path

# Forcar UTF-8 no Windows (evita UnicodeEncodeError com cp1252)
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

ROOT = Path(__file__).parent.resolve()

MOVES = [
    # Infraestrutura
    ("c_cpp_properties.json",   ".vscode/c_cpp_properties.json"),
    ("doctest.h",               "external/doctest/doctest.h"),
    ("test_runner.cpp",         "Tests/test_runner.cpp"),
    ("pre-commit.sh",           "scripts/pre-commit.sh"),
    ("pre-push.sh",             "scripts/pre-push.sh"),
    ("dev_log.txt",             "Development/dev_log.txt"),
    ("project_structure.txt",   "Development/project_structure.txt"),
    ("log_testes.txt",          "Tests/log_testes.txt"),

    # Core
    ("Config.h",                "Game/Core/Config.h"),
    ("CampaignID.h",             "Game/Core/CampaignID.h"),
    ("GameAction.h",             "Game/Core/GameAction.h"),
    ("GameAction.cpp",           "Game/Core/GameAction.cpp"),
    ("KeyBindings.h",            "Game/Core/KeyBindings.h"),
    ("KeyBindings.cpp",          "Game/Core/KeyBindings.cpp"),

    # Testes Unitarios
    ("test_placeholder.cpp",    "Tests/Unit/test_placeholder.cpp"),
    ("test_vulkan_context.cpp", "Tests/Unit/test_vulkan_context.cpp"),
    ("test_input.cpp",          "Tests/Unit/test_input.cpp"),
    ("test_physics.cpp",        "Tests/Unit/test_physics.cpp"),
    ("test_player.cpp",         "Tests/Unit/test_player.cpp"),
    ("test_replay.cpp",         "Tests/Unit/test_replay.cpp"),
    ("test_camera.cpp",         "Tests/Unit/test_camera.cpp"),
    ("test_level.cpp",          "Tests/Unit/test_level.cpp"),
    ("test_campaign.cpp",       "Tests/Unit/test_campaign.cpp"),
    ("test_campaign_id.cpp",    "Tests/Unit/test_campaign_id.cpp"),
    ("test_run_history.cpp",    "Tests/Unit/test_run_history.cpp"),
    ("test_game_action.cpp",    "Tests/Unit/test_game_action.cpp"),
    ("test_keybindings.cpp",    "Tests/Unit/test_keybindings.cpp"),

    # Testes de Integracao
    ("test_vulkan_init.cpp",    "Tests/Integration/test_vulkan_init.cpp"),
    ("test_window.cpp",         "Tests/Integration/test_window.cpp"),
    ("test_swapchain.cpp",      "Tests/Integration/test_swapchain.cpp"),
    ("test_render_pass.cpp",    "Tests/Integration/test_render_pass.cpp"),
    ("test_renderer.cpp",       "Tests/Integration/test_renderer.cpp"),
    ("test_pipeline.cpp",       "Tests/Integration/test_pipeline.cpp"),
    ("test_text_pipeline.cpp",  "Tests/Integration/test_text_pipeline.cpp"),
    ("test_sprite_pipeline.cpp", "Tests/Integration/test_sprite_pipeline.cpp"),

    # Game / Graphics
    ("VulkanContext.h",         "Game/Graphics/VulkanContext.h"),
    ("VulkanContext.cpp",       "Game/Graphics/VulkanContext.cpp"),
    ("BitmapFont.h",            "Game/Graphics/BitmapFont.h"),
    ("TextPipeline.h",          "Game/Graphics/TextPipeline.h"),
    ("TextPipeline.cpp",        "Game/Graphics/TextPipeline.cpp"),
    ("FontRenderer.h",          "Game/Graphics/FontRenderer.h"),
    ("FontRenderer.cpp",        "Game/Graphics/FontRenderer.cpp"),
    ("SpritePipeline.h",        "Game/Graphics/SpritePipeline.h"),
    ("SpritePipeline.cpp",      "Game/Graphics/SpritePipeline.cpp"),
    ("SpriteRenderer.h",        "Game/Graphics/SpriteRenderer.h"),
    ("SpriteRenderer.cpp",      "Game/Graphics/SpriteRenderer.cpp"),
    ("Window.h",                "Game/Graphics/Window.h"),
    ("Window.cpp",              "Game/Graphics/Window.cpp"),
    ("Swapchain.h",             "Game/Graphics/Swapchain.h"),
    ("Swapchain.cpp",           "Game/Graphics/Swapchain.cpp"),
    ("RenderPass.h",            "Game/Graphics/RenderPass.h"),
    ("RenderPass.cpp",          "Game/Graphics/RenderPass.cpp"),
    ("Renderer.h",              "Game/Graphics/Renderer.h"),
    ("Renderer.cpp",            "Game/Graphics/Renderer.cpp"),
    ("Camera.h",                "Game/Graphics/Camera.h"),
    ("Camera.cpp",              "Game/Graphics/Camera.cpp"),
    ("Pipeline.h",              "Game/Graphics/Pipeline.h"),
    ("Pipeline.cpp",            "Game/Graphics/Pipeline.cpp"),

    # Game / Logic
    ("InputManager.h",          "Game/Logic/InputManager.h"),
    ("InputManager.cpp",        "Game/Logic/InputManager.cpp"),
    ("Physics.h",               "Game/Logic/Physics.h"),
    ("Physics.cpp",             "Game/Logic/Physics.cpp"),
    ("Player.h",                "Game/Logic/Player.h"),
    ("Player.cpp",              "Game/Logic/Player.cpp"),
    ("ReplayManager.h",         "Game/Logic/ReplayManager.h"),
    ("ReplayManager.cpp",       "Game/Logic/ReplayManager.cpp"),
    ("Level.h",                 "Game/Logic/Level.h"),
    ("Level.cpp",               "Game/Logic/Level.cpp"),
    ("RunHistory.h",            "Game/Logic/RunHistory.h"),

    # Shaders
    ("base.vert",               "Game/Assets/Shaders/base.vert"),
    ("base.frag",               "Game/Assets/Shaders/base.frag"),
    ("text.vert",               "Game/Assets/Shaders/text.vert"),
    ("text.frag",               "Game/Assets/Shaders/text.frag"),
    ("sprite.vert",             "Game/Assets/Shaders/sprite.vert"),
    ("sprite.frag",             "Game/Assets/Shaders/sprite.frag"),

    # Fontes + bibliotecas stb (header-only)
    ("UIFont.ttf",               "Game/Assets/Fonts/UIFont.ttf"),
    ("stb_truetype.h",          "external/stb/stb_truetype.h"),
    ("stb_image.h",             "external/stb/stb_image.h"),

    # Ferramentas de validacao / geracao
    ("engine.py",               "Development/AI_Validation/sim/engine.py"),
    ("solver.py",               "Development/AI_Validation/sim/solver.py"),
    ("flag_solver.py",          "Development/AI_Validation/sim/flag_solver.py"),
    ("levelgen.py",             "Development/AI_Validation/sim/levelgen.py"),
    ("level_validator.py",      "Development/LevelEditor/level_validator.py"),
    ("ai_validator.py",         "Development/AI_Validation/ai_validator.py"),
]

DIRS_WITH_GITKEEP = [
    ".vscode",
    "Game/Core", "Game/Graphics", "Game/Assets", "Game/Assets/Shaders",
    "Game/Assets/Fonts",
    "Game/Assets/Sprites", "Game/Assets/Sprites/Source",
    "Game/Assets/Levels", "Game/Assets/Levels/NaoValidados", "Game/Assets/Levels/Unused",
    "Game/Logic",
    "Development/LevelEditor", "Development/AI_Validation", "Development/AI_Validation/sim",
    "Development/Runs", "Development/Settings",
    "Tests/Unit", "Tests/Integration", "Tests/System",
    "Tests/Regression", "Tests/Acceptance",
    "external/doctest", "external/stb", "scripts",
]

# Niveis .lvl colocados na raiz pelo utilizador/levelgen
LEVEL_CANDIDATES = ["inicio.lvl", "zigzag.lvl", "precipicio.lvl"]

# Ficheiros .pixil (Pixilart) colocados na raiz. SEM adivinhar papeis
# semanticos a partir do nome (isso ja causou um bug: "personagem.pixil"
# nao correspondia ao "ascended" que estava aqui hardcoded). A conversao
# e' sempre generica e previsivel: nome_do_ficheiro.pixil -> nome_do_ficheiro.png.
# O motor (main.cpp) referencia explicitamente o nome real do ficheiro que
# Rafael escolheu para cada sprite — nao ha adivinhacao nem sinonimos.
PIXIL_NAME_MAP = {
    # Registar aqui APENAS se quiseres um nome de saida DIFERENTE do nome
    # do .pixil (ex: "sprite_bruto_v3": "ground_grass"). Vazio por omissao.
}

# ─── Helpers ─────────────────────────────────────────────────────────────────

def col(text: str, code: str) -> str:
    return f"\033[{code}m{text}\033[0m"

OK   = col("[OK]", "32")
MOVE = col("[MV]", "36")
WARN = col("[!!]", "33")
ERR  = col("[XX]", "31")
DIR  = col("[DR]", "34")
LVL  = col("[LV]", "35")

def read_campaign(campaign_path: Path) -> set:
    """Le campaign.txt e devolve o conjunto de nomes de nivel listados."""
    if not campaign_path.exists():
        return set()
    names = set()
    for line in campaign_path.read_text(encoding='utf-8').splitlines():
        line = line.strip()
        if line and not line.startswith('#'):
            names.add(line)
    return names

def _extract_data_uri_png(data_uri: str) -> bytes:
    """Extrai bytes PNG de um data URI 'data:image/png...base64,<dados>'.
    Tolerante a lixo entre 'image/png' e 'base64,' (visto em exports reais
    do Pixilart — nao se sabe a causa, mas o padrao regex ignora-o)."""
    m = re.match(r'data:image/\w+.*?base64,(.*)', data_uri, re.DOTALL)
    if not m:
        return b''
    try:
        return base64.b64decode(m.group(1))
    except Exception:
        return b''

def convert_pixil_to_png(pixil_path, out_png_path) -> bool:
    """Converte um ficheiro .pixil (Pixilart) para PNG.
    Caso simples (1 layer): extraccao directa dos bytes PNG, zero
    dependencias externas. Caso multi-layer: tenta compor com Pillow (se
    disponivel); se nao estiver, usa so a primeira layer com um aviso.
    """
    import json
    try:
        with open(pixil_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print(f"  {ERR} Nao foi possivel ler {pixil_path.name} como JSON .pixil: {e}")
        return False

    try:
        frame = data['frames'][0]
        layers = frame['layers']
    except (KeyError, IndexError) as e:
        print(f"  {ERR} Estrutura .pixil inesperada em {pixil_path.name}: {e}")
        return False

    if len(layers) == 0:
        print(f"  {ERR} {pixil_path.name}: frame sem layers")
        return False

    if len(layers) == 1:
        png_bytes = _extract_data_uri_png(layers[0]['src'])
        if not png_bytes:
            print(f"  {ERR} {pixil_path.name}: falha ao extrair PNG da layer unica")
            return False
        out_png_path.parent.mkdir(parents=True, exist_ok=True)
        out_png_path.write_bytes(png_bytes)
    else:
        # Multi-layer: tenta compor com Pillow (alpha-over, pela ordem das layers)
        try:
            from PIL import Image
            import io
            composed = None
            for layer in layers:
                if not layer.get('active', True):
                    continue
                png_bytes = _extract_data_uri_png(layer['src'])
                if not png_bytes:
                    continue
                layer_img = Image.open(io.BytesIO(png_bytes)).convert('RGBA')
                if composed is None:
                    composed = Image.new('RGBA', layer_img.size, (0, 0, 0, 0))
                composed.alpha_composite(layer_img)
            if composed is None:
                print(f"  {ERR} {pixil_path.name}: nenhuma layer valida para compor")
                return False
            out_png_path.parent.mkdir(parents=True, exist_ok=True)
            composed.save(out_png_path)
            print(f"  {DIR} {pixil_path.name}: {len(layers)} layers compostas (Pillow)")
        except ImportError:
            print(f"  {WARN} Pillow nao disponivel — {pixil_path.name} tem "
                  f"{len(layers)} layers, a usar so a primeira (pode ficar incompleto)")
            png_bytes = _extract_data_uri_png(layers[0]['src'])
            if not png_bytes:
                return False
            out_png_path.parent.mkdir(parents=True, exist_ok=True)
            out_png_path.write_bytes(png_bytes)

    # Optimizacao sem perda (opcional — melhora automaticamente se instalado).
    # "quero um jogo o mais otimizado possivel... no espaco" (Rafael).
    try:
        result = subprocess.run(
            ["optipng", "-o7", "-quiet", str(out_png_path)],
            capture_output=True, timeout=30
        )
        if result.returncode == 0:
            print(f"  {OK} {out_png_path.name} optimizado sem perdas (optipng -o7)")
    except FileNotFoundError:
        print(f"  {WARN} optipng nao encontrado — {out_png_path.name} guardado sem "
              f"optimizacao extra (instala optipng para reduzir mais o tamanho; "
              f"ver README secção de optimização de espaço)")
    except Exception:
        pass  # optimizacao e' um bonus, nunca deve travar o reorganize

    return True


    """BFS com fisica real (V_MAX=600, TOLERANCE=0.90, LOGICAL_WIDTH=640)."""
    LOGICAL_WIDTH  = 640.0
    LOGICAL_HEIGHT = 360.0
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception:
        return False

    platforms = []
    goal = {'type': 'top'}

    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'): continue
        parts = line.split()
        if parts[0] in ("NAME",): continue
        if parts[0] == "PLATFORM":
            if len(parts) != 5: return False
            x, y, w, h = map(float, parts[1:])
            if x < 0 or (x + w) > LOGICAL_WIDTH: return False
            if (y + h) > LOGICAL_HEIGHT: return False
            platforms.append({'type': 'platform', 'bounds': (x, y, w, h)})
        elif parts[0] == "FLAG":
            if len(parts) != 5: return False
            x, y, w, h = map(float, parts[1:])
            if (y + h) > LOGICAL_HEIGHT: return False
            goal = {'type': 'flag', 'bounds': (x, y, w, h)}

    if not platforms and goal['type'] == 'top':
        return True

    G = 980.0; V_MAX = 600.0; TOL = 0.90; ANGLE = math.pi / 3.0
    VY = V_MAX * math.sin(ANGLE) * TOL
    VX = V_MAX * math.cos(ANGLE) * TOL
    MAX_H = (VY**2) / (2 * G)

    ground = {'type': 'ground', 'bounds': (0, 0, LOGICAL_WIDTH, 20)}
    nodes  = [ground] + platforms + [goal]
    visited = {0}; queue = [0]

    while queue:
        curr = queue.pop(0)
        if curr == len(nodes) - 1:
            return True
        p1 = nodes[curr]['bounds']
        y_start = p1[1] + p1[3]
        for j in range(1, len(nodes)):
            if j in visited: continue
            n2 = nodes[j]
            if n2['type'] == 'top':
                y_end = LOGICAL_HEIGHT + 20; dx = 0.0
            elif n2['type'] == 'flag':
                p2 = n2['bounds']; y_end = p2[1]
                dx = max(0.0, p2[0]-(p1[0]+p1[2]), p1[0]-(p2[0]+p2[2]))
            else:
                p2 = n2['bounds']; y_end = p2[1] + p2[3]
                dx = max(0.0, p2[0]-(p1[0]+p1[2]), p1[0]-(p2[0]+p2[2]))
            dy = y_end - y_start
            if dy > MAX_H: continue
            disc = VY**2 - 2*G*max(0.0, dy)
            if disc < 0: continue
            if dx <= VX * (VY + math.sqrt(disc)) / G:
                visited.add(j); queue.append(j)
    return False

# ─── Main ─────────────────────────────────────────────────────────────────────

def main() -> None:
    print(f"\n{col('  Motor de Reorganizacao -- ASCENDENDO', '1;36')}")
    print("  " + "=" * 55 + "\n")
    moved = 0

    # 1. Criar pastas
    for d in DIRS_WITH_GITKEEP:
        target = ROOT / d
        target.mkdir(parents=True, exist_ok=True)
        gk = target / ".gitkeep"
        if not gk.exists():
            gk.touch()
            print(f"  {DIR} {d}/.gitkeep (criado)")

    # 2. Ficheiros gerais
    for src_name, dst_rel in MOVES:
        src = ROOT / src_name
        dst = ROOT / dst_rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        if src.exists():
            if dst.exists() and src.resolve() != dst.resolve():
                shutil.copy2(str(src), str(dst)); src.unlink()
                print(f"  {MOVE} Atualizado: {dst_rel}"); moved += 1
            elif not dst.exists():
                shutil.move(str(src), str(dst))
                print(f"  {MOVE} Movido:     {dst_rel}"); moved += 1

    # 3. Niveis .lvl — routing inteligente
    campaign_names = read_campaign(ROOT / "Game/Assets/Levels/campaign.txt")
    levels_dir     = ROOT / "Game/Assets/Levels"
    unused_dir     = levels_dir / "Unused"
    invalid_dir    = levels_dir / "NaoValidados"

    for lvl_name in LEVEL_CANDIDATES:
        src = ROOT / lvl_name
        if not src.exists():
            continue
        valid = check_level_validity(str(src))
        if valid:
            if lvl_name in campaign_names:
                dst = levels_dir / lvl_name
                dest_label = f"Game/Assets/Levels/{lvl_name}"
                note = "[campanha]"
            else:
                dst = unused_dir / lvl_name
                dest_label = f"Game/Assets/Levels/Unused/{lvl_name}"
                note = "[nao esta na campanha]"
            if dst.exists() and src.resolve() != dst.resolve():
                shutil.copy2(str(src), str(dst)); src.unlink()
            elif not dst.exists():
                shutil.move(str(src), str(dst))
            print(f"  {LVL} Nivel VALIDO {note}: {dest_label}"); moved += 1
        else:
            dst = invalid_dir / lvl_name
            shutil.move(str(src), str(dst))
            print(f"  {WARN} Nivel INVALIDO -> NaoValidados/: {lvl_name}"); moved += 1

    # 3b. Ficheiros .pixil (Pixilart) -> PNG optimizado
    sprites_dir = ROOT / "Game/Assets/Sprites"
    source_dir  = sprites_dir / "Source"
    for pixil_path in ROOT.glob("*.pixil"):
        # Nome base limpo: "personagem (1).pixil" / "chao__2_.pixil" -> "personagem" / "chao"
        base = re.sub(r'[\s_]*\(?\d+\)?[\s_]*$', '', pixil_path.stem).strip('_').strip()
        out_name = PIXIL_NAME_MAP.get(base, base)
        out_png  = sprites_dir / f"{out_name}.png"

        if convert_pixil_to_png(pixil_path, out_png):
            print(f"  {LVL} Sprite convertido: {pixil_path.name} -> Game/Assets/Sprites/{out_name}.png")
            moved += 1
            # Guardar o .pixil original em Source/ (permite reeditar mais tarde)
            source_dir.mkdir(parents=True, exist_ok=True)
            dest_pixil = source_dir / pixil_path.name
            if dest_pixil.exists():
                dest_pixil.unlink()
            shutil.move(str(pixil_path), str(dest_pixil))
        else:
            print(f"  {ERR} Falha ao converter {pixil_path.name} — ficheiro deixado na raiz para inspecao")

    # 3c. Optimizacao retroactiva — corre optipng sobre TODOS os sprites
    # existentes (nao so os convertidos agora). Util se optipng for
    # instalado DEPOIS de sprites ja terem sido convertidos sem ele —
    # basta correr reorganize.py de novo. Barato/idempotente: optipng
    # sobre um PNG ja optimo e' rapido e nao muda nada.
    if sprites_dir.exists():
        existing_pngs = list(sprites_dir.glob("*.png"))
        if existing_pngs:
            try:
                subprocess.run(["optipng", "-o7", "-quiet", *[str(p) for p in existing_pngs]],
                              capture_output=True, timeout=60)
                print(f"  {OK} {len(existing_pngs)} sprite(s) verificados/optimizados (optipng -o7)")
            except FileNotFoundError:
                pass  # ja avisado acima se algum sprite foi convertido agora; nao repetir aviso
            except Exception:
                pass

    # 4. Mapa estrutural
    try:
        import subprocess
        result = subprocess.run(
            ["python3", "-c",
             "import os; "
             "[print('./' + os.path.join(r,f).replace(os.sep,'/')) "
             "for r,d,files in os.walk('.') "
             "if '.git' not in r "
             "for f in sorted(files)]"],
            capture_output=True, text=True, cwd=ROOT
        )
        out = result.stdout
        (ROOT / "Development/project_structure.txt").write_text(out, encoding='utf-8')
        print(f"  {OK} Mapa estrutural atualizado em Development/project_structure.txt")
    except Exception:
        pass

    print()
    if moved == 0:
        print(f"  {OK} O projeto esta perfeitamente organizado!")
    else:
        print(f"  {OK} Concluido: {moved} ficheiro(s) processado(s).")
    print("\n  " + "=" * 55 + "\n")

if __name__ == "__main__":
    main()
