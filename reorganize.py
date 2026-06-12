#!/usr/bin/env python3
"""
reorganize.py — Motor de Reorganização do ASCENDENDO
Garante que a arquitetura do projeto se mantém imaculada, executa a validação
automática de física dos níveis e move os reprovados para a pasta 'NaoValidados'.
"""

import shutil
import math
from pathlib import Path

ROOT = Path(__file__).parent.resolve()

MOVES = [
    # Infraestrutura e Configurações
    ("c_cpp_properties.json",   ".vscode/c_cpp_properties.json"),
    ("doctest.h",               "external/doctest/doctest.h"),
    ("test_runner.cpp",         "Tests/test_runner.cpp"),
    ("pre-commit.sh",           "scripts/pre-commit.sh"),
    ("pre-push.sh",             ".git/hooks/pre-push"),
    ("dev_log.txt",             "Development/dev_log.txt"),
    ("ai_validator.py",         "Development/AI_Validation/ai_validator.py"),
    ("Config.h",                "Game/Core/Config.h"),

    # Testes Unitários
    ("test_placeholder.cpp",    "Tests/Unit/test_placeholder.cpp"),
    ("test_vulkan_context.cpp", "Tests/Unit/test_vulkan_context.cpp"),
    ("test_input.cpp",          "Tests/Unit/test_input.cpp"),
    ("test_physics.cpp",        "Tests/Unit/test_physics.cpp"),
    ("test_player.cpp",         "Tests/Unit/test_player.cpp"),
    ("test_replay.cpp",         "Tests/Unit/test_replay.cpp"),
    ("test_camera.cpp",         "Tests/Unit/test_camera.cpp"),
    ("test_level.cpp",          "Tests/Unit/test_level.cpp"),

    # Testes de Integração
    ("test_vulkan_init.cpp",    "Tests/Integration/test_vulkan_init.cpp"),
    ("test_window.cpp",         "Tests/Integration/test_window.cpp"),
    ("test_swapchain.cpp",      "Tests/Integration/test_swapchain.cpp"),
    ("test_render_pass.cpp",    "Tests/Integration/test_render_pass.cpp"),
    ("test_renderer.cpp",       "Tests/Integration/test_renderer.cpp"),
    ("test_pipeline.cpp",       "Tests/Integration/test_pipeline.cpp"),

    # Game / Graphics
    ("VulkanContext.h",         "Game/Graphics/VulkanContext.h"),
    ("VulkanContext.cpp",       "Game/Graphics/VulkanContext.cpp"),
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

    # Shaders
    ("base.vert",               "Game/Assets/Shaders/base.vert"),
    ("base.frag",               "Game/Assets/Shaders/base.frag"),
]

DIRS_WITH_GITKEEP = [
    ".vscode", "Game/Core", "Game/Graphics", "Game/Assets/Shaders", "Game/Logic",
    "Game/Assets/Levels/Unused",
    "Game/Assets/Levels/NaoValidados", # Pasta para os níveis inválidos
    "Development/LevelEditor", "Development/AI_Validation",
    "Tests/Unit", "Tests/Integration", "Tests/System", "Tests/Regression", "Tests/Acceptance",
    "external/doctest", "scripts",
]

def col(text: str, code: str) -> str: return f"\033[{code}m{text}\033[0m"
OK, MOVE, WARN, DIR = col("✅", "32"), col("📦", "36"), col("⚠️ ", "33"), col("📁", "34")

