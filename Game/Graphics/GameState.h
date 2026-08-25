#pragma once

namespace gfx {

// Runtime state shared by the game loop and rendering layer.
enum class GameState {
    PLAYING,
    PAUSED,
    CREDITS,
    MENU,
    EDITOR,
};

} // namespace gfx
