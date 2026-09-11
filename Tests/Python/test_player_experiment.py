import importlib.util
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = ROOT / "Development" / "AI_Validation" / "player_model.py"
EXPERIMENT_PATH = ROOT / "Development" / "AI_Validation" / "player_experiment.py"


def load(name: str, path: pathlib.Path):
    spec = importlib.util.spec_from_file_location(name, path)
    assert spec and spec.loader
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


player_model = load("player_model", MODULE_PATH)
player_experiment = load("player_experiment", EXPERIMENT_PATH)


class PlayerExperimentTests(unittest.TestCase):
    def test_profiles_are_ordered_by_expected_skill(self):
        profiles = player_model.PROFILES
        self.assertEqual([p.name for p in profiles], ["novice", "beginner", "intermediate", "advanced", "expert"])
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

    def test_session_is_reproducible(self):
        profile = player_model.profile_by_name("intermediate")
        first = player_experiment.simulate_session("test", profile, 0.5, 5, 42)
        second = player_experiment.simulate_session("test", profile, 0.5, 5, 42)
        self.assertEqual(first, second)

    def test_session_records_learning_attempts(self):
        profile = player_model.profile_by_name("novice")
        session = player_experiment.simulate_session("test", profile, 0.95, 4, 7)
        self.assertEqual(len(session.attempts), 4)
        self.assertTrue(all(attempt.attempt >= 1 for attempt in session.attempts))
        self.assertTrue(all(0.0 <= attempt.progress <= 1.0 for attempt in session.attempts))


if __name__ == "__main__":
    unittest.main()
