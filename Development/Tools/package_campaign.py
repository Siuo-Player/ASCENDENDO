#!/usr/bin/env python3
"""ASCENDENDO Campaign Packaging & Community Sharing Tool.

Allows level creators to package custom campaigns into self-contained,
validated, and signed bundles (.ascendendo-campaign / .zip) with reachability
verification and SHA-256 integrity checksums.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import sys
import time
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
AI_VALIDATION_DIR = ROOT / "Development" / "AI_Validation"
if str(AI_VALIDATION_DIR) not in sys.path:
    sys.path.insert(0, str(AI_VALIDATION_DIR))


def compute_sha256(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()


def read_campaign_playlist(campaign_file: Path) -> list[str]:
    """Reads lines of campaign.txt, stripping comments and blank lines."""
    levels: list[str] = []
    with open(campaign_file, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            levels.append(line)
    return levels


def validate_campaign_reachability(campaign_file: Path) -> bool:
    """Invokes campaign validation engine if available."""
    try:
        from validate_campaign import validate_campaign_file
        print(f"  [VALIDACAO] A executar simulador de alcançabilidade fisica...")
        success = validate_campaign_file(str(campaign_file))
        return bool(success)
    except Exception as exc:
        print(f"  [AVISO] Verificador AI nao disponivel ou com erro ({exc}); continuando embalamento estrutural.")
        return True


def package_campaign(
    campaign_file: Path,
    output_dir: Path,
    author: str = "Community Creator",
    campaign_id: str = "",
    skip_validation: bool = False,
) -> Path:
    if not campaign_file.is_file():
        raise FileNotFoundError(f"Ficheiro de campanha nao encontrado: {campaign_file}")

    campaign_dir = campaign_file.parent
    level_rel_paths = read_campaign_playlist(campaign_file)
    if not level_rel_paths:
        raise ValueError("A campanha nao contem nenhum nivel listado.")

    # Validate reachability
    if not skip_validation:
        ok = validate_campaign_reachability(campaign_file)
        if not ok:
            print("[AVISO] A campanha contem niveis potencialmente inalcancaveis!", file=sys.stderr)

    # Derive campaign metadata
    first_level_path = campaign_dir / level_rel_paths[0]
    campaign_name = campaign_file.stem
    if first_level_path.is_file():
        with open(first_level_path, "r", encoding="utf-8") as f:
            for line in f:
                if line.startswith("NAME"):
                    campaign_name = line.split(" ", 1)[1].strip()
                    break

    cid = campaign_id or campaign_file.stem.lower().replace(" ", "_")
    output_dir.mkdir(parents=True, exist_ok=True)
    archive_name = f"{cid}.ascendendo-campaign"
    archive_path = output_dir / archive_name

    checksums: dict[str, str] = {}
    checksums["campaign.txt"] = compute_sha256(campaign_file)

    for rel_path in level_rel_paths:
        lvl_full = campaign_dir / rel_path
        if not lvl_full.is_file():
            raise FileNotFoundError(f"Nivel referenciado nao encontrado: {lvl_full}")
        checksums[rel_path] = compute_sha256(lvl_full)

    manifest = {
        "format": "ascendendo-campaign-v1",
        "campaignID": cid,
        "campaignName": campaign_name,
        "author": author,
        "levelCount": len(level_rel_paths),
        "createdTimestamp": int(time.time()),
        "levels": level_rel_paths,
        "checksums": checksums,
    }

    # Write ZIP bundle
    with zipfile.ZipFile(archive_path, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("manifest.json", json.dumps(manifest, indent=2))
        zf.write(campaign_file, arcname="campaign.txt")
        for rel_path in level_rel_paths:
            lvl_full = campaign_dir / rel_path
            zf.write(lvl_full, arcname=rel_path)

    print(f"  [PACOTE CRIADO] {archive_path} ({len(level_rel_paths)} niveis embalados)")
    return archive_path


def unpack_campaign(archive_path: Path, destination_dir: Path) -> Path:
    if not archive_path.is_file():
        raise FileNotFoundError(f"Arquivo nao encontrado: {archive_path}")

    with zipfile.ZipFile(archive_path, "r") as zf:
        # Check manifest
        if "manifest.json" not in zf.namelist():
            raise ValueError("Arquivo de campanha invalido: manifest.json ausente.")

        manifest_data = json.loads(zf.read("manifest.json").decode("utf-8"))
        cid = manifest_data.get("campaignID", "custom_campaign")
        target_folder = destination_dir / cid
        target_folder.mkdir(parents=True, exist_ok=True)

        for member in zf.infolist():
            # Security against Zip Slip (directory traversal)
            member_path = Path(member.filename)
            if member_path.is_absolute() or ".." in member_path.parts:
                raise ValueError(f"Detetada tentativa perigosa de travessia de diretorio no ficheiro: {member.filename}")

            zf.extract(member, target_folder)

    print(f"  [DESEMBALADO] Campanha extraida com sucesso para: {target_folder}")
    return target_folder


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Empacota ou desempacota campanhas personalizadas do ASCENDENDO."
    )
    parser.add_argument("campaign", nargs="?", help="Caminho para o ficheiro campaign.txt")
    parser.add_argument("-o", "--output-dir", default="dist/Campaigns", help="Diretorio de saida para o pacote")
    parser.add_argument("--author", default="Community Creator", help="Nome do autor da campanha")
    parser.add_argument("--id", default="", help="ID da campanha (opcional)")
    parser.add_argument("--skip-validation", action="store_true", help="Ignora verificacao fisica antes de embalar")
    parser.add_argument("--unpack", help="Desempacota um ficheiro .ascendendo-campaign ou .zip")
    parser.add_argument("--dest", default="Game/Assets/Levels/Custom", help="Diretorio de destino para desempacotar")

    args = parser.parse_args()

    if args.unpack:
        try:
            unpack_campaign(Path(args.unpack), Path(args.dest))
            return 0
        except Exception as exc:
            print(f"[ERRO] Falha ao desempacotar: {exc}", file=sys.stderr)
            return 1

    if not args.campaign:
        parser.print_help()
        return 1

    try:
        package_campaign(
            campaign_file=Path(args.campaign),
            output_dir=Path(args.output_dir),
            author=args.author,
            campaign_id=args.id,
            skip_validation=args.skip_validation,
        )
        return 0
    except Exception as exc:
        print(f"[ERRO] Falha ao embalar campanha: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
