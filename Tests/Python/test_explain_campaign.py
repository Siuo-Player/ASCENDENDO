import importlib.util
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
VALIDATOR_PATH = ROOT / "Development" / "AI_Validation" / "validate_campaign.py"
DIAGNOSTIC_PATH = ROOT / "Development" / "AI_Validation" / "explain_campaign.py"

validator_spec = importlib.util.spec_from_file_location("validate_campaign", VALIDATOR_PATH)
assert validator_spec and validator_spec.loader
validator = importlib.util.module_from_spec(validator_spec)
validator_spec.loader.exec_module(validator)

# The diagnostic imports validate_campaign by module name, so install the same
# module object before loading it.
import sys
sys.modules["validate_campaign"] = validator

diagnostic_spec = importlib.util.spec_from_file_location("explain_campaign", DIAGNOSTIC_PATH)
assert diagnostic_spec and diagnostic_spec.loader
diagnostic = importlib.util.module_from_spec(diagnostic_spec)
diagnostic_spec.loader.exec_module(diagnostic)


class ExplainCampaignTests(unittest.TestCase):
    def write_level(self, text: str) -> pathlib.Path:
        handle = tempfile.NamedTemporaryFile("w", suffix=".lvl", delete=False, encoding="utf-8")
        handle.write(text)
        handle.close()
        path = pathlib.Path(handle.name)
        self.addCleanup(path.unlink, missing_ok=True)
        return path

    def test_diagnostic_reuses_authoritative_transition_search(self):
        path = self.write_level(
            "NAME explainable\n"
            "PLATFORM 280 16 160 16\n"
            "PLATFORM 436 76 176 16\n"
            "PLATFORM 148 136 176 16\n"
        )
        level = validator.parse_level(str(path))
        result = diagnostic.diagnose_level(level)
        self.assertTrue(result["transitions"])
        transition = result["transitions"][0]
        self.assertGreaterEqual(transition.robustness, 0.0)
        self.assertLessEqual(transition.robustness, 1.0)
        self.assertGreaterEqual(transition.landing_margin_px, 0.0)
        self.assertGreaterEqual(transition.launch_margin_px, 0.0)
        self.assertGreaterEqual(transition.charge_margin, 0.0)
        self.assertGreaterEqual(transition.target_width_px, 0.0)
        self.assertIn("robustness", transition.weighted_risks)
        self.assertIn("landing_margin", transition.weighted_risks)


if __name__ == "__main__":
    unittest.main()
