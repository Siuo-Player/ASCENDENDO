#include "Logic/EditorSession.h"
#include "Core/Config.h"
#include "Core/Viewport.h"

#include <algorithm>
#include <utility>

namespace logic {

namespace {

bool insideEditorCanvas(const AABB& bounds, float levelWidth, float levelHeight) {
    return bounds.min.x >= 0.0f && bounds.min.y >= 0.0f &&
           bounds.max.x <= levelWidth && bounds.max.y <= levelHeight;
}

bool sameDocumentState(const LevelData& a, const LevelData& b) {
    if (a.screenCount != b.screenCount || a.platforms.size() != b.platforms.size() ||
        a.spawnPosition.has_value() != b.spawnPosition.has_value() ||
        a.flag.has_value() != b.flag.has_value())
        return false;

    for (std::size_t i = 0; i < a.platforms.size(); ++i) {
        const auto& x = a.platforms[i];
        const auto& y = b.platforms[i];
        if (x.min.x != y.min.x || x.min.y != y.min.y ||
            x.max.x != y.max.x || x.max.y != y.max.y)
            return false;
    }
    if (a.spawnPosition &&
        (a.spawnPosition->x != b.spawnPosition->x ||
         a.spawnPosition->y != b.spawnPosition->y))
        return false;
    if (a.flag &&
        (a.flag->min.x != b.flag->min.x || a.flag->min.y != b.flag->min.y ||
         a.flag->max.x != b.flag->max.x || a.flag->max.y != b.flag->max.y))
        return false;
    return true;
}

} // namespace

EditorSession::EditorSession(bool finalCampaignLevel, const AABB& initialGround)
    : m_document(finalCampaignLevel, initialGround),
      m_controller(m_document) {}

void EditorSession::setPersistenceTarget(std::string path, std::string name) {
    m_persistencePath = std::move(path);
    m_documentName = std::move(name);
    if (m_documentName.empty()) m_documentName = "Editor Level";
}

EditorSaveResult EditorSession::saveLevel() {
    return saveLevel(m_persistencePath, m_documentName);
}

EditorSaveResult EditorSession::saveLevel(const std::string& path,
                                          const std::string& name) {
    EditorSaveResult result;
    result.generation = m_document.generation();
    result.path = path;

    if (path.empty()) {
        result.message = "Caminho de gravação vazio";
        m_lastSaveResult = result;
        return result;
    }

    const EditorValidationResult validation = validateEditorDocument(m_document);
    result.validationPassed = validation.valid;
    if (!validation.valid) {
        result.message = validation.message;
        m_lastSaveResult = result;
        return result;
    }

    if (!saveEditorLevel(m_document, path, name)) {
        result.message = "Falha de I/O ao gravar o nível";
        m_lastSaveResult = result;
        return result;
    }

    result.success = true;
    result.message = "Nível gravado com sucesso";
    m_lastSaveResult = result;
    return result;
}

bool EditorSession::startValidation() {
    return startValidation(m_persistencePath);
}

bool EditorSession::startValidation(const std::string& path) {
    if (path.empty() || m_validationTask.running()) return false;

    const std::uint64_t generation = m_document.generation();
    m_validationResult = {};
    m_validationResult.state = EditorValidationState::RUNNING;
    m_validationResult.generation = generation;
    m_validationResult.levelPath = path;
    m_validationResult.message = "Validação em execução";

    return m_validationTask.start(m_document.toLevelData(m_documentName),
                                  generation,
                                  path);
}

EditorAsyncValidationResult EditorSession::pollValidation() {
    refreshValidationResult();
    return m_validationResult;
}

void EditorSession::refreshValidationResult() {
    if (m_validationTask.running()) return;

    const EditorAsyncValidationResult result = m_validationTask.poll();
    if (result.state == EditorValidationState::IDLE) return;

    if (result.state == EditorValidationState::COMPLETE &&
        result.generation != m_document.generation()) {
        m_validationResult = result;
        m_validationResult.state = EditorValidationState::STALE;
        m_validationResult.valid = false;
        m_validationResult.message =
            "Resultado descartado: o documento foi alterado durante a validação";
        return;
    }

    m_validationResult = result;
}

EditorPreview EditorSession::preview() const {
    EditorPreview result;
    result.tool = m_controller.toolMode();

    if (m_controller.entityTool() != EditorEntityTool::PLATFORM)
        return result;

    if (m_controller.hasSelection() &&
        m_controller.mode() == EditorMouseMode::MOVING) {
        const AABB bounds = m_document.platforms()[m_controller.selectedIndex()].bounds;
        result.visible = insideEditorCanvas(bounds,
                                            m_document.levelWidth(),
                                            m_document.levelHeight());
        result.bounds = bounds;
        return result;
    }

    if (m_controller.toolMode() == EditorToolMode::STAMP) {
        const Vec2 size = LevelEditorDocument::presetSize(m_controller.sizePreset());
        result.bounds = {
            {m_cursor.world.x - size.x * 0.5f, m_cursor.world.y - size.y * 0.5f},
            {m_cursor.world.x + size.x * 0.5f, m_cursor.world.y + size.y * 0.5f},
        };
        result.visible = insideEditorCanvas(result.bounds,
                                            m_document.levelWidth(),
                                            m_document.levelHeight());
        return result;
    }

    if (m_leftDragActive && m_controller.mode() == EditorMouseMode::NONE) {
        result.bounds = {
            {std::min(m_pressedWorld.x, m_cursor.world.x),
             std::min(m_pressedWorld.y, m_cursor.world.y)},
            {std::max(m_pressedWorld.x, m_cursor.world.x),
             std::max(m_pressedWorld.y, m_cursor.world.y)},
        };
        result.visible = insideEditorCanvas(result.bounds,
                                            m_document.levelWidth(),
                                            m_document.levelHeight());
    }
    return result;
}

EditorRenderSnapshot EditorSession::renderSnapshot() const {
    EditorRenderSnapshot snapshot;
    snapshot.platforms.reserve(m_document.platformCount());
    for (const auto& platform : m_document.platforms())
        snapshot.platforms.push_back(platform.bounds);

    snapshot.levelWidth = m_document.levelWidth();
    snapshot.levelHeight = m_document.levelHeight();
    snapshot.viewBottomY = m_viewBottomY;
    snapshot.screenCount = m_document.screenCount();

    snapshot.hasSelection = m_controller.hasSelection();
    snapshot.selectedIndex = snapshot.hasSelection
        ? m_controller.selectedIndex()
        : static_cast<std::size_t>(-1);
    snapshot.cursorWorld = m_cursor.world;
    snapshot.spawnPosition = m_document.spawnPosition();
    snapshot.hasFlag = m_document.hasFlag();
    if (snapshot.hasFlag)
        snapshot.flagBounds = *m_document.flag();
    snapshot.entityTool = m_controller.entityTool();
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
    const Vec2 mouseLogical{logical.x, logical.y};

    if (!m_keyboardCursorActive || !m_haveMousePosition ||
        mouseLogical.x != m_lastMouseLogical.x ||
        mouseLogical.y != m_lastMouseLogical.y) {
        m_cursor.logical = mouseLogical;
        m_cursor.world = m_controller.cursorFromLogical(
            m_cursor.logical, {0.0f, m_viewBottomY}).world;
        m_cursor.world.x = std::clamp(m_cursor.world.x, 0.0f, m_document.levelWidth());
        m_cursor.world.y = std::clamp(m_cursor.world.y, 0.0f, m_document.levelHeight());
        m_keyboardCursorActive = false;
    }

    m_lastMouseLogical = mouseLogical;
    m_haveMousePosition = true;
}

void EditorSession::followKeyboardCursor() {
    const float viewportHeight = config::LOGICAL_HEIGHT;
    const float maxView = std::max(0.0f, m_document.levelHeight() - viewportHeight);
    m_viewBottomY = std::clamp(m_cursor.world.y - viewportHeight * 0.5f,
                               0.0f, maxView);
}

void EditorSession::moveKeyboardCursor(float dx, float dy) {
    m_keyboardCursorActive = true;
    m_cursor.world.x = std::clamp(m_cursor.world.x + dx, 0.0f, m_document.levelWidth());
    m_cursor.world.y = std::clamp(m_cursor.world.y + dy, 0.0f, m_document.levelHeight());
    m_cursor.logical = m_cursor.world;
    followKeyboardCursor();
}

bool EditorSession::placeKeyboardEntity() {
    switch (m_controller.entityTool()) {
        case EditorEntityTool::SPAWN:
            return m_controller.placeSpawnAt(m_cursor.world);
        case EditorEntityTool::FLAG:
            return m_controller.placeFlagAt(m_cursor.world);
        case EditorEntityTool::PLATFORM:
            break;
    }

    const std::size_t hit = m_controller.hitPlatform(m_cursor.world);
    if (m_controller.hasSelection() && hit != m_controller.selectedIndex()) {
        const auto& selected = m_document.platforms()[m_controller.selectedIndex()].bounds;
        const Vec2 newMin{
            m_cursor.world.x - selected.width() * 0.5f,
            m_cursor.world.y - selected.height() * 0.5f,
        };
        return m_document.movePlatform(m_controller.selectedIndex(), newMin);
    }

    if (hit < m_document.platformCount()) {
        m_controller.clearSelection();
        m_controller.setToolMode(EditorToolMode::STAMP);
        if (!m_controller.beginMove(m_cursor.world)) return false;
        m_controller.cancelMove();
        return true;
    }

    return m_controller.stampAt(m_cursor.world);
}

void EditorSession::recordEditBaseline(const LevelData& before) {
    if (m_applyingHistory) return;
    if (!m_undoHistory.empty() && sameDocumentState(m_undoHistory.back(), before)) return;
    m_undoHistory.push_back(before);
    m_redoHistory.clear();
}

bool EditorSession::undo() {
    if (m_undoHistory.empty()) return false;
    m_applyingHistory = true;

    const LevelData current = m_document.toLevelData(m_documentName);
    const LevelData target = m_undoHistory.back();
    if (!m_document.restoreFromLevelData(target)) {
        m_applyingHistory = false;
        return false;
    }

    m_undoHistory.pop_back();
    m_redoHistory.push_back(current);
    m_controller.clearSelection();
    m_applyingHistory = false;
    return true;
}

bool EditorSession::redo() {
    if (m_redoHistory.empty()) return false;
    m_applyingHistory = true;

    const LevelData current = m_document.toLevelData(m_documentName);
    const LevelData target = m_redoHistory.back();
    if (!m_document.restoreFromLevelData(target)) {
        m_applyingHistory = false;
        return false;
    }

    m_redoHistory.pop_back();
    m_undoHistory.push_back(current);
    m_controller.clearSelection();
    m_applyingHistory = false;
    return true;
}

void EditorSession::updateKeyboard(const InputManager& input,
                                   const core::KeyBindings& bindings) {
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorCursorLeft))
        moveKeyboardCursor(-1.0f, 0.0f);
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorCursorRight))
        moveKeyboardCursor(1.0f, 0.0f);
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorCursorUp))
        moveKeyboardCursor(0.0f, 1.0f);
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorCursorDown))
        moveKeyboardCursor(0.0f, -1.0f);

    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorPanUp)) {
        const float maxView = std::max(0.0f, m_document.levelHeight() - config::LOGICAL_HEIGHT);
        m_viewBottomY = std::clamp(m_viewBottomY + 16.0f, 0.0f, maxView);
    }
    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorPanDown))
        m_viewBottomY = std::clamp(m_viewBottomY - 16.0f, 0.0f,
                                   std::max(0.0f, m_document.levelHeight() - config::LOGICAL_HEIGHT));

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
            case EditorSizePreset::SMALL:
                m_controller.setSizePreset(EditorSizePreset::MEDIUM);
                break;
            case EditorSizePreset::MEDIUM:
                m_controller.setSizePreset(EditorSizePreset::LARGE);
                break;
            case EditorSizePreset::LARGE:
                break;
        }
    }

    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSelectPlatform)) {
        m_controller.setEntityTool(EditorEntityTool::PLATFORM);
        m_controller.clearSelection();
    } else if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSelectSpawn)) {
        m_controller.setEntityTool(EditorEntityTool::SPAWN);
        m_controller.clearSelection();
    } else if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSelectFlag)) {
        m_controller.setEntityTool(EditorEntityTool::FLAG);
        m_controller.clearSelection();
    }

    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorPlace))
        placeKeyboardEntity();

    if (core::isActionJustPressed(bindings, input, core::GameAction::DeleteSelection)) {
        if (m_controller.entityTool() == EditorEntityTool::FLAG) {
            m_controller.removeFlag();
        } else if (m_controller.entityTool() == EditorEntityTool::PLATFORM) {
            if (m_controller.hasSelection())
                m_document.removePlatform(m_controller.selectedIndex());
            m_controller.clearSelection();
        }
    }

    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorSave))
        saveLevel();

    if (core::isActionJustPressed(bindings, input, core::GameAction::EditorValidate))
        startValidation();
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

        if (m_controller.entityTool() == EditorEntityTool::SPAWN) {
            m_controller.placeSpawnAt(m_cursor.world);
            m_leftDragActive = false;
            return;
        }
        if (m_controller.entityTool() == EditorEntityTool::FLAG) {
            m_controller.placeFlagAt(m_cursor.world);
            m_leftDragActive = false;
            return;
        }

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
                           int32_t windowWidth,
                           int32_t windowHeight) {
    const LevelData before = m_document.toLevelData(m_documentName);
    const bool movingBefore = (m_controller.mode() == EditorMouseMode::MOVING);
    const bool undoPressed = core::isActionJustPressed(
        bindings, input, core::GameAction::EditorUndo);
    const bool redoPressed = core::isActionJustPressed(
        bindings, input, core::GameAction::EditorRedo);

    updateCursor(input, windowWidth, windowHeight);

    if (undoPressed || redoPressed) {
        if (undoPressed) undo();
        else redo();
        m_gestureBaselineValid = false;
        refreshValidationResult();
        return;
    }

    updateKeyboard(input, bindings);
    updateMouse(input);

    const LevelData after = m_document.toLevelData(m_documentName);
    const bool movingAfter = (m_controller.mode() == EditorMouseMode::MOVING);

    if (!movingBefore && movingAfter) {
        m_gestureBaseline = before;
        m_gestureBaselineValid = true;
    } else if (movingBefore && !movingAfter) {
        if (m_gestureBaselineValid && !sameDocumentState(m_gestureBaseline, after))
            recordEditBaseline(m_gestureBaseline);
        m_gestureBaselineValid = false;
    } else if (!movingBefore && !movingAfter) {
        if (!sameDocumentState(before, after))
            recordEditBaseline(before);
    }

    refreshValidationResult();
}

void EditorSession::cancelInteraction() {
    m_leftDragActive = false;
    if (m_controller.mode() == EditorMouseMode::MOVING) {
        m_controller.cancelMove();
        return;
    }
    m_controller.clearSelection();
}

} // namespace logic
