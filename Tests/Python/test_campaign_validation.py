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
        path = self.write_level("NAME bad\nPLATFORM 0 16 641 16\n")
        with self.assertRaisesRegex(ValueError, r"geometry outside 640\.0x360\.0"):
            module.parse_level(str(path))

    def test_rejects_authored_flag(self):
        path = self.write_level(
            "NAME bad\nPLATFORM 280 80 160 16\nFLAG 0 96 640 32\n"
        )
        with self.assertRaisesRegex(ValueError, "FLAG is derived"):
            module.parse_level(str(path))

    def test_rejects_authored_spawn(self):
        path = self.write_level(
            "NAME bad\nSPAWN 320 16\nPLATFORM 280 80 160 16\n"
        )
        with self.assertRaisesRegex(ValueError, "SPAWN is derived"):
            module.parse_level(str(path))

    def test_reachable_level_gets_difficulty_report(self):
        path = self.write_level(
            "NAME reachable\n"
            "PLATFORM 280 16 160 16\n"
            "PLATFORM 436 76 176 16\n"
            "PLATFORM 148 136 176 16\n"
            "PLATFORM 436 196 128 16\n"
        )
        report = module.validate_level(str(path))
        self.assertTrue(report["valid"], report["errors"])
        self.assertIn(report["difficulty"]["rating"], {"tutorial", "easy", "medium", "hard", "extreme"})
        self.assertGreaterEqual(report["difficulty"]["score"], 0.0)
        self.assertLessEqual(report["difficulty"]["score"], 100.0)

    def test_final_level_derives_goal_from_highest_platform(self):
        path = self.write_level(
            "NAME final\n"
            "PLATFORM 280 16 160 16\n"
            "PLATFORM 436 76 176 16\n"
            "PLATFORM 148 136 176 16\n"
            "PLATFORM 436 196 128 16\n"
        )
        report = module.validate_level(str(path), final=True)
        self.assertTrue(report["valid"], report["errors"])
        self.assertTrue(report["has_flag"])

    def test_level_without_platforms_is_rejected(self):
        path = self.write_level("NAME empty\n")
        report = module.validate_level(str(path))
        self.assertFalse(report["valid"])
        self.assertIn("no platforms", report["errors"])


    def test_catalogue_validates_all_campaigns_and_preserves_identity(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            catalogue_root = root / "Campaigns"
            first = catalogue_root / "01-first"
            second = catalogue_root / "02-second"
            (first / "levels").mkdir(parents=True)
            (second / "levels").mkdir(parents=True)
            (first / "levels" / "level-001.lvl").write_text("NAME first\nPLATFORM 240 64 160 16\n", encoding="utf-8")
            (second / "levels" / "level-001.lvl").write_text("NAME second\nPLATFORM 0 300 64 16\n", encoding="utf-8")
            (first / "campaign.txt").write_text("levels/level-001.lvl\n", encoding="utf-8")
            (second / "campaign.txt").write_text("levels/level-001.lvl\n", encoding="utf-8")
            catalogue = catalogue_root / "catalogue.txt"
            catalogue.write_text(
                "first|Primeira|01-first/campaign.txt\n"
                "second|Segunda|02-second/campaign.txt\n",
                encoding="utf-8",
            )
            reports = module.validate_catalogue(str(catalogue))
            self.assertEqual([report["campaign_id"] for report in reports], ["first", "second"])
            self.assertFalse(all(report["valid"] for report in reports))

if __name__ == "__main__":
    unittest.main()
