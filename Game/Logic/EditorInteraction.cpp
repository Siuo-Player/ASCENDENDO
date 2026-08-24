#include "Logic/EditorInteraction.h"
#include "Core/Config.h"

#include <algorithm>

namespace logic {

EditorCursor EditorInteractionController::cursorFromLogical(const Vec2& logical,
                                                              const gfx::Camera& camera) const {
    // O renderer usa coordenadas logicas com origem no canto superior esquerdo
    // e a camera como offset de mundo. O mesmo referencial evita drift entre
    // desenho e clique.
    return {logical, {logical.x + camera.position.x,
                      logical.y + camera.position.y}};
}

std::size_t EditorInteractionController::hitPlatform(const Vec2& world) const {
    const auto& platforms = m_document.platforms();
    // Iteração inversa: quando entidades se sobrepõem, a última criada fica
    // visualmente por cima e é portanto a mais natural para selecionar.
    for (std::size_t i = platforms.size(); i-- > 0;) {
        const AABB& b = platforms[i].bounds;
        if (world.x >= b.min.x && world.x < b.max.x &&
            world.y >= b.min.y && world.y < b.max.y)
            return i;
    }
    return platforms.size();
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
    const std::size_t index = hitPlatform(world);
    if (index == m_document.platformCount()) return false;

    const AABB& b = m_document.platforms()[index].bounds;
    m_selected = index;
    m_mode = EditorMouseMode::MOVING;
    m_moveOffsetX = world.x - b.min.x;
    m_moveOffsetY = world.y - b.min.y;
    return true;
}

bool EditorInteractionController::updateMove(const Vec2& world) {
    if (!hasSelection() || m_mode != EditorMouseMode::MOVING)
        return false;

    const Vec2 requestedMin{
        world.x - m_moveOffsetX,
        world.y - m_moveOffsetY,
    };
    return m_document.movePlatform(m_selected, requestedMin);
}

bool EditorInteractionController::endMove() {
    if (m_mode != EditorMouseMode::MOVING) return false;
    m_mode = EditorMouseMode::NONE;
    return hasSelection();
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
    m_moveOffsetX = 0.0f;
    m_moveOffsetY = 0.0f;
}

} // namespace logic
