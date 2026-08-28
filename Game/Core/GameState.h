#pragma once

namespace core {

// Runtime state owned by the game/session state machine.
// Rendering may consume this value, but the state itself is not a graphics type.
enum class GameState {
    PLAYING,
    PAUSED,
    CREDITS,
    MENU,
    EDITOR,
};

} // namespace core
