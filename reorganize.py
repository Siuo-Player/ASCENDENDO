#!/usr/bin/env python3
"""
reorganize.py — Reorganiza os ficheiros da raiz para a estrutura correta do projeto.
Executar UMA VEZ na raiz do projeto (onde está o Makefile).
É idempotente: seguro correr múltiplas vezes sem dano.

Uso:
    python reorganize.py
"""

import shutil
from pathlib import Path

ROOT = Path(__file__).parent.resolve()

# ── Ficheiros a mover: (nome na raiz, destino relativo correto) ───────────────
# Atualizar esta lista sempre que novos ficheiros forem adicionados ao projeto.
MOVES = [
    # ── Infraestrutura (Fase 1) ───────────────────────────────────────────────
    ("doctest.h",               "external/doctest/doctest.h"),
    ("test_runner.cpp",         "Tests/test_runner.cpp"),
    ("pre-commit.sh",           "scripts/pre-commit.sh"),
    ("dev_log.txt",             "Development/dev_log.txt"),

    # ── Testes Unitarios ──────────────────────────────────────────────────────
    ("test_placeholder.cpp",    "Tests/Unit/test_placeholder.cpp"),
    ("test_vulkan_context.cpp", "Tests/Unit/test_vulkan_context.cpp"),

    # ── Testes de Integracao ──────────────────────────────────────────────────
    ("test_vulkan_init.cpp",    "Tests/Integration/test_vulkan_init.cpp"),

    # ── Game/Graphics (Fase 2+) ───────────────────────────────────────────────
    ("VulkanContext.h",         "Game/Graphics/VulkanContext.h"),
    ("VulkanContext.cpp",       "Game/Graphics/VulkanContext.cpp"),
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
SKIP = col("⏭ ", "33")
MOVE = col("📦", "36")
MISS = col("⚠️ ", "33")
ERR  = col("❌", "31")
DIR  = col("📁", "34")

def main() -> None:
    print()
    print(col("  Reorganizador de Estrutura — Vertical Precision Platformer", "1"))
    print("  " + "─" * 55)
    print()

    errors: list[str] = []

    # 1. Criar pastas e .gitkeep
    print(col("  [1/2] Criar pastas e ficheiros .gitkeep", "1"))
    for d in DIRS_WITH_GITKEEP:
        target = ROOT / d
        target.mkdir(parents=True, exist_ok=True)
        gitkeep = target / ".gitkeep"
        if not gitkeep.exists():
            gitkeep.touch()
            print(f"  {DIR} {d}/.gitkeep  (criado)")
        else:
            print(f"  {OK} {d}/  (já existe)")
    print()

    # 2. Mover ficheiros
    print(col("  [2/2] Mover ficheiros para os destinos corretos", "1"))
    for src_name, dst_rel in MOVES:
        src = ROOT / src_name
        dst = ROOT / dst_rel

        dst.parent.mkdir(parents=True, exist_ok=True)

        if dst.exists() and not src.exists():
            # Ficheiro já está no sítio certo
            print(f"  {OK} {dst_rel}  (já no lugar)")

        elif src.exists() and not dst.exists():
            # Mover da raiz para o destino
            shutil.move(str(src), str(dst))
            print(f"  {MOVE} {src_name}  →  {dst_rel}")

        elif src.exists() and dst.exists():
            # Conflito: existe nos dois lados — manter o destino, alertar
            print(f"  {MISS} CONFLITO: '{src_name}' existe na raiz E em '{dst_rel}'.")
            print(f"       A manter '{dst_rel}'. Remove '{src_name}' da raiz manualmente se for duplicado.")

        else:
            # Ficheiro não encontrado em lado nenhum
            msg = f"'{src_name}' não encontrado (nem na raiz nem em '{dst_rel}')"
            print(f"  {ERR} {msg}")
            errors.append(msg)

    print()
    print("  " + "─" * 55)

    if errors:
        print(f"  {ERR} Concluído com {len(errors)} aviso(s):")
        for e in errors:
            print(f"       • {e}")
        print()
        print("  Cria os ficheiros em falta a partir dos que descarregaste.")
    else:
        print(f"  {OK} Tudo reorganizado sem erros!")

    print()
    print(col("  Estrutura resultante esperada:", "1"))
    print("""
  Ascendendo/
  ├── .gitignore
  ├── Makefile
  ├── deps.py
  ├── README.md
  ├── reorganize.py
  ├── Development/
  │   ├── dev_log.txt
  │   ├── LevelEditor/
  │   └── AI_Validation/
  ├── Game/
  │   ├── Graphics/
  │   │   ├── VulkanContext.h
  │   │   └── VulkanContext.cpp
  │   ├── Assets/
  │   └── Logic/
  ├── Tests/
  │   ├── test_runner.cpp
  │   ├── Unit/
  │   │   ├── test_placeholder.cpp
  │   │   └── test_vulkan_context.cpp
  │   ├── Integration/
  │   │   └── test_vulkan_init.cpp
  │   ├── System/
  │   ├── Regression/
  │   └── Acceptance/
  ├── external/
  │   └── doctest/
  │       └── doctest.h
  └── scripts/
      └── pre-commit.sh
""")
    print("  Próximo passo: instala clang++ e make (ver instruções).")
    print("  Depois abre um Git Bash e corre:  make tests")
    print()


if __name__ == "__main__":
    main()
