#!/usr/bin/env python3
"""
deps.py — Verificador e gestor de dependências
Vertical Precision Platformer

Verifica todas as ferramentas e bibliotecas necessárias.
NUNCA descarrega nem instala nada sem permissão explícita do utilizador.

Uso:
    python deps.py          — verifica todas as dependências
    python deps.py --quiet  — só imprime erros
"""

import subprocess
import sys
import os
import shutil
import argparse
from datetime import datetime
from pathlib import Path

# ── Configuração ──────────────────────────────────────────────────────────────

DEV_LOG = Path("Development/dev_log.txt")
IS_WINDOWS = sys.platform == "win32"

# ── Logging ───────────────────────────────────────────────────────────────────

def log_to_devlog(message: str, level: str = "INFO") -> None:
    """Regista no dev_log.txt (append-only, imutável)."""
    if not DEV_LOG.exists():
        return  # Não cria o ficheiro — esse trabalho é do setup inicial
    timestamp = datetime.now().strftime("[%Y-%m-%d %H:%M:%S]")
    with open(DEV_LOG, "a", encoding="utf-8") as f:
        f.write(f"{timestamp} [deps.py/{level}] {message}\n")

# ── Verificação de Ferramentas ────────────────────────────────────────────────

def check_tool(command: str, version_flag: str = "--version") -> tuple[bool, str]:
    """
    Verifica se uma ferramenta está disponível no PATH.
    Retorna (encontrado: bool, versão: str).
    """
    try:
        result = subprocess.run(
            [command, version_flag],
            capture_output=True, text=True, timeout=5
        )
        output = (result.stdout or result.stderr).strip()
        first_line = output.splitlines()[0] if output else ""
        return True, first_line
    except FileNotFoundError:
        return False, ""
    except subprocess.TimeoutExpired:
        return False, "(timeout)"

def check_vulkan_sdk() -> tuple[bool, str]:
    """Procura o Vulkan SDK em locais comuns e na variável de ambiente."""
    # 1. Variável de ambiente VULKAN_SDK (mais fiável)
    sdk_env = os.environ.get("VULKAN_SDK", "")
    if sdk_env and Path(sdk_env).exists():
        return True, f"VULKAN_SDK={sdk_env}"

    # 2. Localização padrão no Windows
    if IS_WINDOWS:
        sdk_root = Path("C:/VulkanSDK")
        if sdk_root.exists():
            versions = sorted(sdk_root.iterdir(), key=lambda p: p.name)
            if versions:
                return True, str(versions[-1])

    # 3. Linux: vulkaninfo no PATH
    if shutil.which("vulkaninfo"):
        found, version = check_tool("vulkaninfo", "--version")
        return True, version or "system install"

    # 4. Linux: pkg-config
    try:
        result = subprocess.run(
            ["pkg-config", "--modversion", "vulkan"],
            capture_output=True, text=True, timeout=5
        )
        if result.returncode == 0:
            return True, f"v{result.stdout.strip()}"
    except FileNotFoundError:
        pass

    return False, ""

# ── Definição de Dependências ─────────────────────────────────────────────────

TOOLS: list[dict] = [
    {
        "name":        "clang++",
        "command":     "clang++",
        "version_flag":"--version",
        "description": "Compilador C++ (LLVM/Clang)",
        "required":    True,
        "install_win": "winget install LLVM.LLVM  |  ou  https://releases.llvm.org/",
        "install_lin": "sudo apt install clang   |  ou  sudo pacman -S clang",
    },
    {
        "name":        "make",
        "command":     "make",
        "version_flag":"--version",
        "description": "GNU Make (sistema de build)",
        "required":    True,
        "install_win": "choco install make  |  ou incluído com Git Bash / MSYS2",
        "install_lin": "sudo apt install make",
    },
    {
        "name":        "git",
        "command":     "git",
        "version_flag":"--version",
        "description": "Controlo de versão",
        "required":    True,
        "install_win": "https://git-scm.com/download/win",
        "install_lin": "sudo apt install git",
    },
    {
        "name":        "python3",
        "command":     "python" if IS_WINDOWS else "python3",
        "version_flag":"--version",
        "description": "Python 3 (scripts de desenvolvimento)",
        "required":    True,
        "install_win": "https://python.org/  |  ou  winget install Python.Python.3",
        "install_lin": "sudo apt install python3",
    },
    {
        "name":        "llvm-ar",
        "command":     "llvm-ar",
        "version_flag":"--version",
        "description": "Archiver LLVM (para criar libgame.a com objetos Clang)",
        "required":    False,  # opcional: ar do sistema funciona na maioria dos casos
        "install_win": "incluído com LLVM — verificar se está no PATH",
        "install_lin": "sudo apt install llvm",
    },
]

