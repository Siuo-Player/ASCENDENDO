#pragma once
// =============================================================================
//  Game/Logic/EditorSession.h
//
//  Camada de orquestração do editor.
//  Liga InputManager + EditorInteractionController sem conhecer Vulkan nem
//  estado de câmera.
// =============================================================================

#include "Core/KeyBindings.h"
#include "Logic/EditorInteraction.h"
#include "Logic/EditorLevelIO.h"
#include "Logic/EditorRenderSnapshot.h"
#include "Logic/EditorValidation.h"
#include "Logic/InputManager.h"
#include "Logic/LevelEditorValidator.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace logic {

struct EditorPreview {
    bool visible = false;
    AABB bounds{};
    EditorToolMode tool = EditorToolMode::STAMP;
};

struct EditorSaveResult {
    bool success = false;
    bool validationPassed = false;
    std::uint64_t generation = 0;
    std::string path;
    std::string message;
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
    std::uint64_t documentGeneration() const { return m_document.generation(); }
    bool canUndo() const { return !m_undoHistory.empty(); }
    bool canRedo() const { return !m_redoHistory.empty(); }

    void setPersistenceTarget(std::string path,
                              std::string name = "Editor Level");
    const std::string& persistencePath() const { return m_persistencePath; }
    const std::string& documentName() const { return m_documentName; }

    // Campaign Editor calls this only after LevelDataIO has parsed the level.
    // The LevelEditorDocument validates the new data with the selected
    // campaign-level policy before replacing the current document.
    bool loadLevelData(const LevelData& data,
                       bool finalCampaignLevel,
                       std::string path,
                       std::string name = "Editor Level") {
        if (!m_document.restoreFromLevelData(data, finalCampaignLevel)) return false;

        m_controller.clearSelection();
        m_document.setFinalCampaignLevel(finalCampaignLevel);
        setPersistenceTarget(std::move(path), std::move(name));
        m_undoHistory.clear();
        m_redoHistory.clear();
        m_validationTask.discard();
        m_validationResult = {};
        m_lastSaveResult = {};
        m_keyboardCursorActive = false;
        m_haveMousePosition = false;
        m_leftDragActive = false;
        return true;
    }

    EditorSaveResult saveLevel();
    EditorSaveResult saveLevel(const std::string& path,
                               const std::string& name = "Editor Level");

    bool startValidation();
    bool startValidation(const std::string& path);
    EditorAsyncValidationResult pollValidation();
    const EditorAsyncValidationResult& validationResult() const { return m_validationResult; }

    const EditorSaveResult& lastSaveResult() const { return m_lastSaveResult; }

    EditorPreview preview() const;
    EditorRenderSnapshot renderSnapshot() const;

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
    void refreshValidationResult();

    void recordEditBaseline(const LevelData& before);
    bool undo();
    bool redo();
    bool placeKeyboardEntity();
    void moveKeyboardCursor(float dx, float dy);

    LevelEditorDocument m_document;
    EditorInteractionController m_controller;
    EditorCursor m_cursor{};
    Vec2 m_lastMouseLogical{};
    Vec2 m_pressedWorld{};
    bool m_leftDragActive = false;
    bool m_haveMousePosition = false;
    bool m_keyboardCursorActive = false;

    std::string m_persistencePath;
    std::string m_documentName = "Editor Level";
    EditorSaveResult m_lastSaveResult{};
    EditorValidationTask m_validationTask;
    EditorAsyncValidationResult m_validationResult{};

    std::vector<LevelData> m_undoHistory;
    std::vector<LevelData> m_redoHistory;
    bool m_applyingHistory = false;
};

} // namespace logic
