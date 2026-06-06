#!/usr/bin/env python3
"""
reorganize.py — Reorganiza os ficheiros da raiz para a estrutura correta do projeto.
Executar UMA VEZ na raiz do projeto sempre que novos ficheiros forem descarregados.
"""

import shutil
from pathlib import Path

ROOT = Path(__file__).parent.resolve()

# ── Ficheiros a mover: (nome na raiz, destino relativo correto) ───────────────
# O script sabe exatamente onde cada ficheiro do motor deve morar.
MOVES = [
    # Infraestrutura
    ("doctest.h",               "external/doctest/doctest.h"),
    ("test_runner.cpp",         "Tests/test_runner.cpp"),
    ("pre-commit.sh",           "scripts/pre-commit.sh"),
    ("dev_log.txt",             "Development/dev_log.txt"),

    # Testes Unitarios
    ("test_placeholder.cpp",    "Tests/Unit/test_placeholder.cpp"),
    ("test_vulkan_context.cpp", "Tests/Unit/test_vulkan_context.cpp"),

    # Testes de Integracao
    ("test_vulkan_init.cpp",    "Tests/Integration/test_vulkan_init.cpp"),
    ("test_window.cpp",         "Tests/Integration/test_window.cpp"),
    ("test_swapchain.cpp",      "Tests/Integration/test_swapchain.cpp"), # Fase 2.4

    # Game/Graphics
    ("VulkanContext.h",         "Game/Graphics/VulkanContext.h"),
    ("VulkanContext.cpp",       "Game/Graphics/VulkanContext.cpp"),
    ("Window.h",                "Game/Graphics/Window.h"),
    ("Window.cpp",              "Game/Graphics/Window.cpp"),
    ("Swapchain.h",             "Game/Graphics/Swapchain.h"),            # Fase 2.4
    ("Swapchain.cpp",           "Game/Graphics/Swapchain.cpp"),          # Fase 2.4
]

# ── Pastas que devem existir (com .gitkeep para o git as rastrear) ────────────
DIRS_WITH_GITKEEP = [
    "Game/Graphics",
    "Game/Assets",
    "Game/Logic",
    "Development/LevelEditor",
    "Development/AI_Validation",
    "Tests/Integration",
    "Tests/System",
    "Tests/Regression",
    "Tests/Acceptance",
    "external/doctest",
    "scripts",
]

# ─────────────────────────────────────────────────────────────────────────────

def col(text: str, code: str) -> str:
    """ANSI color — funciona no Windows Terminal, Git Bash e Linux."""
    return f"\033[{code}m{text}\033[0m"

OK   = col("✅", "32")
MOVE = col("📦", "36")
ERR  = col("❌", "31")
DIR  = col("📁", "34")

def main() -> None:
    print()
    print(col("  Reorganizador de Estrutura — Vertical Precision Platformer", "1"))
    print("  " + "─" * 55)
    print()

    errors: list[str] = []

    # 1. Garantir Pastas e .gitkeep
    print(col("  [1/2] Verificar pastas e ficheiros .gitkeep", "1"))
    for d in DIRS_WITH_GITKEEP:
        target = ROOT / d
        target.mkdir(parents=True, exist_ok=True)
        gitkeep = target / ".gitkeep"
        if not gitkeep.exists():
            gitkeep.touch()
            print(f"  {DIR} {d}/.gitkeep  (criado)")
        else:
            print(f"  {OK} {d}/  (verificado)")
    print()

    # 2. Mover e Substituir Ficheiros
    print(col("  [2/2] Processar ficheiros do projeto", "1"))
    for src_name, dst_rel in MOVES:
        src = ROOT / src_name
        dst = ROOT / dst_rel

        dst.parent.mkdir(parents=True, exist_ok=True)

        if dst.exists() and not src.exists():
            print(f"  {OK} {dst_rel}  (já no lugar)")

        elif src.exists() and not dst.exists():
            shutil.move(str(src), str(dst))
            print(f"  {MOVE} {src_name}  →  {dst_rel}")

        elif src.exists() and dst.exists():
            if src.resolve() != dst.resolve():
                shutil.copy2(str(src), str(dst))
                src.unlink()
                print(f"  {MOVE} {src_name}  →  {dst_rel} (SUBSTITUÍDO)")

        else:
            msg = f"'{src_name}' não encontrado (nem na raiz nem em '{dst_rel}')"
            print(f"  {ERR} {msg}")
            errors.append(msg)

    print()
    print("  " + "─" * 55)

    if errors:
        print(f"  {ERR} Concluído com {len(errors)} ficheiro(s) em falta:")
        for e in errors:
            print(f"       • {e}")
        print(col("\n  -> Cria estes ficheiros na raiz ou descarrega-os antes de compilar.", "33"))
    else:
        print(f"  {OK} Tudo reorganizado sem erros! Todos os ficheiros mapeados existem.")
    print()

if __name__ == "__main__":
    main()