# ── Pedido de Permissão ───────────────────────────────────────────────────────

def ask_permission(name: str, description: str, install_cmd: str) -> bool:
    """
    Pede confirmação explícita antes de qualquer instalação.
    Conforme Regra 2 do README: o script pede permissão por texto antes de qualquer download.
    """
    print(f"\n  ┌─ Instalação automática ─────────────────────────────────────")
    print(f"  │  Ferramenta  : {name}")
    print(f"  │  Descrição   : {description}")
    print(f"  │  Comando     : {install_cmd}")
    print(f"  └─────────────────────────────────────────────────────────────")
    try:
        answer = input("  Permitir execução do comando acima? [s/N] ").strip().lower()
    except (EOFError, KeyboardInterrupt):
        print()
        return False
    return answer in ("s", "sim", "y", "yes")

# ── Main ──────────────────────────────────────────────────────────────────────

def main() -> int:
    parser = argparse.ArgumentParser(description="Verificador de dependências.")
    parser.add_argument("--quiet", action="store_true", help="Só imprime erros")
    args = parser.parse_args()

    quiet = args.quiet

    if not quiet:
        print()
        print("  ╔══════════════════════════════════════════════════════════╗")
        print("  ║     Vertical Precision Platformer — deps.py             ║")
        print(f"  ║     Plataforma: {'Windows' if IS_WINDOWS else 'Linux':<42}║")
        print("  ╚══════════════════════════════════════════════════════════╝")
        print()

    results: list[tuple[str, bool, str, bool]] = []  # (nome, encontrado, versão, obrigatório)
    all_required_ok = True

    # ── Verificar ferramentas ──────────────────────────────────────────────
    for tool in TOOLS:
        found, version = check_tool(tool["command"], tool["version_flag"])
        results.append((tool["name"], found, version, tool["required"]))
        if not found and tool["required"]:
            all_required_ok = False

    # ── Verificar Vulkan SDK ───────────────────────────────────────────────
    vulkan_found, vulkan_info = check_vulkan_sdk()
    results.append(("Vulkan SDK", vulkan_found, vulkan_info, True))
    if not vulkan_found:
        all_required_ok = False

    # ── Imprimir resultados ────────────────────────────────────────────────
    if not quiet:
        print("  Dependências:")
        print("  ─────────────────────────────────────────────────────────")
        for name, found, version, required in results:
            icon = "✅" if found else ("❌" if required else "⚠️ ")
            label = "(opcional)" if not required else ""
            ver_str = f"  ← {version}" if found and version else ""
            print(f"  {icon}  {name:<16} {label}{ver_str}")

        print()

    # ── Instruções para dependências em falta ─────────────────────────────
    missing_required = [(n, f, v, r) for n, f, v, r in results if not f and r]
    missing_optional = [(n, f, v, r) for n, f, v, r in results if not f and not r]

    if missing_required:
        print("  ❌ Dependências obrigatórias em falta:")
        print()
        for name, _, _, _ in missing_required:
            tool_info = next((t for t in TOOLS if t["name"] == name), None)
            if tool_info:
                install = tool_info["install_win"] if IS_WINDOWS else tool_info["install_lin"]
                print(f"     {name}:")
                print(f"       → {install}")
            elif name == "Vulkan SDK":
                print(f"     Vulkan SDK:")
                print(f"       → https://vulkan.lunarg.com/sdk/home")
        print()

    if missing_optional and not quiet:
        print("  ⚠️  Dependências opcionais em falta (não bloqueiam o build):")
        for name, _, _, _ in missing_optional:
            tool_info = next((t for t in TOOLS if t["name"] == name), None)
            if tool_info:
                install = tool_info["install_win"] if IS_WINDOWS else tool_info["install_lin"]
                print(f"     {name}: {install}")
        print()

    # ── Resultado final ────────────────────────────────────────────────────
    if all_required_ok:
        if not quiet:
            print("  ✅ Todas as dependências obrigatórias satisfeitas.")
            print("     Podes correr:  make tests")
            print()
        log_to_devlog("Verificação de dependências concluída: OK")
        return 0
    else:
        print("  ❌ Resolve as dependências em falta antes de continuar.")
        print()
        log_to_devlog("Verificação de dependências: FALHOU — dependências obrigatórias em falta", "WARN")
        return 1


if __name__ == "__main__":
    sys.exit(main())
