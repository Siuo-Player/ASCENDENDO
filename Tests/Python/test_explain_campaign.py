import contextlib
import importlib.util
import io
import pathlib
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
VALIDATOR_PATH = ROOT / "Development" / "AI_Validation" / "validate_campaign.py"
DIAGNOSTIC_PATH = ROOT / "Development" / "AI_Validation" / "explain_campaign.py"

validator_spec = importlib.util.spec_from_file_location("validate_campaign", VALIDATOR_PATH)
assert validator_spec and validator_spec.loader
validator = importlib.util.module_from_spec(validator_spec)
validator_spec.loader.exec_module(validator)
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

    def test_diagnostic_reports_authoritative_control_space_route(self):
        path = self.write_level(
            "NAME explainable\n"
            "PLATFORM 280 16 160 16\n"
            "PLATFORM 436 76 176 16\n"
            "PLATFORM 148 136 176 16\n"
        )
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            diagnostic.explain_level(str(path))

        text = output.getvalue()
        self.assertIn("model: control-space route completion", text)
        self.assertIn("control=", text)
        self.assertIn("launch_coverage=", text)
        self.assertIn("charge_coverage=", text)
        self.assertIn("route product=", text)

        report = validator.validate_level(str(path))
        self.assertTrue(report["valid"], report["errors"])
        self.assertTrue(report["route"])


if __name__ == "__main__":
    unittest.main()
