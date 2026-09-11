import importlib.util
import pathlib
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = ROOT / "Development" / "AI_Validation" / "validate_campaign.py"
spec = importlib.util.spec_from_file_location("validate_campaign", MODULE_PATH)
assert spec and spec.loader
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)


class CampaignValidationTests(unittest.TestCase):
    def write_level(self, text: str) -> pathlib.Path:
        handle = tempfile.NamedTemporaryFile("w", suffix=".lvl", delete=False, encoding="utf-8")
        handle.write(text)
        handle.close()
        path = pathlib.Path(handle.name)
        self.addCleanup(path.unlink, missing_ok=True)
        return path

    def test_rejects_out_of_bounds_geometry(self):
        path = self.write_level("NAME bad\nPLATFORM 0 0 641 16\nFLAG 0 16 640 32\n")
        with self.assertRaisesRegex(ValueError, r"geometry outside 640\.0x360\.0"):
            module.parse_level(str(path))

    def test_rejects_multiple_flags(self):
        path = self.write_level(
            "NAME bad\nPLATFORM 0 0 640 16\nFLAG 0 16 32 32\nFLAG 32 16 32 32\n"
        )
        with self.assertRaisesRegex(ValueError, "multiple FLAG"):
            module.parse_level(str(path))

    def test_reachable_level_gets_difficulty_report(self):
        path = self.write_level(
            "NAME reachable\nSPAWN 312 16\nPLATFORM 280 0 160 16\n"
            "PLATFORM 436 60 176 16\nPLATFORM 148 120 176 16\n"
            "PLATFORM 436 175 128 16\nFLAG 389 195 128 40\n"
        )
        report = module.validate_level(str(path))
        self.assertTrue(report["valid"], report["errors"])
        self.assertIn(report["difficulty"]["rating"], {"tutorial", "easy", "medium", "hard", "extreme"})
        self.assertGreaterEqual(report["difficulty"]["score"], 0.0)
        self.assertLessEqual(report["difficulty"]["score"], 100.0)

    def test_non_final_flag_is_mechanically_valid_without_campaign_policy(self):
        path = self.write_level(
            "NAME nonfinal\nPLATFORM 0 0 640 16\nFLAG 288 40 64 32\n"
        )
        report = module.validate_level(str(path))
        self.assertTrue(report["valid"], report["errors"])

    def test_strict_campaign_policy_rejects_non_final_flag(self):
        path = self.write_level(
            "NAME nonfinal\nPLATFORM 0 0 640 16\nFLAG 288 40 64 32\n"
        )
        report = module.validate_level(str(path), strict_flag_policy=True, final=False)
        self.assertFalse(report["valid"])
        self.assertIn("non-final campaign level", " ".join(report["errors"]))

    def test_level_without_platforms_is_rejected(self):
        path = self.write_level("NAME empty\n")
        report = module.validate_level(str(path))
        self.assertFalse(report["valid"])
        self.assertIn("no platforms", report["errors"])


if __name__ == "__main__":
    unittest.main()
