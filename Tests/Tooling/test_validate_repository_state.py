from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from Development.Tools import validate_repository_state as state


class RepositoryStateValidatorTests(unittest.TestCase):
    def test_matching_campaign_counts_pass(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            campaign = root / "campaign.txt"
            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            campaign.write_text("one.lvl\ntwo.lvl\nthree.lvl\n", encoding="utf-8")
            readme.write_text("campanha actualmente com **3 níveis**.\n", encoding="utf-8")
            roadmap.write_text("campanha de **3 níveis**.\n", encoding="utf-8")
            with patch.object(state, "CAMPAIGN", campaign), patch.object(state, "README", readme), patch.object(state, "ROADMAP", roadmap):
                self.assertEqual(state.validate(), [])

    def test_stale_documented_count_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            campaign = root / "campaign.txt"
            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            campaign.write_text("one.lvl\ntwo.lvl\n", encoding="utf-8")
            readme.write_text("campanha actualmente com **15 níveis**.\n", encoding="utf-8")
            roadmap.write_text("campanha de **2 níveis**.\n", encoding="utf-8")
            with patch.object(state, "CAMPAIGN", campaign), patch.object(state, "README", readme), patch.object(state, "ROADMAP", roadmap):
                errors = state.validate()
            self.assertEqual(len(errors), 1)
            self.assertIn("README.md", errors[0])
            self.assertIn("15", errors[0])
            self.assertIn("2", errors[0])

    def test_duplicate_campaign_entries_fail(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            campaign = root / "campaign.txt"
            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            campaign.write_text("one.lvl\none.lvl\n", encoding="utf-8")
            readme.write_text("campanha actualmente com **2 níveis**.\n", encoding="utf-8")
            roadmap.write_text("campanha de **2 níveis**.\n", encoding="utf-8")
            with patch.object(state, "CAMPAIGN", campaign), patch.object(state, "README", readme), patch.object(state, "ROADMAP", roadmap):
                errors = state.validate()
            self.assertTrue(any("duplicate" in error for error in errors))

    def test_comments_and_blank_lines_do_not_count(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            campaign = root / "campaign.txt"
            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            campaign.write_text("# comment\n\none.lvl\n# another\ntwo.lvl\n", encoding="utf-8")
            readme.write_text("campanha actualmente com **2 níveis**.\n", encoding="utf-8")
            roadmap.write_text("campanha de **2 níveis**.\n", encoding="utf-8")
            with patch.object(state, "CAMPAIGN", campaign), patch.object(state, "README", readme), patch.object(state, "ROADMAP", roadmap):
                self.assertEqual(state.validate(), [])


if __name__ == "__main__":
    unittest.main()
