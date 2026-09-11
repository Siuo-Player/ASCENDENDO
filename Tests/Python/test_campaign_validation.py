import importlib.util
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = ROOT / "Development" / "AI_Validation" / "validate_campaign.py"
spec = importlib.util.spec_from_file_location("validate_campaign", MODULE_PATH)
assert spec and spec.loader
module = importlib.util.module_from_spec(spec)
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

    def test_reports_unreachable_flag(self):
        path = self.write_level(
            "NAME impossible\nPLATFORM 0 0 640 16\nPLATFORM 0 300 32 16\nFLAG 0 332 32 28\n"
        )
        report = module.validate_level(str(path))
        self.assertFalse(report["valid"])
        self.assertTrue(any("unreachable" in e.lower() for e in report["errors"]))

    def test_reports_mechanical_difficulty_for_reachable_level(self):
        path = self.write_level(
            "NAME reachable\nPLATFORM 0 0 640 16\nPLATFORM 436 89 176 16\n"
            "PLATFORM 148 174 176 16\nPLATFORM 436 219 128 16\n"
            "FLAG 389 235 128 40\n"
        )
        report = module.validate_level(str(path))
        self.assertTrue(report["valid"], report["errors"])
        difficulty = report["difficulty"]
        self.assertIn(difficulty["rating"], {"tutorial", "easy", "medium", "hard", "extreme"})
        self.assertGreaterEqual(difficulty["score"], 0.0)
        self.assertLessEqual(difficulty["score"], 100.0)
        self.assertGreaterEqual(difficulty["mean_robustness"], 0.0)
        self.assertLessEqual(difficulty["mean_robustness"], 1.0)
        self.assertGreaterEqual(difficulty["minimum_robustness"], 0.0)
        self.assertLessEqual(difficulty["minimum_robustness"], 1.0)


if __name__ == "__main__":
    unittest.main()
