import importlib.util
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
VALIDATION = ROOT / "Development" / "AI_Validation"

if str(VALIDATION) not in sys.path:
    sys.path.insert(0, str(VALIDATION))


def load(name: str, path: pathlib.Path):
    spec = importlib.util.spec_from_file_location(name, path)
    assert spec and spec.loader
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


player_model = load("player_model", VALIDATION / "player_model.py")
player_experiment = load("player_experiment", VALIDATION / "player_experiment.py")


LEVEL = ROOT / "Game" / "Assets" / "Levels" / "inicio.lvl"


class PlayerExperimentTests(unittest.TestCase):
    def test_profiles_are_ordered_by_expected_skill(self):
        profiles = player_model.PROFILES
        self.assertEqual(
            [p.name for p in profiles],
            ["novice", "beginner", "intermediate", "advanced", "expert"],
        )
        for weaker, stronger in zip(profiles, profiles[1:]):
            self.assertLessEqual(stronger.horizontal_error_px, weaker.horizontal_error_px)
            self.assertLessEqual(stronger.charge_error, weaker.charge_error)
            self.assertGreaterEqual(stronger.consistency, weaker.consistency)

    def test_learning_reduces_error_but_does_not_create_perfect_control(self):
        profile = player_model.profile_by_name("beginner")
        learned = profile.after_failure()
        self.assertLess(learned.horizontal_error_px, profile.horizontal_error_px)
        self.assertLess(learned.charge_error, profile.charge_error)
        self.assertGreater(learned.horizontal_error_px, 0.0)
        self.assertGreater(learned.charge_error, 0.0)

    def test_episode_is_reproducible(self):
        profile = player_model.profile_by_name("intermediate")
        first = player_experiment.simulate_session(LEVEL, profile, seed=42, max_events=20)
        second = player_experiment.simulate_session(LEVEL, profile, seed=42, max_events=20)
        self.assertEqual(first, second)

    def test_episode_reports_physical_units(self):
        profile = player_model.profile_by_name("novice")
        session = player_experiment.simulate_session(LEVEL, profile, seed=7, max_events=20)
        self.assertGreaterEqual(session.progress_max_px, 0.0)
        self.assertGreaterEqual(session.total_progress_loss_px, 0.0)
        self.assertGreaterEqual(session.largest_progress_loss_px, 0.0)
        self.assertGreaterEqual(session.failure_count, 0)
        self.assertGreaterEqual(session.jump_count, 0)
        self.assertGreaterEqual(session.recovery_count, 0)
        if session.completion_time_s is not None:
            self.assertGreater(session.completion_time_s, 0.0)

    def test_no_artificial_timeout_is_encoded_as_completion_time(self):
        profile = player_model.profile_by_name("expert")
        session = player_experiment.simulate_session(LEVEL, profile, seed=1, max_events=1)
        self.assertFalse(session.completed)
        self.assertIsNone(session.completion_time_s)


if __name__ == "__main__":
    unittest.main()
