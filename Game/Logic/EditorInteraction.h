#pragma once
// =============================================================================
//  Game/Logic/EditorInteraction.h
//
//  Fase 9.4 — controlador de interação do editor.
//  Mantém a política de seleção/mouse separada de GLFW e Vulkan para permitir
//  testes unitários sem GPU.
// =============================================================================

#include "Graphics/Camera.h"
#include "Logic/LevelEditor.h"

#include <cstddef>

namespace logic {

enum class EditorMouseMode {
    NONE,
    STAMPING,
    DRAGGING,
    MOVING,
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
                                   const gfx::Camera& camera) const;

    // Devolve platformCount() quando nada foi encontrado.
    std::size_t hitPlatform(const Vec2& world) const;

    bool stampAt(const Vec2& world);
    bool dragFromTo(const Vec2& startWorld, const Vec2& endWorld);

    bool beginMove(const Vec2& world);
    bool updateMove(const Vec2& world);
    bool endMove();

    bool deleteAt(const Vec2& world);

    void clearSelection();
    bool hasSelection() const { return m_selected != npos(); }
    std::size_t selectedIndex() const { return m_selected; }

    void setSizePreset(EditorSizePreset preset) { m_sizePreset = preset; }
    EditorSizePreset sizePreset() const { return m_sizePreset; }

    EditorMouseMode mode() const { return m_mode; }

private:
    static std::size_t npos() { return static_cast<std::size_t>(-1); }
    static AABB centeredStamp(const Vec2& world, Vec2 size);

    LevelEditorDocument& m_document;
    std::size_t          m_selected = npos();
    EditorMouseMode      m_mode = EditorMouseMode::NONE;
    EditorSizePreset     m_sizePreset = EditorSizePreset::MEDIUM;
    float                m_moveOffsetX = 0.0f;
    float                m_moveOffsetY = 0.0f;
};

} // namespace logic
