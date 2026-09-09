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
    OpenCampaignEditor,
    OpenControls,
    Quit,
    EditorPanUp,
    EditorPanDown,
    EditorCursorLeft,
    EditorCursorRight,
    EditorCursorUp,
    EditorCursorDown,
    EditorToggleMode,
    EditorSizeDown,
    EditorSizeUp,
    EditorPlace,
    EditorUndo,
    EditorRedo,
    DeleteSelection,
    EditorSave,
    EditorTest,
    EditorValidate,
    EditorSelectPlatform,
    EditorSelectSpawn,
    EditorSelectFlag,
    CampaignSelectPrevious,
    CampaignSelectNext,
    CampaignMoveEarlier,
    CampaignMoveLater,
    CampaignOpenLevel,
};

inline constexpr GameAction ALL_ACTIONS[] = {
    GameAction::MoveLeft, GameAction::MoveRight, GameAction::Jump,
    GameAction::Pause, GameAction::UIConfirm, GameAction::UILeft,
    GameAction::UIRight, GameAction::OpenEditor, GameAction::OpenCampaignEditor,
    GameAction::OpenControls, GameAction::Quit, GameAction::EditorPanUp,
    GameAction::EditorPanDown, GameAction::EditorCursorLeft,
    GameAction::EditorCursorRight, GameAction::EditorCursorUp,
    GameAction::EditorCursorDown, GameAction::EditorToggleMode,
    GameAction::EditorSizeDown, GameAction::EditorSizeUp,
    GameAction::EditorPlace, GameAction::EditorUndo, GameAction::EditorRedo,
    GameAction::DeleteSelection, GameAction::EditorSave,
    GameAction::EditorTest, GameAction::EditorValidate,
    GameAction::EditorSelectPlatform, GameAction::EditorSelectSpawn,
    GameAction::EditorSelectFlag, GameAction::CampaignSelectPrevious,
    GameAction::CampaignSelectNext, GameAction::CampaignMoveEarlier,
    GameAction::CampaignMoveLater, GameAction::CampaignOpenLevel,
};
inline constexpr std::size_t ACTION_COUNT = sizeof(ALL_ACTIONS) / sizeof(ALL_ACTIONS[0]);

const char* actionDisplayName(GameAction action);
const char* actionSerializedName(GameAction action);
bool actionFromSerializedName(const char* name, GameAction& out);

} // namespace core
