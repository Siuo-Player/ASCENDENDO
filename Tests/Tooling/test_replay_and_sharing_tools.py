from __future__ import annotations

import hashlib
import hmac
import json
import zipfile
from pathlib import Path

import pytest

from Development.Tools.verify_replay import (
    DEFAULT_HMAC_KEY,
    ReplayHeader,
    TickInput,
    decode_tick_byte,
    parse_and_verify_replay,
)
from Development.Tools.package_campaign import (
    package_campaign,
    unpack_campaign,
    compute_sha256,
)


def test_decode_tick_byte():
    # 0x01 = left, 0x02 = right, 0x04 = held, 0x08 = pressed, 0x10 = released
    b = 0x01 | 0x04
    t = decode_tick_byte(b)
    assert t.left is True
    assert t.right is False
    assert t.jump_held is True
    assert t.jump_pressed is False
    assert t.jump_released is False


def test_replay_verification_and_tamper_detection(tmp_path: Path):
    payload = (
        "ASCENDENDO_REPLAY_V1\n"
        "version:1\n"
        "campaignID:test_campaign\n"
        "campaignName:Test Campaign\n"
        "playerID:speedrunner_01\n"
        "playerName:Speedrunner\n"
        "claimedSeconds:1.5000\n"
        "totalTicks:90\n"
        "timestamp:1700000000\n"
        "INPUTS:\n"
        + "0104" * 45
        + "\n"
    )

    sig = hmac.new(DEFAULT_HMAC_KEY, payload.encode("utf-8"), hashlib.sha256).hexdigest()
    signed_file = tmp_path / "run.replay"
    signed_content = f"ASCENDENDO_SECURE_V1\nHMAC:{sig}\n{payload}"
    signed_file.write_bytes(signed_content.encode("utf-8"))

    # 1. Valid replay
    data = parse_and_verify_replay(signed_file)
    assert data.is_signature_valid is True
    assert data.header.campaign_id == "test_campaign"
    assert data.header.player_id == "speedrunner_01"
    assert len(data.inputs) == 90
    assert data.inputs[0].left is True
    assert data.inputs[1].jump_held is True

    # 2. Tampered replay (altered claimed time)
    tampered_payload = payload.replace("claimedSeconds:1.5000", "claimedSeconds:0.5000")
    tampered_file = tmp_path / "tampered.replay"
    tampered_file.write_bytes(f"ASCENDENDO_SECURE_V1\nHMAC:{sig}\n{tampered_payload}".encode("utf-8"))

    tampered_data = parse_and_verify_replay(tampered_file)
    assert tampered_data.is_signature_valid is False


def test_package_and_unpack_campaign(tmp_path: Path):
    source_dir = tmp_path / "my_campaign"
    source_dir.mkdir()

    lvl1 = source_dir / "step1.lvl"
    lvl1.write_text("NAME Step 1\nPLATFORM 200 30 100 16\n", encoding="utf-8")
    lvl2 = source_dir / "step2.lvl"
    lvl2.write_text("NAME Step 2\nPLATFORM 300 80 80 16\n", encoding="utf-8")

    camp_file = source_dir / "campaign.txt"
    camp_file.write_text("step1.lvl\nstep2.lvl\n", encoding="utf-8")

    out_dir = tmp_path / "dist"
    pkg = package_campaign(
        campaign_file=camp_file,
        output_dir=out_dir,
        author="Tester",
        campaign_id="test_camp",
        skip_validation=True,
    )

    assert pkg.is_file()
    assert pkg.name == "test_camp.ascendendo-campaign"

    # Test unpacking
    unpack_dir = tmp_path / "unpacked"
    target = unpack_campaign(pkg, unpack_dir)
    assert target.is_dir()
    assert (target / "campaign.txt").is_file()
    assert (target / "step1.lvl").is_file()
    assert (target / "step2.lvl").is_file()
    assert (target / "manifest.json").is_file()

    manifest = json.loads((target / "manifest.json").read_text(encoding="utf-8"))
    assert manifest["campaignID"] == "test_camp"
    assert manifest["levelCount"] == 2
    assert manifest["author"] == "Tester"


def test_unpack_zip_slip_prevention(tmp_path: Path):
    # Craft an evil zip containing ../escaped.txt
    evil_zip = tmp_path / "evil.zip"
    with zipfile.ZipFile(evil_zip, "w") as zf:
        zf.writestr("manifest.json", '{"campaignID": "evil"}')
        zf.writestr("../pwned.txt", "malicious payload")

    target_dir = tmp_path / "target"
    with pytest.raises(ValueError, match="travessia de diretorio"):
        unpack_campaign(evil_zip, target_dir)
