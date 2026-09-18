from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from Development.Tools import validate_repository_state as state


class RepositoryStateValidatorTests(unittest.TestCase):
    def _catalogue(self, root: Path, entries: int) -> tuple[Path, Path]:
        assets = root / "Assets"
        campaign_dir = assets / "Campaigns" / "01-test"
        levels_dir = campaign_dir / "levels"
        levels_dir.mkdir(parents=True)

        level_names = []
        for index in range(entries):
            name = f"level-{index + 1:03d}.lvl"
            (levels_dir / name).write_text(
                f"NAME Level {index + 1}\n",
                encoding="utf-8",
            )
            level_names.append(f"levels/{name}")

        (campaign_dir / "campaign.txt").write_text(
            "\n".join(level_names) + "\n",
            encoding="utf-8",
        )

        catalogue = assets / "Campaigns" / "catalogue.txt"
        catalogue.write_text(
            "test|Campanha Teste|01-test/campaign.txt\n",
            encoding="utf-8",
        )

        return assets / "Levels" / "campaign.txt", catalogue

    def test_matching_canonical_campaign_count_passes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            legacy, catalogue = self._catalogue(root, 3)
            legacy.parent.mkdir(parents=True)
            legacy.write_text("legacy.lvl\n", encoding="utf-8")

            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            readme.write_text("catálogo oficial com **3 níveis**.\n", encoding="utf-8")
            roadmap.write_text("catálogo oficial com **3 níveis**.\n", encoding="utf-8")

            with (
                patch.object(state, "CAMPAIGN", legacy),
                patch.object(state, "CATALOGUE", catalogue),
                patch.object(state, "README", readme),
                patch.object(state, "ROADMAP", roadmap),
            ):
                self.assertEqual(state.validate(), [])

    def test_stale_documented_count_fails(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            legacy, catalogue = self._catalogue(root, 2)
            legacy.parent.mkdir(parents=True)
            legacy.write_text("legacy.lvl\n", encoding="utf-8")

            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            readme.write_text("catálogo oficial com **15 níveis**.\n", encoding="utf-8")
            roadmap.write_text("catálogo oficial com **2 níveis**.\n", encoding="utf-8")

            with (
                patch.object(state, "CAMPAIGN", legacy),
                patch.object(state, "CATALOGUE", catalogue),
                patch.object(state, "README", readme),
                patch.object(state, "ROADMAP", roadmap),
            ):
                errors = state.validate()

            self.assertEqual(len(errors), 1)
            self.assertIn("README.md", errors[0])
            self.assertIn("15", errors[0])
            self.assertIn("2", errors[0])

    def test_duplicate_canonical_entries_fail(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            legacy, catalogue = self._catalogue(root, 2)
            campaign = catalogue.parent / "01-test" / "campaign.txt"
            campaign.write_text("levels/level-001.lvl\nlevels/level-001.lvl\n", encoding="utf-8")
            legacy.parent.mkdir(parents=True)
            legacy.write_text("legacy.lvl\n", encoding="utf-8")

            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            readme.write_text("catálogo oficial com **2 níveis**.\n", encoding="utf-8")
            roadmap.write_text("catálogo oficial com **2 níveis**.\n", encoding="utf-8")

            with (
                patch.object(state, "CAMPAIGN", legacy),
                patch.object(state, "CATALOGUE", catalogue),
                patch.object(state, "README", readme),
                patch.object(state, "ROADMAP", roadmap),
            ):
                errors = state.validate()

            self.assertTrue(any("duplicate" in error for error in errors))

    def test_legacy_campaign_is_used_when_canonical_catalogue_is_missing(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            campaign = root / "campaign.txt"
            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            missing_catalogue = root / "missing" / "catalogue.txt"

            campaign.write_text("one.lvl\ntwo.lvl\n", encoding="utf-8")
            readme.write_text("campanha actualmente com **2 níveis**.\n", encoding="utf-8")
            roadmap.write_text("campanha de **2 níveis**.\n", encoding="utf-8")

            with (
                patch.object(state, "CAMPAIGN", campaign),
                patch.object(state, "CATALOGUE", missing_catalogue),
                patch.object(state, "README", readme),
                patch.object(state, "ROADMAP", roadmap),
            ):
                self.assertEqual(state.validate(), [])

    def test_comments_and_blank_lines_do_not_count(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            legacy, catalogue = self._catalogue(root, 2)
            campaign = catalogue.parent / "01-test" / "campaign.txt"
            campaign.write_text(
                "# comment\n\nlevels/level-001.lvl\n# another\nlevels/level-002.lvl\n",
                encoding="utf-8",
            )
            legacy.parent.mkdir(parents=True)
            legacy.write_text("legacy.lvl\n", encoding="utf-8")

            readme = root / "README.md"
            roadmap = root / "ROADMAP.md"
            readme.write_text("catálogo oficial com **2 níveis**.\n", encoding="utf-8")
            roadmap.write_text("catálogo oficial com **2 níveis**.\n", encoding="utf-8")

            with (
                patch.object(state, "CAMPAIGN", legacy),
                patch.object(state, "CATALOGUE", catalogue),
                patch.object(state, "README", readme),
                patch.object(state, "ROADMAP", roadmap),
            ):
                self.assertEqual(state.validate(), [])


if __name__ == "__main__":
    unittest.main()
