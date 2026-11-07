import unittest
from packages.studio_auth import StudioAuthCore, AuthMode


class StudioAuthAscendendoTests(unittest.TestCase):
    def setUp(self):
        self.auth = StudioAuthCore(
            project_slug="ascendendo",
            sector_salt="ascendendo-sector-salt-2026",
            secret_key="ascendendo-dev-key",
            pbkdf2_iterations=10_000,
        )

    def test_ppid_prefix_and_zero_blast_radius(self):
        # 1. Verify Ascendendo prefix is 'as_'
        google_sub = "google_user_unique_12345"
        ascendendo_id = self.auth.derive_pairwise_id(google_sub)
        self.assertTrue(ascendendo_id.startswith("as_"), f"Expected 'as_' prefix, got {ascendendo_id}")

        # 2. Verify cross-project isolation: same Google Sub yields DIFFERENT ID in RedWar
        redwar_auth = StudioAuthCore(
            project_slug="redwar",
            sector_salt="ascendendo-sector-salt-2026",
            secret_key="ascendendo-dev-key",
            pbkdf2_iterations=10_000,
        )
        redwar_id = redwar_auth.derive_pairwise_id(google_sub)
        self.assertTrue(redwar_id.startswith("re_"))
        self.assertNotEqual(ascendendo_id, redwar_id, "Zero blast radius violated: PPIDs must be different!")

    def test_guest_session_creation(self):
        guest = self.auth.create_guest_session()
        self.assertTrue(guest.player_id.startswith("guest_"))
        self.assertEqual(guest.auth_mode, AuthMode.GUEST)
        self.assertTrue(guest.is_anonymous)

    def test_password_hashing_and_constant_time_verification(self):
        salt, hash_val = self.auth.hash_password("AscendendoJump2026!")
        self.assertTrue(self.auth.verify_password("AscendendoJump2026!", salt, hash_val))
        self.assertFalse(self.auth.verify_password("WrongPassword!", salt, hash_val))


if __name__ == "__main__":
    unittest.main()
