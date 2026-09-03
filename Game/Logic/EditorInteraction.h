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

    // Camera remains a presentation concern; Logic receives only the
    // position needed to map logical cursor coordinates into world space.
    EditorCursor cursorFromLogical(const Vec2& logical,
                                   const Vec2& cameraPosition) const;

    // Devolve platformCount() quando nada foi encontrado.
    std::size_t hitPlatform(const Vec2& world) const;

    bool stampAt(const Vec2& world);
    bool dragFromTo(const Vec2& startWorld, const Vec2& endWorld);

    bool beginMove(const Vec2& world);
    bool updateMove(const Vec2& world);
    bool endMove();
    bool cancelMove();

    bool placeSpawnAt(const Vec2& world);
    bool placeFlagAt(const Vec2& world);
    bool removeFlag();

    bool deleteAt(const Vec2& world);

    void clearSelection();
    bool hasSelection() const { return m_selected != npos(); }
    std::size_t selectedIndex() const { return m_selected; }

    void setSizePreset(EditorSizePreset preset) { m_sizePreset = preset; }
    EditorSizePreset sizePreset() const { return m_sizePreset; }

    // Ferramenta persistente de entidade. PLATFORM mantém STAMP/DRAG;
    // SPAWN move a posição inicial e FLAG coloca/remove o objetivo final.
    void setEntityTool(EditorEntityTool tool) { m_entityTool = tool; }
    EditorEntityTool entityTool() const { return m_entityTool; }

    // Ferramenta persistente do editor. O G alterna entre STAMP e DRAG.
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
    EditorEntityTool     m_entityTool = EditorEntityTool::PLATFORM;
    EditorToolMode       m_toolMode = EditorToolMode::STAMP;
    EditorSizePreset     m_sizePreset = EditorSizePreset::MEDIUM;
    float                m_moveOffsetX = 0.0f;
    float                m_moveOffsetY = 0.0f;
    AABB                 m_moveOriginalBounds{};
    bool                 m_moveHasOriginal = false;
};

} // namespace logic
