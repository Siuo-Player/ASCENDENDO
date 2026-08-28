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

EditorRenderSnapshot EditorSession::renderSnapshot() const {
    EditorRenderSnapshot snapshot;
    snapshot.platforms.reserve(m_document.platformCount());
    for (const auto& platform : m_document.platforms())
        snapshot.platforms.push_back(platform.bounds);

    snapshot.hasSelection = m_controller.hasSelection();
    snapshot.selectedIndex = snapshot.hasSelection
        ? m_controller.selectedIndex()
        : static_cast<std::size_t>(-1);
    snapshot.cursorWorld = m_cursor.world;
    snapshot.tool = m_controller.toolMode();
    snapshot.sizePreset = m_controller.sizePreset();

    const EditorPreview p = preview();
    snapshot.previewVisible = p.visible;
    snapshot.previewBounds = p.bounds;
    return snapshot;
}

void EditorSession::updateCursor(const InputManager& input,
                                 int32_t windowWidth,
                                 int32_t windowHeight) {
    const core::LogicalPoint logical = core::windowToLogical(
        input.cursorX(), input.cursorY(),
        windowWidth, windowHeight,
        config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
    m_cursor.logical = {logical.x, logical.y};

    // Level Editor = exactly one 640x360 screen. There is no camera transform
    // here: cursor coordinates and rendered geometry share the same space.
    m_cursor.world = m_controller.cursorFromLogical(m_cursor.logical, {0.0f, 0.0f}).world;
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