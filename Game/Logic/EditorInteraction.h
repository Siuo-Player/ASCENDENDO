#pragma once
// =============================================================================
//  Game/Logic/EditorInteraction.h
//
//  Fase 9.4 — controlador de interação do editor.
//  Mantém a política de seleção/mouse separada de GLFW e Vulkan para permitir
//  testes unitários sem GPU.
// =============================================================================

#include "Logic/LevelEditor.h"

#include <cstddef>

namespace logic {

enum class EditorMouseMode {
    NONE,
    STAMPING,
    DRAGGING,
    MOVING,
};

enum class EditorResizeHandle {
    NONE,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
};

// Kept for action compatibility. Only PLATFORM represents authored content;
// SPAWN and FLAG are derived state and are never selectable authoring tools.
enum class EditorEntityTool {
    PLATFORM,
    SPAWN,
    FLAG,
};

struct EditorCursor {
    Vec2 logical;
    Vec2 world;
};

class EditorInteractionController {
public:
    explicit EditorInteractionController(LevelEditorDocument& document)
        : m_document(document) {}

    EditorCursor cursorFromLogical(const Vec2& logical,
                                   const Vec2& cameraPosition) const;
    std::size_t hitPlatform(const Vec2& world) const;
    EditorResizeHandle hitResizeHandle(const Vec2& world, float tolerance = 8.0f) const;

    bool stampAt(const Vec2& world);
    bool dragFromTo(const Vec2& startWorld, const Vec2& endWorld);

    bool beginMove(const Vec2& world);
    bool updateMove(const Vec2& world);
    bool endMove();
    bool cancelMove();
    EditorResizeHandle resizeHandle() const { return m_resizeHandle; }

    bool placeSpawnAt(const Vec2& world);
    bool placeFlagAt(const Vec2& world);
    bool removeFlag();

    bool deleteAt(const Vec2& world);

    void clearSelection();
    bool selectPlatform(std::size_t index);
    bool hasSelection() const { return m_selected != npos(); }
    std::size_t selectedIndex() const { return m_selected; }

    void setSizePreset(EditorSizePreset preset) { m_sizePreset = preset; }
    EditorSizePreset sizePreset() const { return m_sizePreset; }

    // Legacy action names remain source-compatible, but the editor has exactly
    // one authoring entity type: PLATFORM.
    void setEntityTool(EditorEntityTool tool) {
        (void)tool;
        m_entityTool = EditorEntityTool::PLATFORM;
    }
    EditorEntityTool entityTool() const { return m_entityTool; }

    void setToolMode(EditorToolMode mode) { m_toolMode = mode; }
    void toggleToolMode() {
        m_toolMode = m_toolMode == EditorToolMode::STAMP
            ? EditorToolMode::DRAG
            : EditorToolMode::STAMP;
    }
    EditorToolMode toolMode() const { return m_toolMode; }

    EditorMouseMode mode() const { return m_mode; }

private:
    static std::size_t npos() { return static_cast<std::size_t>(-1); }
    static AABB centeredStamp(const Vec2& world, Vec2 size);

    LevelEditorDocument& m_document;
    std::size_t          m_selected = npos();
    EditorMouseMode      m_mode = EditorMouseMode::NONE;
    EditorResizeHandle   m_resizeHandle = EditorResizeHandle::NONE;
    EditorEntityTool     m_entityTool = EditorEntityTool::PLATFORM;
    EditorToolMode       m_toolMode = EditorToolMode::STAMP;
    EditorSizePreset     m_sizePreset = EditorSizePreset::MEDIUM;
    float                m_moveOffsetX = 0.0f;
    float                m_moveOffsetY = 0.0f;
    AABB                 m_moveOriginalBounds{};
    AABB                 m_resizeOriginalBounds{};
    Vec2                 m_resizeFixedCorner{};
    bool                 m_moveHasOriginal = false;
    bool                 m_resizeHasOriginal = false;
};

} // namespace logic
