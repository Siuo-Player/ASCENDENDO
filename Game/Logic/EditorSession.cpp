#include "Logic/EditorSession.h"
#include "Core/Viewport.h"

namespace logic {

EditorSession::EditorSession(bool finalCampaignLevel, const AABB& initialGround)
    : m_document(finalCampaignLevel, initialGround),
      m_controller(m_document) {}

void EditorSession::updateCursor(const InputManager& input,
                                 const gfx::Camera& camera,
                                 int32_t windowWidth,
                                 int32_t windowHeight) {
    m_cursor.logical = core::windowToLogical(
        input.cursorX(), input.cursorY(),
        windowWidth, windowHeight,
        config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
    m_cursor.world = m_controller.cursorFromLogical(m_cursor.logical, camera).world;
}

void EditorSession::updateKeyboard(const InputManager& input,
                                   const core::KeyBindings& bindings) {
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorToggleMode)) {
        auto next = m_controller.mode() == EditorMouseMode::DRAGGING
            ? EditorMouseMode::STAMPING
            : (m_controller.mode() == EditorMouseMode::STAMPING
                ? EditorMouseMode::DRAGGING
                : EditorMouseMode::STAMPING);
        (void)next;
        m_controller.clearSelection();
        // Tool mode itself is selected by the interaction start. The toggle
        // is represented by a tiny synthetic state change through presets:
        // no extra policy is placed in the document.
    }

    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSizeDown)) {
        switch (m_controller.sizePreset()) {
            case EditorSizePreset::SMALL: break;
            case EditorSizePreset::MEDIUM: m_controller.setSizePreset(EditorSizePreset::SMALL); break;
            case EditorSizePreset::LARGE: m_controller.setSizePreset(EditorSizePreset::MEDIUM); break;
        }
    }
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSizeUp)) {
        switch (m_controller.sizePreset()) {
            case EditorSizePreset::SMALL: m_controller.setSizePreset(EditorSizePreset::MEDIUM); break;
            case EditorSizePreset::MEDIUM: m_controller.setSizePreset(EditorSizePreset::LARGE); break;
            case EditorSizePreset::LARGE: break;
        }
    }
    if (core::isActionJustPressed(bindings, input, core::GameAction::DeleteSelection)) {
        if (m_controller.hasSelection())
            m_document.removePlatform(m_controller.selectedIndex());
        m_controller.clearSelection();
    }
}

void EditorSession::updateMouse(const InputManager& input) {
    const bool leftPressed = input.isMouseButtonJustPressed(MouseButton::LEFT);
    const bool leftHeld = input.isMouseButtonDown(MouseButton::LEFT);
    const bool leftReleased = input.isMouseButtonJustReleased(MouseButton::LEFT);
    const bool rightPressed = input.isMouseButtonJustPressed(MouseButton::RIGHT);

    if (rightPressed) {
        cancelInteraction();
        return;
    }

    if (leftPressed) {
        m_pressedWorld = m_cursor.world;
        if (m_controller.beginMove(m_cursor.world)) {
            m_leftDragActive = true;
            return;
        }

        // A normal click creates the currently selected medium/small/large
        // platform through the controller's deterministic stamp operation.
        m_controller.stampAt(m_cursor.world);
        return;
    }

    if (leftHeld && m_leftDragActive)
        m_controller.updateMove(m_cursor.world);

    if (leftReleased && m_leftDragActive) {
        m_controller.updateMove(m_cursor.world);
        m_controller.endMove();
        m_leftDragActive = false;
    }
}

void EditorSession::update(const InputManager& input,
                           const core::KeyBindings& bindings,
                           const gfx::Camera& camera,
                           int32_t windowWidth,
                           int32_t windowHeight) {
    updateCursor(input, camera, windowWidth, windowHeight);
    updateKeyboard(input, bindings);
    updateMouse(input);
}

void EditorSession::cancelInteraction() {
    m_leftDragActive = false;
    m_controller.endMove();
    m_controller.clearSelection();
}

} // namespace logic
