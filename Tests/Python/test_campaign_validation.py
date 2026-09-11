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
        tmp = tempfile.NamedTemporaryFile("w", suffix=".lvl", delete=False, encoding="utf-8")
        tmp.write(text)
        tmp.close()
        path = pathlib.Path(tmp.name)
        self.addCleanup(path.unlink, missing_ok=True)
        return path

    def test_rejects_out_of_bounds_geometry(self):
        path = self.write_level("NAME bad\nPLATFORM 0 0 641 16\nFLAG 0 16 640 32\n")
        with self.assertRaisesRegex(ValueError, "outside 640x360"):
            module.validate_level(str(path))

    def test_rejects_multiple_flags(self):
        path = self.write_level(
            "NAME bad\nPLATFORM 0 0 640 16\nFLAG 0 16 32 32\nFLAG 32 16 32 32\n"
        )
        with self.assertRaisesRegex(ValueError, "multiple FLAG"):
            module.validate_level(str(path))

    def test_rejects_non_final_flag_when_campaign_policy_is_non_final(self):
        path = self.write_level("NAME nonfinal\nPLATFORM 0 0 640 16\nFLAG 0 16 64 32\n")
        report = module.validate_level(str(path), forbid_flag=True)
        self.assertFalse(report["valid"])
        self.assertIn("non-final level", " ".join(report["errors"]))

    def test_parses_authored_spawn_and_variable_screen_count(self):
        path = self.write_level(
            "NAME vertical\nSCREENS 2\nSPAWN 100 20\n"
            "PLATFORM 80 0 160 16\nPLATFORM 80 420 160 16\n"
        )
        parsed = module.parse_level(str(path))
        self.assertEqual(parsed.screen_count, 2)
        self.assertEqual(parsed.spawn, (100.0, 20.0))
        self.assertEqual(len(parsed.platforms), 2)

    def test_reports_unreachable_flag(self):
        path = self.write_level(
            "NAME impossible\nPLATFORM 0 0 640 16\nPLATFORM 0 300 32 16\nFLAG 0 332 32 28\n"
        )
        report = module.validate_level(str(path))
        self.assertFalse(report["valid"])
        self.assertTrue(any("unreachable" in e.lower() for e in report["errors"]))

    def test_reports_mechanical_difficulty_for_reachable_level(self):
        path = self.write_level(
            "NAME reachable\nSPAWN 312 16\nPLATFORM 280 0 160 16\n"
            "PLATFORM 436 89 176 16\nPLATFORM 148 174 176 16\n"
            "PLATFORM 436 219 128 16\nFLAG 389 235 128 40\n"
        )
        report = module.validate_level(str(path), require_flag=True)
        self.assertTrue(report["valid"], report["errors"])
        difficulty = report["difficulty"]
        self.assertIn(difficulty["rating"], {"tutorial", "easy", "medium", "hard", "extreme"})
        self.assertGreaterEqual(difficulty["score"], 0.0)
        self.assertLessEqual(difficulty["score"], 100.0)
        self.assertGreaterEqual(difficulty["mean_robustness"], 0.0)
        self.assertLessEqual(difficulty["mean_robustness"], 1.0)
        self.assertGreaterEqual(difficulty["minimum_robustness"], 0.0)
        self.assertLessEqual(difficulty["minimum_robustness"], 1.0)

    def test_non_final_level_gets_difficulty_from_reachable_top(self):
        path = self.write_level(
            "NAME noflag\nSPAWN 312 16\nPLATFORM 280 0 160 16\nPLATFORM 300 70 128 16\n"
        )
        report = module.validate_level(str(path))
        self.assertTrue(report["valid"], report["errors"])
        self.assertNotEqual(report["difficulty"]["rating"], "unreachable")
        self.assertGreaterEqual(report["difficulty"]["transitions"], 1)


if __name__ == "__main__":
    unittest.main()
