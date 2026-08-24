#include "Logic/EditorSession.h"
#include "Core/Config.h"
#include "Core/Viewport.h"

#include <algorithm>

namespace logic {

namespace {

bool insideEditorCanvas(const AABB& bounds) {
    return bounds.min.x >= 0.0f && bounds.min.y >= 0.0f &&
           bounds.max.x <= config::LOGICAL_WIDTH &&
           bounds.max.y <= config::LOGICAL_HEIGHT;
}

}

EditorSession::EditorSession(bool finalCampaignLevel, const AABB& initialGround)
    : m_document(finalCampaignLevel, initialGround),
      m_controller(m_document) {}

EditorPreview EditorSession::preview() const {
    EditorPreview result;
    result.tool = m_controller.toolMode();

    if (m_controller.hasSelection() &&
        m_controller.mode() == EditorMouseMode::MOVING) {
        const AABB bounds = m_document.platforms()[m_controller.selectedIndex()].bounds;
        result.visible = insideEditorCanvas(bounds);
        result.bounds = bounds;
        return result;
    }

    if (m_controller.toolMode() == EditorToolMode::STAMP) {
        const Vec2 size = LevelEditorDocument::presetSize(m_controller.sizePreset());
        result.bounds = {
            {m_cursor.world.x - size.x * 0.5f, m_cursor.world.y - size.y * 0.5f},
            {m_cursor.world.x + size.x * 0.5f, m_cursor.world.y + size.y * 0.5f},
        };
        result.visible = insideEditorCanvas(result.bounds);
        return result;
    }

    if (m_leftDragActive && m_controller.mode() == EditorMouseMode::NONE) {
        result.bounds = {
            {std::min(m_pressedWorld.x, m_cursor.world.x),
             std::min(m_pressedWorld.y, m_cursor.world.y)},
            {std::max(m_pressedWorld.x, m_cursor.world.x),
             std::max(m_pressedWorld.y, m_cursor.world.y)},
        };
        result.visible = insideEditorCanvas(result.bounds);
    }
    return result;
}

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
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorToggleMode))
        m_controller.toggleToolMode();

    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSizeDown)) {
        switch (m_controller.sizePreset()) {
            case EditorSizePreset::SMALL: break;
            case EditorSizePreset::MEDIUM: m_controller.setSizePreset(EditorSizePreset::SMALL); break;
            case EditorSizePreset::LARGE: m_controller.setSizePreset(EditorSizePreset::MEDIUM); break;
        }
    }
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSizeUp)) {
        switch (m_controller.sizePreset()) {
            case EditorSizePreset::SMALL: break;
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

        if (m_controller.toolMode() == EditorToolMode::STAMP) {
            m_controller.stampAt(m_cursor.world);
            return;
        }

        m_leftDragActive = true;
        m_controller.clearSelection();
        return;
    }

    if (leftHeld && m_leftDragActive &&
        m_controller.mode() == EditorMouseMode::MOVING)
        m_controller.updateMove(m_cursor.world);

    if (leftReleased && m_leftDragActive) {
        if (m_controller.mode() == EditorMouseMode::MOVING) {
            m_controller.updateMove(m_cursor.world);
            m_controller.endMove();
        } else if (m_controller.toolMode() == EditorToolMode::DRAG) {
            m_controller.dragFromTo(m_pressedWorld, m_cursor.world);
        }
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
