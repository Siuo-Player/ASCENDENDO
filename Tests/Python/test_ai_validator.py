import importlib.util
import pathlib
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = ROOT / "Development" / "AI_Validation" / "ai_validator.py"
spec = importlib.util.spec_from_file_location("ai_validator", MODULE_PATH)
assert spec and spec.loader
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)


class AiValidatorCatalogueTests(unittest.TestCase):
    def write_level(self, path: pathlib.Path, text: str) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def write_campaign(self, campaign_dir: pathlib.Path, level_text: str) -> pathlib.Path:
        level = campaign_dir / "levels" / "level-001.lvl"
        self.write_level(level, level_text)
        playlist = campaign_dir / "campaign.txt"
        playlist.write_text("levels/level-001.lvl\n", encoding="utf-8")
        return playlist

    def test_catalogue_validates_every_campaign(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            campaigns = root / "Campaigns"
            self.write_campaign(
                campaigns / "01-ok",
                "NAME ok\nPLATFORM 240 64 160 16\n",
            )
            self.write_campaign(
                campaigns / "02-broken",
                "NAME broken\nPLATFORM 0 300 64 16\n",
            )
            catalogue = campaigns / "catalogue.txt"
            catalogue.write_text(
                "ok|Primeira|01-ok/campaign.txt\n"
                "broken|Segunda|02-broken/campaign.txt\n",
                encoding="utf-8",
            )

            self.assertFalse(module.validate_catalogue(str(catalogue)))

    def test_catalogue_with_all_valid_campaigns_passes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            campaigns = root / "Campaigns"
            self.write_campaign(
                campaigns / "01-ok",
                "NAME ok\nPLATFORM 240 64 160 16\n",
            )
            self.write_campaign(
                campaigns / "02-ok",
                "NAME also-ok\n"
                "PLATFORM 240 64 160 16\n"
                "PLATFORM 400 124 128 16\n",
            )
            catalogue = campaigns / "catalogue.txt"
            catalogue.write_text(
                "ok1|Primeira|01-ok/campaign.txt\n"
                "ok2|Segunda|02-ok/campaign.txt\n",
                encoding="utf-8",
            )

            self.assertTrue(module.validate_catalogue(str(catalogue)))


if __name__ == "__main__":
    unittest.main()
