#pragma once
// =============================================================================
//  Game/Core/GameAction.h
//
//  @version 9.1
//  @history
//    v9.1 — criado. Fase 9.1: Sistema de Configuracao de Controlos.
//
//  Acoes logicas do jogo, independentes de tecla fisica. KeyBindings.h faz a
//  ponte entre uma GameAction e a(s) tecla(s) fisica(s) (logic::Key) que a
//  acionam — permite reatribuicao de teclas sem tocar em Player/main.cpp,
//  que passam a perguntar "a accao X esta activa?" em vez de "a tecla Y esta
//  premida?".
// =============================================================================
#include <cstddef>

namespace core {

enum class GameAction {
    MoveLeft,
    MoveRight,
    Jump,
    Pause,       // ESC durante PLAYING -> abre PAUSED; de novo -> retoma
    UIConfirm,   // confirma opcao seleccionada em MENU/PAUSED/CONTROLS
    UILeft,      // navega opcao anterior em menus
    UIRight,     // navega opcao seguinte em menus
    OpenEditor,  // abre o Editor de Niveis (Fase 9.3) a partir de MENU/PAUSED
    Quit,        // sai do jogo (equivalente a seleccionar "Sair")
};

// Todas as acoes, para iterar (ex: desenhar a lista no ecra CONTROLS).
inline constexpr GameAction ALL_ACTIONS[] = {
    GameAction::MoveLeft, GameAction::MoveRight, GameAction::Jump,
    GameAction::Pause,    GameAction::UIConfirm,  GameAction::UILeft,
    GameAction::UIRight,  GameAction::OpenEditor, GameAction::Quit,
};
inline constexpr std::size_t ACTION_COUNT = sizeof(ALL_ACTIONS) / sizeof(ALL_ACTIONS[0]);

// Nome legivel (PT) para UI — ecra CONTROLS.
const char* actionDisplayName(GameAction action);

// Nome estavel (ASCII, sem espacos) para o ficheiro controls.cfg. NUNCA mudar
// um nome ja usado sem migrar ficheiros de config existentes — e a chave
// persistida em disco.
const char* actionSerializedName(GameAction action);

// Inverso de actionSerializedName. Devolve false se o nome nao corresponder
// a nenhuma accao conhecida (linha invalida/de versao futura no ficheiro).
bool actionFromSerializedName(const char* name, GameAction& out);

} // namespace core
