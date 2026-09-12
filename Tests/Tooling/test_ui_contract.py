from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GAME_SESSION = ROOT / "Game" / "Logic" / "GameSession.cpp"
RUNTIME_PATHS = ROOT / "Game" / "Core" / "RuntimePaths.cpp"
MAIN = ROOT / "main.cpp"
MAKEFILE = ROOT / "Makefile"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def test_controls_shortcut_has_runtime_handler():
    source = read(GAME_SESSION)
    assert "GameAction::OpenControls" in source
    assert "stateMachine_.select(4, 5)" in source
    assert "stateMachine_.select(0, 5)" in source


def test_menu_uses_mouse_hover_before_click():
    source = read(GAME_SESSION)
    assert "hoveredMenuBox" in source
    assert "hitTestMenuBox(pt.x, pt.y, count, logicalWidth)" in source


def test_runtime_assets_resolve_from_build_directory():
    source = read(RUNTIME_PATHS)
    assert 'candidate / "Game" / "Assets"' in source
    assert "for (int depth = 0; depth < 4" in source


def test_fullscreen_and_run_commands_are_advertised():
    main = read(MAIN)
    makefile = read(MAKEFILE)
    assert "F11 ecrã inteiro" in main
    assert "make run" in makefile
    assert "run: game" in makefile
