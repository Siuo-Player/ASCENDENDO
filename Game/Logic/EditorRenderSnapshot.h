#pragma once
// =============================================================================
//  Game/Logic/EditorRenderSnapshot.h
//
//  Dados mínimos necessários para desenhar o editor.
//  Independente de Vulkan; não expõe LevelEditorDocument ao renderer.
// =============================================================================

#include "Logic/Physics.h"
#include "Logic/EditorInteraction.h"

#include <cstddef>
#include <vector>

namespace logic {

struct EditorRenderSnapshot {
    std::vector<AABB> platforms;
    std::size_t selectedIndex = static_cast<std::size_t>(-1);

    Vec2 cursorWorld{};
    AABB previewBounds{};

    bool hasSelection = false;
    bool previewVisible = false;

    EditorToolMode tool = EditorToolMode::STAMP;
    EditorSizePreset sizePreset = EditorSizePreset::MEDIUM;
};

} // namespace logic
