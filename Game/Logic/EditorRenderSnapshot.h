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
    Vec2 spawnPosition{};
    AABB flagBounds{};
    AABB previewBounds{};

    float levelWidth = 640.0f;
    float levelHeight = 360.0f;
    float viewBottomY = 0.0f;
    std::size_t screenCount = 1;

    bool hasSelection = false;
    bool hasFlag = false;
    bool previewVisible = false;

    EditorEntityTool entityTool = EditorEntityTool::PLATFORM;
    EditorToolMode tool = EditorToolMode::STAMP;
    EditorSizePreset sizePreset = EditorSizePreset::MEDIUM;
};

} // namespace logic
