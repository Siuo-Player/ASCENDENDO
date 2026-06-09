#!/usr/bin/env python3
"""
reorganize.py — Motor de Reorganização do ASCENDENDO
Garante que a arquitetura do projeto se mantém imaculada.
Executar na raiz sempre que novos ficheiros forem descarregados ou criados.
"""

import shutil
from pathlib import Path

ROOT = Path(__file__).parent.resolve()

# ── Mapeamento de Ficheiros (Origem na Raiz -> Destino Final) ───────────────
MOVES = [
    # Configurações de Editor
    ("c_cpp_properties.json",   ".vscode/c_cpp_properties.json"),

    # Infraestrutura
    ("doctest.h",               "external/doctest/doctest.h"),
    ("test_runner.cpp",         "Tests/test_runner.cpp"),
    ("pre-commit.sh",           "scripts/pre-commit.sh"),
    ("dev_log.txt",             "Development/dev_log.txt"),

    # Testes Unitários
    ("test_placeholder.cpp",    "Tests/Unit/test_placeholder.cpp"),
    ("test_vulkan_context.cpp", "Tests/Unit/test_vulkan_context.cpp"),

    # Testes de Integração
    ("test_vulkan_init.cpp",    "Tests/Integration/test_vulkan_init.cpp"),
    ("test_window.cpp",         "Tests/Integration/test_window.cpp"),
    ("test_swapchain.cpp",      "Tests/Integration/test_swapchain.cpp"), # Fase 2.4
    ("test_render_pass.cpp",    "Tests/Integration/test_render_pass.cpp"), # Fase 2.5

    # Game/Graphics (O Motor)
    ("VulkanContext.h",         "Game/Graphics/VulkanContext.h"),
    ("VulkanContext.cpp",       "Game/Graphics/VulkanContext.cpp"),
    ("Window.h",                "Game/Graphics/Window.h"),
    ("Window.cpp",              "Game/Graphics/Window.cpp"),
    ("Swapchain.h",             "Game/Graphics/Swapchain.h"),            # Fase 2.4
    ("Swapchain.cpp",           "Game/Graphics/Swapchain.cpp"),          # Fase 2.4
    ("RenderPass.h",            "Game/Graphics/RenderPass.h"),           # Fase 2.5
    ("RenderPass.cpp",          "Game/Graphics/RenderPass.cpp"),         # Fase 2.5
    ("Renderer.h",              "Game/Graphics/Renderer.h"),             # Fase 2.6
    ("Renderer.cpp",            "Game/Graphics/Renderer.cpp"),           # Fase 2.6

    # ── Testes de Integracao (continuacao) ────────────────────────────────────
    ("test_renderer.cpp",       "Tests/Integration/test_renderer.cpp"),  # Fase 2.6

    # ── Game/Logic (Fase 3+) ──────────────────────────────────────────────────
    ("InputManager.h",          "Game/Logic/InputManager.h"),            # Fase 3.1
    ("InputManager.cpp",        "Game/Logic/InputManager.cpp"),          # Fase 3.1
    ("Physics.h",               "Game/Logic/Physics.h"),                 # Fase 3.1
    ("Physics.cpp",             "Game/Logic/Physics.cpp"),               # Fase 3.1

    # ── Testes Unit (continuacao) ─────────────────────────────────────────────
    ("test_input.cpp",          "Tests/Unit/test_input.cpp"),            # Fase 3.1
    ("test_physics.cpp",        "Tests/Unit/test_physics.cpp"),          # Fase 3.1
    ("Player.h",                "Game/Logic/Player.h"),                  # Fase 3.2
    ("Player.cpp",              "Game/Logic/Player.cpp"),                # Fase 3.2
    ("test_player.cpp",         "Tests/Unit/test_player.cpp"),           # Fase 3.2
    
    # ── Replay e Save States (Fase 3.3) ───────────────────────────────────────
    ("ReplayManager.h",         "Game/Logic/ReplayManager.h"),           # Fase 3.3
    ("ReplayManager.cpp",       "Game/Logic/ReplayManager.cpp"),         # Fase 3.3
    ("test_replay.cpp",         "Tests/Unit/test_replay.cpp"),           # Fase 3.3

    # ── Camera e Viewport (Fase 4.1) ──────────────────────────────────────────
    ("Camera.h",                "Game/Graphics/Camera.h"),               # Fase 4.1
    ("Camera.cpp",              "Game/Graphics/Camera.cpp"),             # Fase 4.1
    ("test_camera.cpp",         "Tests/Unit/test_camera.cpp"),           # Fase 4.1
]

# ── Estrutura de Diretórios Obrigatória (Com proteção .gitkeep) ──────────────
DIRS_WITH_GITKEEP = [
    ".vscode",
    "Game/Graphics",
    "Game/Assets",
    "Game/Logic",
    "Development/LevelEditor",
    "Development/AI_Validation",
    "Tests/Unit",
    "Tests/Integration",
    "Tests/System",
    "Tests/Regression",
    "Tests/Acceptance",
    "external/doctest",
    "scripts",
]

# ── Utilitários de Terminal ──────────────────────────────────────────────────
def col(text: str, code: str) -> str:
    """Aplica cores ANSI ao terminal."""
    return f"\033[{code}m{text}\033[0m"

OK   = col("✅", "32")
MOVE = col("📦", "36")
WARN = col("⚠️ ", "33")
DIR  = col("📁", "34")

def main() -> None:
    print(f"\n{col('  Motor de Reorganização — ASCENDENDO', '1;36')}")
    print("  " + "═" * 55 + "\n")

    errors = []
    moved_count = 0

    # 1. Validação de Estrutura de Pastas (Modo Silencioso)
    for d in DIRS_WITH_GITKEEP:
        target = ROOT / d
        target.mkdir(parents=True, exist_ok=True)
        gitkeep = target / ".gitkeep"
        if not gitkeep.exists():
            gitkeep.touch()
            print(f"  {DIR} {d}/.gitkeep (Criado)")

    # 2. Processamento de Ficheiros
    for src_name, dst_rel in MOVES:
        src = ROOT / src_name
        dst = ROOT / dst_rel

        dst.parent.mkdir(parents=True, exist_ok=True)

        if src.exists():
            # Substituir ficheiro já existente
            if dst.exists() and src.resolve() != dst.resolve():
                shutil.copy2(str(src), str(dst))
                src.unlink()
                print(f"  {MOVE} Atualizado: {dst_rel} (Substituído a partir da raiz)")
                moved_count += 1
            # Mover ficheiro novo
            elif not dst.exists():
                shutil.move(str(src), str(dst))
                print(f"  {MOVE} Movido:     {dst_rel}")
                moved_count += 1
        elif not dst.exists():
            errors.append(dst_rel)

    # 3. Resumo Final
    if moved_count == 0:
        print(f"  {OK} O projeto está perfeitamente organizado!")
    else:
        print(f"\n  {OK} Concluído: {moved_count} ficheiro(s) processado(s).")
        
    if errors:
        print(f"\n  {WARN} {len(errors)} ficheiro(s) em falta:")
        for e in errors:
            print(f"       • {e}")
        print(col("  (Nota: Podes ignorar os ficheiros de Fases que ainda não começaste).", "90"))
    
    print("\n  " + "═" * 55 + "\n")

if __name__ == "__main__":
    main()