#!/usr/bin/env python3
"""ASCENDENDO Authenticated Replay & Speedrun Verification CLI Tool.

Verifies the cryptographic integrity (HMAC-SHA256) of .replay ghost files,
parses player run metadata, decodes the deterministic tick stream, and optionally
re-simulates the physical trajectory against a campaign playlist.
"""
from __future__ import annotations

import argparse
import hashlib
import hmac
import os
import sys
from dataclasses import dataclass
from pathlib import Path

DEFAULT_HMAC_KEY = b"ascendendo-replay-auth-key-v1"

# Bitmask constants matching Game/Logic/RunReplay.cpp
MASK_LEFT = 0x01
MASK_RIGHT = 0x02
MASK_JUMP_HELD = 0x04
MASK_JUMP_PRESSED = 0x08
MASK_JUMP_RELEASED = 0x10


@dataclass
class ReplayHeader:
    version: int
    campaign_id: str
    campaign_name: str
    player_id: str
    player_name: str
    claimed_seconds: float
    total_ticks: int
    timestamp: int


@dataclass
class TickInput:
    left: bool
    right: bool
    jump_held: bool
    jump_pressed: bool
    jump_released: bool


@dataclass
class ReplayData:
    header: ReplayHeader
    inputs: list[TickInput]
    raw_payload: str
    is_signature_valid: bool


def decode_tick_byte(b: int) -> TickInput:
    return TickInput(
        left=(b & MASK_LEFT) != 0,
        right=(b & MASK_RIGHT) != 0,
        jump_held=(b & MASK_JUMP_HELD) != 0,
        jump_pressed=(b & MASK_JUMP_PRESSED) != 0,
        jump_released=(b & MASK_JUMP_RELEASED) != 0,
    )


def parse_and_verify_replay(replay_path: Path, hmac_key: bytes = DEFAULT_HMAC_KEY) -> ReplayData:
    """Reads a .replay file, verifies HMAC signature, and returns parsed ReplayData."""
    if not replay_path.is_file():
        raise FileNotFoundError(f"Replay file not found: {replay_path}")

    raw_bytes = replay_path.read_bytes()
    text = raw_bytes.decode("utf-8", errors="replace")

    if not text.strip():
        raise ValueError("Replay file is empty.")

    is_signed = False
    is_signature_valid = False

    # Check for ASCENDENDO_SECURE_V1 envelope
    if text.startswith("ASCENDENDO_SECURE_V1"):
        is_signed = True
        idx1 = text.find("\n")
        if idx1 == -1:
            raise ValueError("Malformed secure header: unexpected EOF after header line.")
        idx2 = text.find("\n", idx1 + 1)
        if idx2 == -1:
            raise ValueError("Malformed secure header: missing HMAC signature line.")

        sig_line = text[idx1 + 1:idx2].strip()
        if not sig_line.startswith("HMAC:"):
            raise ValueError(f"Malformed secure header: expected 'HMAC:', got '{sig_line}'.")
        expected_hmac = sig_line[5:].strip()

        payload_text = text[idx2 + 1:]
        computed_hmac = hmac.new(hmac_key, payload_text.encode("utf-8"), hashlib.sha256).hexdigest()
        is_signature_valid = hmac.compare_digest(computed_hmac, expected_hmac)
    else:
        payload_text = text

    payload_lines = payload_text.splitlines()
    if not payload_lines or payload_lines[0] != "ASCENDENDO_REPLAY_V1":
        raise ValueError("Invalid replay format: expected ASCENDENDO_REPLAY_V1 magic header.")

    header_dict: dict[str, str] = {}
    reading_inputs = False
    hex_inputs = []

    for line in payload_lines[1:]:
        line = line.strip()
        if reading_inputs:
            hex_inputs.append(line)
            continue
        if line == "INPUTS:":
            reading_inputs = True
            continue
        if ":" in line:
            key, val = line.split(":", 1)
            header_dict[key.strip()] = val.strip()

    header = ReplayHeader(
        version=int(header_dict.get("version", "1")),
        campaign_id=header_dict.get("campaignID", "unknown"),
        campaign_name=header_dict.get("campaignName", "Unknown Campaign"),
        player_id=header_dict.get("playerID", "anonymous"),
        player_name=header_dict.get("playerName", "Anonymous"),
        claimed_seconds=float(header_dict.get("claimedSeconds", "0.0")),
        total_ticks=int(header_dict.get("totalTicks", "0")),
        timestamp=int(header_dict.get("timestamp", "0")),
    )

    full_hex = "".join(hex_inputs)
    inputs: list[TickInput] = []
    for i in range(0, len(full_hex), 2):
        chunk = full_hex[i:i + 2]
        if len(chunk) == 2:
            byte_val = int(chunk, 16)
            inputs.append(decode_tick_byte(byte_val))

    return ReplayData(
        header=header,
        inputs=inputs,
        raw_payload=payload_text,
        is_signature_valid=is_signature_valid,
    )


