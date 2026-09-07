from __future__ import annotations

import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))

from Development.Tools.validate_platform_asset_registry import validate_registry


VALID_SHA = "a" * 64


def populated_registry(path: str, digest: str) -> str:
    return f"""# Registry\n\n## Candidate A\n- `runtime_path`: `{path}`\n- `content_sha256`: `{digest}`\n"""


class PlatformAssetRegistryValidatorTests(unittest.TestCase):
    def test_unpopulated_identity_is_allowed(self) -> None:
        text = """## Candidate A\n- `runtime_path`: `UNPOPULATED — exact binary not yet staged`\n- `content_sha256`: `UNPOPULATED — exact binary not yet staged`\n"""
        with tempfile.TemporaryDirectory() as tmp:
            self.assertEqual(validate_registry(text, Path(tmp)), [])

    def test_partial_identity_fails_closed(self) -> None:
        text = f"""## Candidate A\n- `runtime_path`: `Game/Assets/A.png`\n- `content_sha256`: `UNPOPULATED`\n"""
        with tempfile.TemporaryDirectory() as tmp:
            self.assertTrue(validate_registry(text, Path(tmp)))

    def test_malformed_hash_fails(self) -> None:
        text = populated_registry("Game/Assets/A.png", "not-a-sha256")
        with tempfile.TemporaryDirectory() as tmp:
            self.assertTrue(validate_registry(text, Path(tmp)))

    def test_unsafe_path_fails(self) -> None:
        for path in (
            "../outside.png",
            "/absolute/path.png",
            "C:/absolute/path.png",
            "C:relative.png",
        ):
            text = populated_registry(path, VALID_SHA)
            with tempfile.TemporaryDirectory() as tmp:
                self.assertTrue(validate_registry(text, Path(tmp)), path)

    def test_unpopulated_sentinel_is_exact(self) -> None:
        text = populated_registry("UNPOPULATEDevil/path.png", VALID_SHA)
        with tempfile.TemporaryDirectory() as tmp:
            self.assertTrue(validate_registry(text, Path(tmp)))

        text = """## Candidate A\n- `runtime_path`: `UNPOPULATEDfoo`\n- `content_sha256`: `UNPOPULATEDfoo`\n"""
        with tempfile.TemporaryDirectory() as tmp:
            self.assertTrue(validate_registry(text, Path(tmp)))

    def test_missing_file_fails(self) -> None:
        text = populated_registry("Game/Assets/A.png", VALID_SHA)
        with tempfile.TemporaryDirectory() as tmp:
            self.assertTrue(validate_registry(text, Path(tmp)))

    def test_matching_file_and_hash_pass(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            target = root / "Game" / "Assets" / "A.png"
            target.parent.mkdir(parents=True)
            target.write_bytes(b"exact asset bytes")
            digest = hashlib.sha256(target.read_bytes()).hexdigest()
            text = populated_registry("Game/Assets/A.png", digest)
            self.assertEqual(validate_registry(text, root), [])

    def test_hash_mismatch_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            target = root / "Game" / "Assets" / "A.png"
            target.parent.mkdir(parents=True)
            target.write_bytes(b"different bytes")
            text = populated_registry("Game/Assets/A.png", VALID_SHA)
            self.assertTrue(validate_registry(text, root))


if __name__ == "__main__":
    unittest.main()
