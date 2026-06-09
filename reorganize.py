#!/usr/bin/env python3
"""
reorganize.py — Motor de Reorganização do ASCENDENDO
Garante que a arquitetura do projeto se mantém imaculada.
Executar na raiz sempre que novos ficheiros forem descarregados ou criados.
"""

import shutil
from pathlib import Path

ROOT = Path(__file__).parent.resolve()

MOVES = [
    # Configurações de Editor & Infra
    ("c_cpp_properties.json",   ".vscode/c_cpp_properties.json"),
    ("doctest.h",               "external/doctest/doctest.h"),
    ("test_runner.cpp",         "Tests/test_runner.cpp"),
    ("pre-commit.sh",           "scripts/pre-commit.sh"),
    ("dev_log.txt",             "Development/dev_log.txt"),

    # Testes Unitários
    ("test_placeholder.cpp",    "Tests/Unit/test_placeholder.cpp"),
    ("test_vulkan_context.cpp", "Tests/Unit/test_vulkan_context.cpp"),
    ("test_input.cpp",          "Tests/Unit/test_input.cpp"),            # Fase 3.1
    ("test_physics.cpp",        "Tests/Unit/test_physics.cpp"),          # Fase 3.1
    ("test_player.cpp",         "Tests/Unit/test_player.cpp"),           # Fase 3.2
    ("test_replay.cpp",         "Tests/Unit/test_replay.cpp"),           # Fase 3.3
    ("test_camera.cpp",         "Tests/Unit/test_camera.cpp"),           # Fase 4.1

    # Testes de Integração
    ("test_vulkan_init.cpp",    "Tests/Integration/test_vulkan_init.cpp"),
    ("test_window.cpp",         "Tests/Integration/test_window.cpp"),
    ("test_swapchain.cpp",      "Tests/Integration/test_swapchain.cpp"),
    ("test_render_pass.cpp",    "Tests/Integration/test_render_pass.cpp"),
    ("test_renderer.cpp",       "Tests/Integration/test_renderer.cpp"),

    # Game/Graphics (O Motor)
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

    # Game/Logic (Física e Input)
    ("InputManager.h",          "Game/Logic/InputManager.h"),
    ("InputManager.cpp",        "Game/Logic/InputManager.cpp"),
    ("Physics.h",               "Game/Logic/Physics.h"),
    ("Physics.cpp",             "Game/Logic/Physics.cpp"),
    ("Player.h",                "Game/Logic/Player.h"),
    ("Player.cpp",              "Game/Logic/Player.cpp"),
    ("ReplayManager.h",         "Game/Logic/ReplayManager.h"),
    ("ReplayManager.cpp",       "Game/Logic/ReplayManager.cpp"),

    # ── Fase 5: Shaders e Pipeline ────────────────────────────────────────────
    ("base.vert",               "Game/Assets/Shaders/base.vert"),
    ("base.frag",               "Game/Assets/Shaders/base.frag"),
    ("Pipeline.h",              "Game/Graphics/Pipeline.h"),
    ("Pipeline.cpp",            "Game/Graphics/Pipeline.cpp"),
    ("test_pipeline.cpp",       "Tests/Integration/test_pipeline.cpp"),
]

DIRS_WITH_GITKEEP = [
    ".vscode", "Game/Graphics", "Game/Assets/Shaders", "Game/Logic",
    "Development/LevelEditor", "Development/AI_Validation",
    "Tests/Unit", "Tests/Integration", "Tests/System", "Tests/Regression", "Tests/Acceptance",
    "external/doctest", "scripts",
]

def col(text: str, code: str) -> str: return f"\033[{code}m{text}\033[0m"
OK, MOVE, WARN, DIR = col("✅", "32"), col("📦", "36"), col("⚠️ ", "33"), col("📁", "34")

def main() -> None:
    print(f"\n{col('  Motor de Reorganização — ASCENDENDO', '1;36')}")
    print("  " + "═" * 55 + "\n")
    errors = []
    moved_count = 0

    for d in DIRS_WITH_GITKEEP:
        target = ROOT / d
        target.mkdir(parents=True, exist_ok=True)
        gitkeep = target / ".gitkeep"
        if not gitkeep.exists():
            gitkeep.touch()
            print(f"  {DIR} {d}/.gitkeep (Criado)")

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
        elif not dst.exists(): errors.append(dst_rel)

    if moved_count == 0: print(f"  {OK} O projeto está perfeitamente organizado!")
    else: print(f"\n  {OK} Concluído: {moved_count} ficheiro(s) processado(s).")
    if errors:
        print(f"\n  {WARN} {len(errors)} ficheiro(s) em falta (Ignora se for de Fases futuras):")
        for e in errors: print(f"       • {e}")
    print("\n  " + "═" * 55 + "\n")

if __name__ == "__main__": main()