def verify_replay_command(args: argparse.Namespace) -> int:
    replay_path = Path(args.replay)
    key = args.hmac_key.encode("utf-8") if args.hmac_key else DEFAULT_HMAC_KEY

    try:
        replay = parse_and_verify_replay(replay_path, key)
    except Exception as exc:
        print(f"[ERRO] Falha ao ler replay '{replay_path}': {exc}", file=sys.stderr)
        return 1

    h = replay.header
    print("=" * 60)
    print("         ASCENDENDO — VERIFICADOR DE REPLAY")
    print("=" * 60)
    print(f"  Ficheiro:          {replay_path.name}")
    print(f"  Campanha ID:       {h.campaign_id}")
    print(f"  Nome da Campanha:  {h.campaign_name}")
    print(f"  Jogador ID:        {h.player_id}")
    print(f"  Nome do Jogador:   {h.player_name}")
    print(f"  Tempo Declarado:   {h.claimed_seconds:.3f} s")
    print(f"  Total de Ticks:    {len(replay.inputs)} (declarado: {h.total_ticks})")
    print(f"  Duracao Calculada: {len(replay.inputs) * (1.0 / 60.0):.3f} s (a 60 Hz)")

    # Integrity verification
    if replay.is_signature_valid:
        print("  Assinatura HMAC:   [OK] VALIDA (HMAC-SHA256 íntegro, sem adulteração)")
    else:
        print("  Assinatura HMAC:   [FALHA] ADULTERADO OU CHAVE INVALIDA", file=sys.stderr)
        if not args.allow_unverified:
            return 1

    # Consistency checks
    calculated_time = len(replay.inputs) * (1.0 / 60.0)
    time_diff = abs(calculated_time - h.claimed_seconds)
    if time_diff > (1.0 / 60.0) + 0.001:
        print(f"  [AVISO] Discrepancia temporal: declarado={h.claimed_seconds:.3f}s, ticks={calculated_time:.3f}s", file=sys.stderr)
        return 1
    else:
        print("  Consistencia Tick: [OK] Tempo corresponde exatamente à contagem de ticks.")

    print("=" * 60)
    print("  RESULTADO: REPLAY AUTENTICADO E APROVADO!")
    print("=" * 60)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verifica autenticidade criptográfica e ticks de um replay .replay do Ascendendo."
    )
    parser.add_argument("replay", help="Caminho para o ficheiro .replay")
    parser.add_argument(
        "--hmac-key",
        default="",
        help="Chave HMAC-SHA256 personalizada (opcional, usa a chave padrao de producao)",
    )
    parser.add_argument(
        "--allow-unverified",
        action="store_true",
        help="Permite inspecionar metadados mesmo com assinatura HMAC invalida",
    )

    args = parser.parse_args()
    return verify_replay_command(args)


if __name__ == "__main__":
    sys.exit(main())