def check_level_validity(filepath) -> bool:
    """Algoritmo BFS com margem de erro de 15% para validação física real do nível."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except:
        return False

    platforms = []
    goal_node = {'type': 'top', 'bounds': (0, 640.0, 360.0, 0)}

    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'): continue
        parts = line.split()
        if parts[0] == "PLATFORM":
            if len(parts) != 5: return False
            x, y, w, h = map(float, parts[1:])
            if x < 0.0 or (x + w) > 360.0: return False
            platforms.append({'type': 'platform', 'bounds': (x, y, w, h)})
        elif parts[0] == "FLAG":
            if len(parts) != 5: return False
            x, y, w, h = map(float, parts[1:])
            goal_node = {'type': 'flag', 'bounds': (x, y, w, h)}

    nodes = [{'type': 'ground', 'bounds': (0, 0, 360.0, 0)}] + platforms + [goal_node]
    visited = {0}
    queue = [0]
    
    while queue:
        curr = queue.pop(0)
        if curr == len(nodes) - 1:
            return True
            
        for i in range(1, len(nodes)):
            if i not in visited:
                n1 = nodes[curr]
                n2 = nodes[i]
                p1 = n1['bounds']
                y_start = p1[1] + p1[3]
                
                t2 = n2['type']
                if t2 == 'top':
                    y_end = 640.0
                    dx = 0.0
                elif t2 == 'flag':
                    p2 = n2['bounds']
                    y_end = p2[1]
                    if p1[0] + p1[2] < p2[0]: dx = p2[0] - (p1[0] + p1[2])
                    elif p1[0] > p2[0] + p2[2]: dx = p1[0] - (p2[0] + p2[2])
                    else: dx = 0.0
                else:
                    p2 = n2['bounds']
                    y_end = p2[1] + p2[3]
                    if p1[0] + p1[2] < p2[0]: dx = p2[0] - (p1[0] + p1[2])
                    elif p1[0] > p2[0] + p2[2]: dx = p1[0] - (p2[0] + p2[2])
                    else: dx = 0.0
                    
                dy = y_end - y_start
                
                G_val = 980.0
                V_MAX_val = 800.0
                ANGLE_val = math.pi / 3.0
                TOLERANCE_val = 0.85
                
                VY_val = V_MAX_val * math.sin(ANGLE_val) * TOLERANCE_val
                VX_val = V_MAX_val * math.cos(ANGLE_val) * TOLERANCE_val
                MAX_JUMP_Y_val = (VY_val**2) / (2 * G_val)
                
                if dy <= MAX_JUMP_Y_val:
                    if dy > 0:
                        disc = VY_val**2 - 2 * G_val * dy
                        if disc >= 0:
                            t_max = (VY_val + math.sqrt(disc)) / G_val
                            max_dx = VX_val * t_max
                            if dx <= max_dx:
                                visited.add(i)
                                queue.append(i)
                    else:
                        t_max = (VY_val + math.sqrt(VY_val**2 - 2 * G_val * dy)) / G_val
                        max_dx = VX_val * t_max
                        if dx <= max_dx:
                            visited.add(i)
                            queue.append(i)
    return False

def remove_from_campaign(filename: str) -> None:
    """Remove automaticamente entradas de níveis inválidos da playlist campaign.txt."""
    campaign_paths = [ROOT / "campaign.txt", ROOT / "Game/Assets/Levels/campaign.txt"]
    for cp in campaign_paths:
        if cp.exists():
            with open(cp, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            new_lines = []
            removed = False
            for line in lines:
                if filename in line:
                    removed = True
                    continue
                new_lines.append(line)
            if removed:
                with open(cp, 'w', encoding='utf-8') as f:
                    f.writelines(new_lines)
                print(f"  {WARN} {filename} removido automaticamente da playlist (campaign.txt)!")

def generate_structure_map(dir_path: Path, prefix="", ignore_dirs={'.git', 'build', '__pycache__'}):
    lines = []
    paths = sorted([p for p in dir_path.iterdir() if p.name not in ignore_dirs and not p.name.startswith('.')])
    for i, p in enumerate(paths):
        is_last = (i == len(paths) - 1)
        connector = "└── " if is_last else "├── "
        lines.append(f"{prefix}{connector}{p.name}{'/' if p.is_dir() else ''}")
        if p.is_dir():
            extension = "    " if is_last else "│   "
            lines.extend(generate_structure_map(p, prefix + extension, ignore_dirs))
    return lines

def main() -> None:
    print(f"\n{col('  Motor de Reorganização — ASCENDENDO', '1;36')}")
    print("  " + "═" * 55 + "\n")
    moved_count = 0

    if (ROOT / "campaign.txt").exists():
        MOVES.append(("campaign.txt", "Game/Assets/Levels/campaign.txt"))

    for d in DIRS_WITH_GITKEEP:
        target = ROOT / d
        target.mkdir(parents=True, exist_ok=True)
        gitkeep = target / ".gitkeep"
        if not gitkeep.exists():
            gitkeep.touch(); print(f"  {DIR} {d}/.gitkeep (Criado)")

    # Executa as movimentações padrão primeiro
    for src_name, dst_rel in MOVES:
        src = ROOT / src_name; dst = ROOT / dst_rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        if src.exists():
            if dst.exists() and src.resolve() != dst.resolve():
                shutil.copy2(str(src), str(dst)); src.unlink()
                print(f"  {MOVE} Atualizado: {dst_rel}"); moved_count += 1
            elif not dst.exists():
                shutil.move(str(src), str(dst))
                print(f"  {MOVE} Movido:     {dst_rel}"); moved_count += 1

    # Sistema Automático de Filtragem e Validação Física de Níveis
    levels_to_process = []
    for f in ROOT.glob("*.lvl"):
        levels_to_process.append((f, True))
    
    levels_dir = ROOT / "Game/Assets/Levels"
    if levels_dir.exists():
        for f in levels_dir.glob("*.lvl"):
            levels_to_process.append((f, False))

    for filepath, is_from_root in levels_to_process:
        if not filepath.exists(): continue
        
        filename = filepath.name
        is_valid = check_level_validity(filepath)
        
        if is_valid:
            target_path = ROOT / "Game/Assets/Levels" / filename
            if is_from_root:
                shutil.move(str(filepath), str(target_path))
                print(f"  {OK} Nível VÁLIDO movido para: Game/Assets/Levels/{filename}")
                moved_count += 1
        else:
            target_path = ROOT / "Game/Assets/Levels/NaoValidados" / filename
            shutil.move(str(filepath), str(target_path))
            print(f"  {WARN} Nível INVÁLIDO '{filename}' detetado! Movido para NaoValidados/")
            remove_from_campaign(filename)
            moved_count += 1

    dev_dir = ROOT / "Development"
    dev_dir.mkdir(parents=True, exist_ok=True)
    map_file = dev_dir / "project_structure.txt"
    tree_lines = generate_structure_map(ROOT)
    with open(map_file, "w", encoding="utf-8") as f:
        f.write("ASCENDENDO/\n")
        f.write("\n".join(tree_lines))
        f.write("\n")
    print(f"  {OK} Mapa estrutural atualizado em Development/project_structure.txt")

    hook_path = ROOT / ".git/hooks/pre-push"
    if hook_path.exists():
        hook_path.chmod(0o755)

    if moved_count == 0: print(f"  {OK} O projeto está perfeitamente organizado!")
    else: print(f"\n  {OK} Concluído: {moved_count} ficheiro(s) processado(s).")
    print("\n  " + "═" * 55 + "\n")

if __name__ == "__main__": main()