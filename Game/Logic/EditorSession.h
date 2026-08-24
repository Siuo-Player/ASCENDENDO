#pragma once
// =============================================================================
//  Game/Logic/EditorSession.h
//
//  Fase 9.4 — camada de orquestração do editor.
//  Liga InputManager + Camera + EditorInteractionController sem conhecer
//  Vulkan. A sessão é a fonte de estado da interação visual.
// =============================================================================

#include "Core/KeyBindings.h"
#include "Graphics/Camera.h"
#include "Logic/EditorInteraction.h"
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

    // Atualiza input e interação do editor para um frame.
    void update(const InputManager& input,
                const core::KeyBindings& bindings,
                const gfx::Camera& camera,
                int32_t windowWidth,
                int32_t windowHeight);

    void cancelInteraction();

private:
    void updateCursor(const InputManager& input,
                      const gfx::Camera& camera,
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
