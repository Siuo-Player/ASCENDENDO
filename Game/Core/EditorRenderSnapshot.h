#pragma once
// =============================================================================
//  Game/Core/EditorRenderSnapshot.h
//
//  Dados mínimos necessários para desenhar o editor.
//  Este tipo é deliberadamente independente de Vulkan e não expõe
//  LevelEditorDocument. A sessão/editor produz o snapshot; o renderer apenas
//  o consome.
// =============================================================================

#include "Logic/Physics.h"
#include "Logic/EditorInteraction.h"

#include <cstddef>
#include <vector>

namespace core {

struct EditorRenderSnapshot {
    std::vector<logic::AABB> platforms;
    std::size_t selectedIndex = static_cast<std::size_t>(-1);

    logic::Vec2 cursorWorld{};
    logic::AABB previewBounds{};

    bool hasSelection = false;
    bool previewVisible = false;

    logic::EditorToolMode tool = logic::EditorToolMode::STAMP;
    logic::EditorSizePreset sizePreset = logic::EditorSizePreset::MEDIUM;
};

} // namespace core
