from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from Development.AI_Validation.validate_campaign_catalogue import validate_campaign_catalogue


class CampaignCatalogueValidatorTests(unittest.TestCase):
    def _root(self, tmp: str) -> tuple[Path, Path]:
        root = Path(tmp)
        levels = root / "levels"
        levels.mkdir()
        return root / "campaign.txt", levels

    def test_valid_playlist_passes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            (levels / "one.lvl").write_text("NAME One\n", encoding="utf-8")
            (levels / "two.lvl").write_text("NAME Two\n", encoding="utf-8")
            campaign.write_text("# comment\none.lvl\n\ntwo.lvl\n", encoding="utf-8")
            self.assertEqual(validate_campaign_catalogue(campaign, levels), [])

    def test_missing_level_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            campaign.write_text("missing.lvl\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("does not exist" in error for error in errors))

    def test_duplicate_level_is_rejected_case_insensitively(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            (levels / "One.lvl").write_text("NAME One\n", encoding="utf-8")
            campaign.write_text("One.lvl\none.LVL\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("duplicate campaign level" in error for error in errors))

    def test_parent_escape_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            campaign.write_text("../outside.lvl\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("portable" in error for error in errors))

    def test_backslash_path_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            campaign.write_text("nested\\level.lvl\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("portable" in error for error in errors))

    def test_non_lvl_entry_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            (levels / "readme.txt").write_text("not a level\n", encoding="utf-8")
            campaign.write_text("readme.txt\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("must end in .lvl" in error for error in errors))

    def test_whitespace_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            (levels / "one.lvl").write_text("NAME One\n", encoding="utf-8")
            campaign.write_text(" one.lvl\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("whitespace" in error for error in errors))

    def test_empty_campaign_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            campaign.write_text("# only comments\n\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("no level entries" in error for error in errors))

    def test_nested_relative_level_is_allowed(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            nested = levels / "bonus"
            nested.mkdir()
            (nested / "level.lvl").write_text("NAME Bonus\n", encoding="utf-8")
            campaign.write_text("bonus/level.lvl\n", encoding="utf-8")
            self.assertEqual(validate_campaign_catalogue(campaign, levels), [])

    def test_authored_flag_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            campaign, levels = self._root(tmp)
            (levels / "one.lvl").write_text(
                "NAME One\nPLATFORM 0 0 640 16\nFLAG 0 16 640 40\n",
                encoding="utf-8",
            )
            campaign.write_text("one.lvl\n", encoding="utf-8")
            errors = validate_campaign_catalogue(campaign, levels)
            self.assertTrue(any("FLAG is not allowed" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
