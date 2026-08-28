#pragma once

namespace logic {

// Semantic input sample consumed by one fixed simulation tick.
// This type intentionally contains no hardware, window, or key-binding state.
struct TickInput {
    bool left = false;
    bool right = false;
    bool jumpHeld = false;
    bool jumpPressed = false;
    bool jumpReleased = false;
};

} // namespace logic
