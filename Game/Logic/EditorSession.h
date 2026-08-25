#pragma once
// =============================================================================
//  Game/Logic/EditorSession.h
//
//  Fase 9.6 — camada de orquestração do editor.
//  Liga InputManager + EditorInteractionController sem conhecer Vulkan nem
//  estado de câmera: o Level Editor ocupa sempre uma única tela lógica 640x360.
// =============================================================================

#include "Core/KeyBindings.h"
#include "Logic/EditorInteraction.h"
#include "Logic/EditorRenderSnapshot.h"
#include "Logic/InputManager.h"

#include <cstdint>

namespace logic {

struct EditorPreview {
    bool visible = false;
    AABB bounds{};
    EditorToolMode tool = EditorToolMode::STAMP;
};

class EditorSession {
public:
    explicit EditorSession(bool finalCampaignLevel,
                           const AABB& initialGround = {
                               {0.0f, 0.0f}, {640.0f, 16.0f}
                           });

    LevelEditorDocument& document() { return m_document; }
    const LevelEditorDocument& document() const { return m_document; }

    EditorInteractionController& controller() { return m_controller; }
    const EditorInteractionController& controller() const { return m_controller; }

    EditorCursor cursor() const { return m_cursor; }
    EditorMouseMode mouseMode() const { return m_controller.mode(); }
    EditorSizePreset sizePreset() const { return m_controller.sizePreset(); }
    EditorToolMode toolMode() const { return m_controller.toolMode(); }

    EditorPreview preview() const;
    EditorRenderSnapshot renderSnapshot() const;

    // Atualiza input e interação para uma única tela lógica 640x360.
    void update(const InputManager& input,
                const core::KeyBindings& bindings,
                int32_t windowWidth,
                int32_t windowHeight);

    void cancelInteraction();

private:
    void updateCursor(const InputManager& input,
                      int32_t windowWidth,
                      int32_t windowHeight);
    void updateKeyboard(const InputManager& input,
                        const core::KeyBindings& bindings);
    void updateMouse(const InputManager& input);

    LevelEditorDocument m_document;
    EditorInteractionController m_controller;
    EditorCursor m_cursor{};
    Vec2 m_pressedWorld{};
    bool m_leftDragActive = false;
};

} // namespace logic
