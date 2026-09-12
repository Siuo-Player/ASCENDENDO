from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
GAME_SESSION = ROOT / "Game" / "Logic" / "GameSession.cpp"
RUNTIME_PATHS = ROOT / "Game" / "Core" / "RuntimePaths.cpp"
MAIN = ROOT / "main.cpp"
MAKEFILE = ROOT / "Makefile"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


class UIContractTests(unittest.TestCase):
    def test_controls_shortcut_has_runtime_handler(self):
        source = read(GAME_SESSION)
        self.assertIn("GameAction::OpenControls", source)
        self.assertIn("stateMachine_.select(4, 5)", source)
        self.assertIn("stateMachine_.select(0, 5)", source)

    def test_menu_uses_mouse_hover_before_click(self):
        source = read(GAME_SESSION)
        self.assertIn("hoveredMenuBox", source)
        self.assertIn("hitTestMenuBox(pt.x, pt.y, count, logicalWidth)", source)

    def test_runtime_assets_resolve_from_build_directory(self):
        source = read(RUNTIME_PATHS)
        self.assertIn('candidate / "Game" / "Assets"', source)
        self.assertIn("for (int depth = 0; depth < 4", source)

    def test_fullscreen_and_run_commands_are_advertised(self):
        main = read(MAIN)
        makefile = read(MAKEFILE)
        self.assertIn("F11 ecrã inteiro", main)
        self.assertIn("make run", makefile)
        self.assertIn("run: game", makefile)


if __name__ == "__main__":
    unittest.main()
