#include "Logic/EditorInteraction.h"
#include "Core/Config.h"

#include <algorithm>
#include <cmath>

namespace logic {

EditorCursor EditorInteractionController::cursorFromLogical(const Vec2& logical,
                                                              const Vec2& cameraPosition) const {
    return {logical, {logical.x + cameraPosition.x,
                      logical.y + cameraPosition.y}};
}

std::size_t EditorInteractionController::hitPlatform(const Vec2& world) const {
    const auto& platforms = m_document.platforms();
    for (std::size_t i = platforms.size(); i-- > 0;) {
        const AABB& b = platforms[i].bounds;
        if (world.x >= b.min.x && world.x < b.max.x &&
            world.y >= b.min.y && world.y < b.max.y)
            return i;
    }
    return platforms.size();
}

EditorResizeHandle EditorInteractionController::hitResizeHandle(const Vec2& world,
                                                                 float tolerance) const {
    if (!hasSelection() || m_selected >= m_document.platformCount())
        return EditorResizeHandle::NONE;

    const AABB& b = m_document.platforms()[m_selected].bounds;
    const auto near = [tolerance](const Vec2& point) {
        return std::fabs(point.x) <= tolerance && std::fabs(point.y) <= tolerance;
    };

    if (near({world.x - b.min.x, world.y - b.min.y}))
        return EditorResizeHandle::TOP_LEFT;
    if (near({world.x - b.max.x, world.y - b.min.y}))
        return EditorResizeHandle::TOP_RIGHT;
    if (near({world.x - b.min.x, world.y - b.max.y}))
        return EditorResizeHandle::BOTTOM_LEFT;
    if (near({world.x - b.max.x, world.y - b.max.y}))
        return EditorResizeHandle::BOTTOM_RIGHT;
    return EditorResizeHandle::NONE;
}

AABB EditorInteractionController::centeredStamp(const Vec2& world, Vec2 size) {
    return {
        {world.x - size.x * 0.5f, world.y - size.y * 0.5f},
        {world.x + size.x * 0.5f, world.y + size.y * 0.5f},
    };
}

bool EditorInteractionController::stampAt(const Vec2& world) {
    std::size_t index = npos();
    if (!m_document.addPlatform(centeredStamp(world, LevelEditorDocument::presetSize(m_sizePreset)), &index))
        return false;

    m_selected = index;
    m_mode = EditorMouseMode::STAMPING;
    return true;
}

bool EditorInteractionController::dragFromTo(const Vec2& startWorld, const Vec2& endWorld) {
    AABB rect{
        {std::min(startWorld.x, endWorld.x), std::min(startWorld.y, endWorld.y)},
        {std::max(startWorld.x, endWorld.x), std::max(startWorld.y, endWorld.y)},
    };

    std::size_t index = npos();
    if (!m_document.addPlatform(rect, &index))
        return false;

    m_selected = index;
    m_mode = EditorMouseMode::DRAGGING;
    return true;
}

bool EditorInteractionController::beginMove(const Vec2& world) {
    if (hasSelection()) {
        const EditorResizeHandle handle = hitResizeHandle(world);
        if (handle != EditorResizeHandle::NONE) {
            const AABB& b = m_document.platforms()[m_selected].bounds;
            m_mode = EditorMouseMode::MOVING;
            m_resizeHandle = handle;
            m_resizeOriginalBounds = b;
            m_resizeHasOriginal = true;
            switch (handle) {
                case EditorResizeHandle::TOP_LEFT:
                    m_resizeFixedCorner = b.max;
                    break;
                case EditorResizeHandle::TOP_RIGHT:
                    m_resizeFixedCorner = {b.min.x, b.max.y};
                    break;
                case EditorResizeHandle::BOTTOM_LEFT:
                    m_resizeFixedCorner = {b.max.x, b.min.y};
                    break;
                case EditorResizeHandle::BOTTOM_RIGHT:
                    m_resizeFixedCorner = b.min;
                    break;
                case EditorResizeHandle::NONE:
                    return false;
            }
            return true;
        }
    }

    const std::size_t index = hitPlatform(world);
    if (index == m_document.platformCount()) return false;

    const AABB& b = m_document.platforms()[index].bounds;
    m_selected = index;
    m_mode = EditorMouseMode::MOVING;
    m_resizeHandle = EditorResizeHandle::NONE;
    m_resizeHasOriginal = false;
    m_resizeOriginalBounds = {};
    m_moveOffsetX = world.x - b.min.x;
    m_moveOffsetY = world.y - b.min.y;
    m_moveOriginalBounds = b;
    m_moveHasOriginal = true;
    return true;
}

bool EditorInteractionController::updateMove(const Vec2& world) {
    if (!hasSelection() || m_mode != EditorMouseMode::MOVING)
        return false;

    if (m_resizeHandle != EditorResizeHandle::NONE) {
        AABB requested{};
        switch (m_resizeHandle) {
            case EditorResizeHandle::TOP_LEFT:
                requested = {world, m_resizeFixedCorner};
                break;
            case EditorResizeHandle::TOP_RIGHT:
                requested = {{m_resizeFixedCorner.x, world.y},
                             {world.x, m_resizeFixedCorner.y}};
                break;
            case EditorResizeHandle::BOTTOM_LEFT:
                requested = {{world.x, m_resizeFixedCorner.y},
                             {m_resizeFixedCorner.x, world.y}};
                break;
            case EditorResizeHandle::BOTTOM_RIGHT:
                requested = {m_resizeFixedCorner, world};
                break;
            case EditorResizeHandle::NONE:
                return false;
        }
        if (requested.min.x > requested.max.x || requested.min.y > requested.max.y)
            return false;
        return m_document.resizePlatform(m_selected, requested);
    }

    const Vec2 requestedMin{
        world.x - m_moveOffsetX,
        world.y - m_moveOffsetY,
    };
    return m_document.movePlatform(m_selected, requestedMin);
}

bool EditorInteractionController::endMove() {
    if (m_mode != EditorMouseMode::MOVING) return false;
    m_mode = EditorMouseMode::NONE;
    m_resizeHandle = EditorResizeHandle::NONE;
    m_moveHasOriginal = false;
    m_moveOriginalBounds = {};
    m_resizeHasOriginal = false;
    m_resizeOriginalBounds = {};
    m_resizeFixedCorner = {};
    return hasSelection();
}

bool EditorInteractionController::cancelMove() {
    if (m_mode != EditorMouseMode::MOVING)
        return false;

    if (m_selected >= m_document.platformCount()) {
        clearSelection();
        return false;
    }

    bool restored = false;
    if (m_resizeHandle != EditorResizeHandle::NONE && m_resizeHasOriginal) {
        restored = m_document.resizePlatform(m_selected, m_resizeOriginalBounds);
    } else if (m_moveHasOriginal) {
        restored = m_document.movePlatform(m_selected, m_moveOriginalBounds.min);
    }

    clearSelection();
    return restored;
}

bool EditorInteractionController::placeSpawnAt(const Vec2& world) {
    (void)world;
    clearSelection();
    return false;
}

bool EditorInteractionController::placeFlagAt(const Vec2& world) {
    (void)world;
    clearSelection();
    return false;
}

bool EditorInteractionController::removeFlag() {
    // The campaign goal is derived from the highest platform and cannot be removed.
    return false;
}

bool EditorInteractionController::deleteAt(const Vec2& world) {
    const std::size_t index = hitPlatform(world);
    if (index == m_document.platformCount()) return false;

    if (!m_document.removePlatform(index)) return false;

    if (m_selected == index) {
        clearSelection();
    } else if (m_selected > index && m_selected != npos()) {
        --m_selected;
    }
    m_mode = EditorMouseMode::NONE;
    return true;
}

void EditorInteractionController::clearSelection() {
    m_selected = npos();
    m_mode = EditorMouseMode::NONE;
    m_resizeHandle = EditorResizeHandle::NONE;
    m_moveOffsetX = 0.0f;
    m_moveOffsetY = 0.0f;
    m_moveOriginalBounds = {};
    m_resizeOriginalBounds = {};
    m_resizeFixedCorner = {};
    m_moveHasOriginal = false;
    m_resizeHasOriginal = false;
}

bool EditorInteractionController::selectPlatform(std::size_t index) {
    if (index >= m_document.platformCount()) return false;
    m_selected = index;
    m_mode = EditorMouseMode::NONE;
    m_resizeHandle = EditorResizeHandle::NONE;
    m_moveOffsetX = 0.0f;
    m_moveOffsetY = 0.0f;
    m_moveOriginalBounds = {};
    m_resizeOriginalBounds = {};
    m_resizeFixedCorner = {};
    m_moveHasOriginal = false;
    m_resizeHasOriginal = false;
    return true;
}

} // namespace logic
