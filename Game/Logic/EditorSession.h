#pragma once
// =============================================================================
//  Game/Logic/EditorSession.h
//
//  Camada de orquestração do editor.
//  Liga InputManager + EditorInteractionController sem conhecer Vulkan nem
//  estado de câmera: o Level Editor ocupa sempre uma única tela lógica 640x360.
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

    void setPersistenceTarget(std::string path,
                              std::string name = "Editor Level");
    const std::string& persistencePath() const { return m_persistencePath; }
    const std::string& documentName() const { return m_documentName; }

    EditorSaveResult saveLevel();
    EditorSaveResult saveLevel(const std::string& path,
                               const std::string& name = "Editor Level");

    bool startValidation();
    bool startValidation(const std::string& path);
    EditorValidationResult pollValidation();
    const EditorValidationResult& validationResult() const { return m_validationResult; }

    const EditorSaveResult& lastSaveResult() const { return m_lastSaveResult; }

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
    void refreshValidationResult();

    LevelEditorDocument m_document;
    EditorInteractionController m_controller;
    EditorCursor m_cursor{};
    Vec2 m_pressedWorld{};
    bool m_leftDragActive = false;

    std::string m_persistencePath;
    std::string m_documentName = "Editor Level";
    EditorSaveResult m_lastSaveResult{};
    EditorValidationTask m_validationTask;
    EditorValidationResult m_validationResult{};
};

} // namespace logic