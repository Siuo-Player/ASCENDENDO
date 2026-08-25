// =============================================================================
//  ASCENDENDO — GameAction
// =============================================================================
#include "Core/GameAction.h"
#include <cstring>

namespace core {

const char* actionDisplayName(GameAction action) {
    switch (action) {
        case GameAction::MoveLeft:          return "Mover Esquerda";
        case GameAction::MoveRight:         return "Mover Direita";
        case GameAction::Jump:              return "Saltar";
        case GameAction::Pause:             return "Pausa";
        case GameAction::UIConfirm:         return "Confirmar";
        case GameAction::UILeft:            return "Menu: Anterior";
        case GameAction::UIRight:           return "Menu: Seguinte";
        case GameAction::OpenEditor:        return "Abrir Editor de Nivel";
        case GameAction::OpenCampaignEditor:return "Abrir Editor de Campanha";
        case GameAction::OpenControls:      return "Ver Controlos";
        case GameAction::Quit:              return "Sair do Jogo";
        case GameAction::EditorPanUp:       return "Editor: Deslocar Cima";
        case GameAction::EditorPanDown:     return "Editor: Deslocar Baixo";
        case GameAction::EditorToggleMode:  return "Editor: Alternar STAMP/DRAG";
        case GameAction::EditorSizeDown:    return "Editor: Tamanho Menor";
        case GameAction::EditorSizeUp:      return "Editor: Tamanho Maior";
        case GameAction::DeleteSelection:   return "Editor: Apagar Seleção";
        case GameAction::EditorSave:        return "Editor: Guardar";
        case GameAction::EditorTest:        return "Editor: Testar Nivel";
        case GameAction::EditorValidate:    return "Editor: Validar Nivel";
    }
    return "?";
}

const char* actionSerializedName(GameAction action) {
    switch (action) {
        case GameAction::MoveLeft:           return "MoveLeft";
        case GameAction::MoveRight:          return "MoveRight";
        case GameAction::Jump:               return "Jump";
        case GameAction::Pause:              return "Pause";
        case GameAction::UIConfirm:          return "UIConfirm";
        case GameAction::UILeft:             return "UILeft";
        case GameAction::UIRight:            return "UIRight";
        case GameAction::OpenEditor:         return "OpenEditor";
        case GameAction::OpenCampaignEditor:return "OpenCampaignEditor";
        case GameAction::OpenControls:       return "OpenControls";
        case GameAction::Quit:               return "Quit";
        case GameAction::EditorPanUp:        return "EditorPanUp";
        case GameAction::EditorPanDown:      return "EditorPanDown";
        case GameAction::EditorToggleMode:   return "EditorToggleMode";
        case GameAction::EditorSizeDown:     return "EditorSizeDown";
        case GameAction::EditorSizeUp:       return "EditorSizeUp";
        case GameAction::DeleteSelection:    return "DeleteSelection";
        case GameAction::EditorSave:         return "EditorSave";
        case GameAction::EditorTest:         return "EditorTest";
        case GameAction::EditorValidate:     return "EditorValidate";
    }
    return "Unknown";
}

bool actionFromSerializedName(const char* name, GameAction& out) {
    if (!name) return false;
    for (GameAction a : ALL_ACTIONS) {
        if (std::strcmp(actionSerializedName(a), name) == 0) {
            out = a;
            return true;
        }
    }
    return false;
}

} // namespace core
