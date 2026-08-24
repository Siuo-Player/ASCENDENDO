#pragma once
// =============================================================================
//  Game/Core/GameAction.h
// =============================================================================
#include <cstddef>

namespace core {

enum class GameAction {
    MoveLeft,
    MoveRight,
    Jump,
    Pause,
    UIConfirm,
    UILeft,
    UIRight,
    OpenEditor,
    Quit,
    EditorPanUp,
    EditorPanDown,
    EditorToggleMode,
    EditorSizeDown,
    EditorSizeUp,
    DeleteSelection,
};

inline constexpr GameAction ALL_ACTIONS[] = {
    GameAction::MoveLeft, GameAction::MoveRight, GameAction::Jump,
    GameAction::Pause, GameAction::UIConfirm, GameAction::UILeft,
    GameAction::UIRight, GameAction::OpenEditor, GameAction::Quit,
    GameAction::EditorPanUp, GameAction::EditorPanDown,
    GameAction::EditorToggleMode, GameAction::EditorSizeDown,
    GameAction::EditorSizeUp, GameAction::DeleteSelection,
};
inline constexpr std::size_t ACTION_COUNT = sizeof(ALL_ACTIONS) / sizeof(ALL_ACTIONS[0]);

const char* actionDisplayName(GameAction action);
const char* actionSerializedName(GameAction action);
bool actionFromSerializedName(const char* name, GameAction& out);

} // namespace core
