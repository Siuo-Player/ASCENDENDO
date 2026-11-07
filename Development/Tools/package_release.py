#!/usr/bin/env python3
"""ASCENDENDO 1.0 Release Packaging Tool.

Packages the Windows x64 binary, assets, compiled shaders, licenses,
and documentation into a standalone release archive (ZIP) with SHA-256 checksums.
"""
from __future__ import annotations

import argparse
import hashlib
import os
import shutil
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


QUICKSTART_GUIDE = """================================================================================
                                ASCENDENDO 1.0
                    Vertical Precision Platformer (Windows x64)
================================================================================

Obrigado por descarregares o ASCENDENDO!
O jogo e 100% gratuito, open source e offline-first.
Nenhum dado pessoal (PII) e recolhido ou transmitido.

--------------------------------------------------------------------------------
COMO EXECUTAR
--------------------------------------------------------------------------------
1. Extrai o conteudo deste ficheiro ZIP para uma pasta a tua escolha.
2. Executa o ficheiro: game.exe
   (Requisitos: Windows 10/11 x64, placa grafica compativel com Vulkan 1.2+).

--------------------------------------------------------------------------------
CONTROLOS DO JOGO
--------------------------------------------------------------------------------
* Movimento Horizontal:      A / D  ou  Setas Esquerda / Direita
* Carregar / Executar Salto: ESPACO (Mantem pressionado para dar forca, solta para saltar)
* Alternar Ecra Inteiro:     F11 (Preserva o aspecto 16:9 sem distorcao)
* Pausa / Menu:              ESC
* Editor de Niveis:          E (no menu ou durante o jogo)
* Editor de Campanhas:       C (no menu principal)

--------------------------------------------------------------------------------
CONTAS LOCAIS E SEGURANCA
--------------------------------------------------------------------------------
O ASCENDENDO utiliza um cofre criptografico local (HMAC-SHA256 com PBKDF2)
localizado na pasta 'userdata/Accounts/'.
- Zero telemetria, zero servidores na nuvem, privacidade absoluta.
- Podes criar multiplos utilizadores na mesma maquina.
- O progresso das campanhas e tempos recorde ficam guardados de forma
  isolada e autenticada no cofre do teu utilizador.

--------------------------------------------------------------------------------
DOCUMENTACAO E CODIGO FONTE
--------------------------------------------------------------------------------
Codigo-fonte oficial, issues e atualizacoes:
https://github.com/Siuo-Player/ASCENDENDO

Bom jogo e boa ascensao ao Cume!
"""


def compute_sha256(path: Path) -> str:
    hasher = hashlib.sha256()
    with open(path, "rb") as stream:
        while chunk := stream.read(65536):
            hasher.update(chunk)
    return hasher.hexdigest()


def package_release(output_dir: Path, version: str = "1.0.0") -> tuple[Path, str]:
    output_dir.mkdir(parents=True, exist_ok=True)
    staging_dir = output_dir / f"ascendendo-v{version}-staging"
    if staging_dir.exists():
        shutil.rmtree(staging_dir)
    staging_dir.mkdir(parents=True)

    package_root = staging_dir / "ascendendo"
    package_root.mkdir(parents=True)

    # 1. Executable
    game_exe = ROOT / "build" / "game" / "game.exe"
    if not game_exe.is_file():
        # Check alternative build locations
        alt_exe = ROOT / "game.exe"
        if alt_exe.is_file():
            game_exe = alt_exe
        else:
            raise FileNotFoundError(
                f"Binary not found at {game_exe}. Run 'make game' before packaging."
            )
    shutil.copy2(game_exe, package_root / "game.exe")
    print(f"[OK] Copied {game_exe.name} ({game_exe.stat().st_size / (1024*1024):.1f} MB)")

    # 2. Assets
    assets_src = ROOT / "Game" / "Assets"
    if not assets_src.is_dir():
        raise FileNotFoundError(f"Assets directory not found at {assets_src}")
    shutil.copytree(assets_src, package_root / "Game" / "Assets")
    print("[OK] Copied Game/Assets/ (levels, campaigns, sprites, fonts)")

    # 3. Shaders
    shaders_src = ROOT / "Game" / "Shaders"
    if shaders_src.is_dir():
        shutil.copytree(shaders_src, package_root / "Game" / "Shaders")
        print("[OK] Copied Game/Shaders/")

    # 4. Licenses & Documentation
    license_file = ROOT / "LICENSE"
    if license_file.is_file():
        shutil.copy2(license_file, package_root / "LICENSE")
    notices_file = ROOT / "docs" / "THIRD_PARTY_NOTICES.md"
    if notices_file.is_file():
        shutil.copy2(notices_file, package_root / "THIRD_PARTY_NOTICES.md")
    readme_file = ROOT / "README.md"
    if readme_file.is_file():
        shutil.copy2(readme_file, package_root / "README.md")

    # 5. Quickstart guide
    (package_root / "QUICKSTART.txt").write_text(QUICKSTART_GUIDE, encoding="utf-8")

    # 6. Create Zip archive
    zip_filename = f"ascendendo-v{version}-windows-x64.zip"
    zip_path = output_dir / zip_filename
    if zip_path.exists():
        zip_path.unlink()

    print(f"Creating release archive: {zip_path.name}...")
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as zipf:
        for file in package_root.rglob("*"):
            if file.is_file():
                arcname = file.relative_to(staging_dir)
                zipf.write(file, arcname)

    # Also copy as standard release name
    generic_zip = output_dir / "ascendendo-windows-x64.zip"
    shutil.copy2(zip_path, generic_zip)

    # Checksum
    sha256_hash = compute_sha256(zip_path)
    (output_dir / f"{zip_filename}.sha256").write_text(f"{sha256_hash}  {zip_filename}\n", encoding="utf-8")
    (output_dir / "ascendendo-windows-x64.zip.sha256").write_text(f"{sha256_hash}  ascendendo-windows-x64.zip\n", encoding="utf-8")

    # Clean up staging
    shutil.rmtree(staging_dir, ignore_errors=True)

    print(f"\n[SUCCESS] Packaged {zip_path.name}")
    print(f"Archive size: {zip_path.stat().st_size / (1024*1024):.2f} MB")
    print(f"SHA-256: {sha256_hash}")

    return zip_path, sha256_hash


def main() -> int:
    parser = argparse.ArgumentParser(description="Package Ascendendo Windows Release Archive")
    parser.add_argument("--output-dir", default=str(ROOT / "dist"), help="Target directory for release artifacts")
    parser.add_argument("--version", default="1.0.0", help="Release version string")
    args = parser.parse_args()

    try:
        package_release(Path(args.output_dir), args.version)
        return 0
    except Exception as exc:
        print(f"[ERROR] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
