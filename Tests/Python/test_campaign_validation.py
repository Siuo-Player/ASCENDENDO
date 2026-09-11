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

    def test_reachable_level_gets_control_space_difficulty_report(self):
        path = self.write_level(
            "NAME reachable\n"
            "PLATFORM 280 16 160 16\n"
            "PLATFORM 436 76 176 16\n"
            "PLATFORM 148 136 176 16\n"
            "PLATFORM 436 196 128 16\n"
        )
        report = module.validate_level(str(path))
        self.assertTrue(report["valid"], report["errors"])
        difficulty = report["difficulty"]
        self.assertIn(difficulty["rating"], {"tutorial", "easy", "medium", "hard", "extreme"})
        self.assertGreaterEqual(difficulty["score"], 0.0)
        self.assertLessEqual(difficulty["score"], 100.0)
        self.assertEqual(difficulty["model"], "control_space_route_completion")
        self.assertGreaterEqual(difficulty["estimated_route_completion"], 0.0)
        self.assertLessEqual(difficulty["estimated_route_completion"], 1.0)
        self.assertGreater(difficulty["required_jumps"], 0)
        self.assertTrue(report["route"])
        self.assertIn("profile", report["route"][0])
        self.assertIn("control_success_probability", report["route"][0]["profile"])

    def test_harder_geometry_has_lower_completion_probability(self):
        easy = self.write_level(
            "NAME easy\n"
            "PLATFORM 240 40 240 16\n"
            "PLATFORM 200 104 240 16\n"
            "PLATFORM 160 168 240 16\n"
        )
        hard = self.write_level(
            "NAME hard\n"
            "PLATFORM 304 40 64 16\n"
            "PLATFORM 96 120 64 16\n"
            "PLATFORM 352 200 64 16\n"
        )
        easy_report = module.validate_level(str(easy))
        hard_report = module.validate_level(str(hard))
        self.assertTrue(easy_report["valid"], easy_report["errors"])
        self.assertTrue(hard_report["valid"], hard_report["errors"])
        self.assertGreaterEqual(
            easy_report["difficulty"]["estimated_route_completion"],
            hard_report["difficulty"]["estimated_route_completion"],
        )

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


if __name__ == "__main__":
    unittest.main()
